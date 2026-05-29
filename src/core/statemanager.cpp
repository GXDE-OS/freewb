#include "statemanager.h"

#include <algorithm>
#include <cstdint>

#include "candidatelist.h"
#include "chttrans.h"
#include "committer.h"
#include "common.h"
#include "enginemanager.h"
#include "freewb.h"
#include "idbus.h"
#include "special.h"
#include "key.h"
#include "log.h"
#include "settings.h"

namespace freewb
{

/** 当前引擎是否在 state 的不感兴趣列表中。 */
#define CHECK_ENGINE(manager, state)                                                                                       \
    do                                                                                                                     \
    {                                                                                                                      \
        const Freewb *const _freewb = (manager).freewb_;                                                                   \
        if (_freewb != nullptr && _freewb->engineManager() != nullptr)                                                     \
        {                                                                                                                  \
            const char *const _engine = _freewb->engineManager()->currentEngineName();                                     \
            if (_engine != nullptr)                                                                                        \
            {                                                                                                              \
                const std::vector<std::string> _uninterested = (state).uninterestedEngines();                              \
                if (std::find(_uninterested.begin(), _uninterested.end(), _engine) != _uninterested.end())                 \
                {                                                                                                          \
                    return false;                                                                                          \
                }                                                                                                          \
            }                                                                                                              \
        }                                                                                                                  \
    } while (0)

/** 无参 IDBus：X("dos.", callOpenConfDirMethod) */
#define QUICK_COMMAND_CALL_DBUS_METHOD(X)                                                                                  \
    X("dos.", callOpenConfDirMethod)                                                                                       \
    X("tt.", callSwitchRecodeProofMethod)                                                                                  \
    X("hh.", callSwitchToolbarHideFlgMethod)                                                                               \
    X("cc.", callPanelHideToolbar) /* TODO: 显/隐候选窗 */                                                                 \
    X("mm.", callPanelSwitchCharSetMethod)                                                                                 \
    X("oo.", callOpenUiSettingMethod)                                                                                      \
    X("pp.", callOpenProfessionalSettingMethod)                                                                            \
    X("vv.", callShowVersionInfoMethod)                                                                                    \
    X("qq.", callModQuickTableMethod)                                                                                      \
    X("uu.", callModUserTableMethod)                                                                                       \
    X("uw.", callModWubiTableMethod)                                                                                       \
    X("up.", callModPinyinTableMethod)

/** 需传参 IDBus 方法的快捷命令：X("aa.", handleAddPhrase) */
#define QUICK_COMMAND_HANDLE_CUSTOM(X)                                                                                   \
    X("aa.", handleAddPhrase)                                                                                              \
    X("dd.", handleDeletePhrase)                                                                                           \
    X("ff.", handleDictQuery)                                                                                              \
    X("jj.", handleToggleChttrans)                                                                                         \
    X("ss.", handleSwitchUncommon)                                                                                         \
    X("kk.", handleSwitchVirtualKeyboard)

namespace
{

/** 过滤非汉字内容。 */
std::string filterNonHanziContent(const std::string &text)
{
    std::string out;
    const std::size_t charCount = MbDictionaryTable::utf8CharCount(text);
    out.reserve(text.size());
    for (std::size_t i = 0; i < charCount; ++i)
    {
        const std::string ch = MbDictionaryTable::utf8CharAt(text, i);
        if (ch.size() != 3U)
        {
            continue;
        }
        const unsigned char c0 = static_cast<unsigned char>(ch[0]);
        const unsigned char c1 = static_cast<unsigned char>(ch[1]);
        const unsigned char c2 = static_cast<unsigned char>(ch[2]);
        if ((c0 >> 4U) != 14U || (c1 >> 6U) != 2U || (c2 >> 6U) != 2U)
        {
            continue;
        }
        const std::uint32_t cp =
            (static_cast<std::uint32_t>(c0 & 0x0FU) << 12U) | (static_cast<std::uint32_t>(c1 & 0x3FU) << 6U) |
            static_cast<std::uint32_t>(c2 & 0x3FU);
        if ((cp >= 0x4E00U && cp <= 0x9FFFU) || (cp >= 0x3400U && cp <= 0x4DBFU))
        {
            out.append(ch);
        }
    }
    return out;
}

} // namespace

bool IdleState::processKey(FreewbKeySym /*keysym*/, FreewbKeyState /*state*/)
{
    return false;
}

void IdleState::cancel() {}

const char *IdleState::name() const
{
    return "state:idle";
}

bool IdleState::available() const
{
    return true;
}

void IdleState::changeAvailable()
{
    return;
}

std::vector<std::string> IdleState::uninterestedEngines() const
{
    return {};
}

AddUserPhraseState::AddUserPhraseState(StateManager *manager, Freewb *freewb)
    : manager_(manager), freewb_(freewb)
{
}

const char *AddUserPhraseState::name() const
{
    return "state:addPhrase";
}

bool AddUserPhraseState::available() const
{
    return available_;
}

void AddUserPhraseState::changeAvailable()
{
    available_ = !available_;
}

std::string AddUserPhraseState::utf8SuffixSkipChars(const std::string &text, int skipChars)
{
    const char *p = text.c_str();
    for (int i = 0; i < skipChars && *p != '\0'; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < 0x80U)
        {
            ++p;
        }
        else if ((c >> 5U) == 6U)
        {
            p += 2;
        }
        else if ((c >> 4U) == 14U)
        {
            p += 3;
        }
        else if ((c >> 3U) == 30U)
        {
            p += 4;
        }
        else
        {
            ++p;
        }
    }
    return std::string(p);
}

bool AddUserPhraseState::beginFromHistory()
{
    FREEWB_DEBUG("[{}] beginFromHistory", name());
    fromClipboard_ = false;
    if (freewb_ == nullptr || freewb_->committer() == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(freewb_->committer()->committedText(kPhraseMaxLength));
    sourceCharCount_ = static_cast<int>(MbDictionaryTable::utf8CharCount(originalText_));
    if (sourceCharCount_ <= 1)
    {
        FREEWB_WARN("[{}] beginFromHistory: insufficient hanzi in commit history (count={})", name(), sourceCharCount_);
        originalText_.clear();
        return false;
    }

    phraseLen_ = kDefaultPhraseLen;
    refreshPhrase();
    if (wordText_.empty())
    {
        return false;
    }
    updatePhraseInfoToUI();
    return true;
}

void AddUserPhraseState::refreshPhrase()
{
    if (originalText_.empty() || sourceCharCount_ <= 0 || freewb_ == nullptr || freewb_->engineManager() == nullptr)
    {
        return;
    }

    const int skipChars = sourceCharCount_ - phraseLen_;
    wordText_ = utf8SuffixSkipChars(originalText_, skipChars);
    wordCode_.clear();
    wordCode_ = freewb_->engineManager()->calculateWubiPhraseCode(wordText_);
}

bool AddUserPhraseState::beginFromClipboard()   
{
    FREEWB_DEBUG("[{}] beginFromClipboard", name());
    fromClipboard_ = true;
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(freewb_->dbusProxy()->callGetClipboardMethod());
    FREEWB_DEBUG("clip: {}", originalText_);
    sourceCharCount_ = static_cast<int>(MbDictionaryTable::utf8CharCount(originalText_));
    if (sourceCharCount_ <= 1 || sourceCharCount_ >= kPhraseMaxLength)
    {
        FREEWB_WARN("clip is too long or too short");
        originalText_.clear();
        return false;
    }

    phraseLen_ = sourceCharCount_;
    refreshPhrase();
    if (wordText_.empty())
    {
        originalText_.clear();
        return false;
    }
    updatePhraseInfoToUI();
    return true;
}

void AddUserPhraseState::cancel()
{
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callAddUsrParseMethod(2, "", "");
    }
}

void AddUserPhraseState::updatePhraseInfoToUI()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return;
    }

    // 如果词组编码为空且不是从剪贴板获取，则设置为自定义词组编码
    // 0: 显示待造词提示
    // 1: 确认造词
    // 2: 取消造词
    // 3: 自定义词组编码
    const int flg = (wordCode_.empty() && !fromClipboard_) ? 3 : 0;
    FREEWB_DEBUG("flg: {}, wordCode: {}, wordText: {}", flg, wordCode_, wordText_);
    freewb_->dbusProxy()->callAddUsrParseMethod(flg, wordCode_, wordText_);
}

bool AddUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return true;
    }

    switch (keysym)
    {
    case FreewbKey_Left:
        // 左移：增加词组长度
        if (state == FreewbKeyState_None)
        {
            const int maxChars = sourceCharCount_;
            if (phraseLen_ < maxChars && phraseLen_ < kPhraseMaxLength - 1)
            {
                ++phraseLen_;
                refreshPhrase();
                updatePhraseInfoToUI();
            }
        }
        break;
    case FreewbKey_Right:
        // 右移：减少词组长度
        if (state == FreewbKeyState_None && phraseLen_ > kDefaultPhraseLen)
        {
            --phraseLen_;
            refreshPhrase();
            updatePhraseInfoToUI();
        }
        break;
    case FreewbKey_Return:
        if (state & FreewbKeyState_Ctrl)
        {
            freewb_->dbusProxy()->callAddUsrParseMethod(3, wordCode_, wordText_);
            break;
        }
        if (state == FreewbKeyState_None)
        {
            freewb_->dbusProxy()->callAddUsrParseMethod(1, wordCode_, wordText_);
            if (manager_ != nullptr)
            {
                manager_->enterIdleState();
            }
        }
        break;
    default:
        break;
    }
    return true;
}

std::vector<std::string> AddUserPhraseState::uninterestedEngines() const
{
    return {"engine:py", "engine:en"};
}

DeleteUserPhraseState::DeleteUserPhraseState(StateManager *manager, Freewb *freewb)
    : manager_(manager), freewb_(freewb)
{
}

const char *DeleteUserPhraseState::name() const
{
    return "state:deletePhrase";
}

bool DeleteUserPhraseState::available() const
{
    return available_;
}

void DeleteUserPhraseState::changeAvailable()
{
    available_ = !available_;
}

bool DeleteUserPhraseState::begin(const std::string &wordText)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    wordText_ = filterNonHanziContent(wordText);
    if (wordText_.empty())
    {
        FREEWB_WARN("[{}] begin: no hanzi in commit text", name());
        return false;
    }
    wordCode_.clear();
    if (freewb_->engineManager() != nullptr)
    {
        wordCode_ = freewb_->engineManager()->calculateWubiPhraseCode(wordText_);
    }
    freewb_->dbusProxy()->callDeleteUsrParseMethod(0, wordCode_, wordText_);
    return true;
}

void DeleteUserPhraseState::cancel()
{
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callDeleteUsrParseMethod(2, "", "");
    }
}

bool DeleteUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return true;
    }

    if (keysym == FreewbKey_Return && state == FreewbKeyState_None)
    {
        freewb_->dbusProxy()->callDeleteUsrParseMethod(1, wordCode_, wordText_);
        if (manager_ != nullptr)
        {
            manager_->enterIdleState();
        }
        return true;
    }
    return true;
}

std::vector<std::string> DeleteUserPhraseState::uninterestedEngines() const
{
    return {"engine:py", "engine:en"};
}

TempEnglishState::TempEnglishState(StateManager *manager, Freewb *freewb) : manager_(manager), freewb_(freewb)
{
    registerQuickCommands();
}

const char *TempEnglishState::name() const
{
    return "state:tempEnglish";
}

bool TempEnglishState::available() const
{
    return available_;
}

void TempEnglishState::changeAvailable()
{
    available_ = !available_;
}

bool TempEnglishState::handleAddPhrase()
{
    if (manager_ == nullptr)
    {
        return false;
    }
    const bool entered = manager_->enterAddPhraseState(false);
    if (!entered)
    {
        FREEWB_WARN("[{}] add phrase failed: insufficient commit history", name());
        return false;
    }
    if (freewb_ == nullptr)
    {
        return true;
    }
    CandidateList *const candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return true;
    }
    candidates->setPreeditText("");
    return true;
}

bool TempEnglishState::handleDeletePhrase()
{
    if (manager_ == nullptr || freewb_ == nullptr || freewb_->committer() == nullptr)
    {
        return false;
    }
    const bool entered = manager_->enterDeletePhraseState(freewb_->committer()->lastCommitString());
    if (!entered)
    {
        FREEWB_WARN("[{}] delete phrase failed: empty commit text", name());
        return false;
    }
    if (CandidateList *const candidates = freewb_->candidateList(); candidates != nullptr)
    {
        candidates->setPreeditText("");
    }
    return true;
}

bool TempEnglishState::handleDictQuery()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr || freewb_->committer() == nullptr)
    {
        return false;
    }
    freewb_->dbusProxy()->callDictQueryMethod(freewb_->committer()->lastCommitString());
    return true;
}

bool TempEnglishState::handleToggleChttrans()
{
    if (freewb_ == nullptr)
    {
        return false;
    }
    if (freewb_->chttrans() != nullptr)
    {
        freewb_->chttrans()->changeAvailable();
    }
    if (freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callPanelSwitchChttransMethod();
    }
    return true;
}

bool TempEnglishState::handleSwitchUncommon()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr || freewb_->committer() == nullptr)
    {
        return false;
    }
    freewb_->dbusProxy()->callSwitchUncommonParseStateMethod(freewb_->committer()->lastCommitString(), 1);
    return true;
}

bool TempEnglishState::handleSwitchVirtualKeyboard()
{
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callSwitchVirtualKeyboardModeMethod(0);
    }
    return true;
}

void TempEnglishState::registerQuickCommands()
{
#define QUICK_COMMAND_REGISTER_DBUS_METHOD(PATTERN, METHOD)                                                                 \
    quickCommands_[PATTERN] = [freewb = freewb_, method = &ipc::IDBus::METHOD]()                                          \
    {                                                                                                                     \
        if (freewb != nullptr && freewb->dbusProxy() != nullptr)                                                          \
        {                                                                                                                 \
            (freewb->dbusProxy()->*method)();                                                                             \
        }                                                                                                                 \
        return true;                                                                                                      \
    };
    QUICK_COMMAND_CALL_DBUS_METHOD(QUICK_COMMAND_REGISTER_DBUS_METHOD)
#undef QUICK_COMMAND_REGISTER_DBUS_METHOD

#define QUICK_COMMAND_REGISTER_CUSTOM(PATTERN, HANDLER)                                                                   \
    quickCommands_[PATTERN] = [this]() { return HANDLER(); };
    QUICK_COMMAND_HANDLE_CUSTOM(QUICK_COMMAND_REGISTER_CUSTOM)
#undef QUICK_COMMAND_REGISTER_CUSTOM
}

bool TempEnglishState::dispatchQuickCommand(const std::string &commandBodyWithoutPrefix)
{
    if (manager_ == nullptr)
    {
        return false;
    }
    const auto it = quickCommands_.find(commandBodyWithoutPrefix);
    if (it == quickCommands_.end())
    {
        return false;
    }
    FREEWB_DEBUG("[{}] quick command: {}", name(), commandBodyWithoutPrefix);
    if (!it->second())
    {
        FREEWB_WARN("[{}] quick command failed: {}", name(), commandBodyWithoutPrefix);
        return false;
    }
    if (manager_->isTempEnglish())
    {
        manager_->reset();
    }
    return true;
}

std::vector<std::string> TempEnglishState::uninterestedEngines() const
{
    return {"engine:en"};
}

bool TempEnglishState::begin(const std::string &commandPrefix)
{
    if (commandPrefix.empty() || freewb_ == nullptr)
    {
        return false;
    }
    CandidateList *candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return false;
    }
    // 已有编码或候选时不进入临时英文
    if (!candidates->preeditText().empty() || candidates->totalCandidateCount() > 0)
    {
        return false;
    }
    secondRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_secondRecodeKey().c_str());
    thirdRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_thirdRecodeKey().c_str());
    candidates->clear();
    candidates->setPreeditText(commandPrefix);
    return true;
}

void TempEnglishState::cancel()
{
    if (freewb_ == nullptr)
    {
        return;
    }
    CandidateList *candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return;
    }
    candidates->clear();
    candidates->setPreeditText("");
}

void TempEnglishState::refreshQuickFormatCandidates()
{
    if (freewb_ == nullptr)
    {
        return;
    }
    CandidateList *const candidates = freewb_->candidateList();
    Special *const special = freewb_->special();
    if (candidates == nullptr)
    {
        return;
    }

    const std::string &pe = candidates->preeditText();
    if (pe.size() <= 1U || special == nullptr || !special->available())
    {
        candidates->setCandidates({}, {});
        return;
    }

    const std::vector<std::string> texts = special->formatSpecialValues(pe.substr(1));
    if (texts.empty())
    {
        candidates->setCandidates({}, {});
        return;
    }

    std::vector<std::string> prompts(texts.size());
    candidates->setCandidates(texts, prompts);
}

bool TempEnglishState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (manager_ == nullptr || freewb_ == nullptr)
    {
        return false;
    }

    CandidateList *const candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return false;
    }

    const std::string &preedit = candidates->preeditText();
    const std::size_t candidateCount = candidates->totalCandidateCount();
    Committer *const committer = freewb_->committer();
    // 有足够候选时在此直接上屏；否则按键落入下方逻辑写入预编辑
    if (state == FreewbKeyState_None && committer != nullptr &&
        ((keysym == secondRecodeKey_ && candidateCount > 1) || (keysym == thirdRecodeKey_ && candidateCount > 2)))
    {
        const int idx = keysym == secondRecodeKey_ ? 1 : 2;
        committer->commit(candidates->selectCandidateText(idx));
        manager_->reset();
        return true;
    }

    if (Key::isModifierKeySym(keysym))
    {
        return false;
    }
    switch (keysym)
    {
    // 删除字符
    case FreewbKey_BackSpace:
        if (state != FreewbKeyState_None)
        {
            return true;
        }
        if (preedit.size() <= 1U)
        {
            manager_->reset();
        }
        else
        {
            candidates->popPreeditText();
            refreshQuickFormatCandidates();
        }
        return true;

    // 空格上屏
    case FreewbKey_space:
        if (state != FreewbKeyState_None)
        {
            return true;
        }
        if (preedit.size() > 1U && committer != nullptr)
        {
            if (candidates->size() > 0)
            {
                committer->commit(candidates->selectCandidateText(0));
            }
            else
            {
                committer->commit(preedit.substr(1));
            }
        }
        manager_->reset();
        return true;

    // 回车上屏
    case FreewbKey_Return:
        if (state != FreewbKeyState_None)
        {
            return true;
        }
        if (preedit.size() > 1U && committer != nullptr)
        {
            committer->commit(preedit.substr(1));
        }
        manager_->reset();
        return true;

    case FreewbKey_period:
    default:
        break;
    }

    const char *const key = Key::keySymToName(keysym);
    if (key == nullptr || key[0] == '\0' || key[1] != '\0')
    {
        return true;
    }

    if (keysym == FreewbKey_period && dispatchQuickCommand(preedit.substr(1) + key))
    {
        return true;
    }

    candidates->setPreeditText(preedit + key);
    refreshQuickFormatCandidates();
    return true;
}

StateManager::StateManager(Freewb *freewb)
    : freewb_(freewb), current_(&idle_), add_(this, freewb), del_(this, freewb), tempEnglish_(this, freewb)
{
}

bool StateManager::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (current_ == &idle_)
    {
        return false;
    }

    if (keysym == FreewbKey_Escape && state == FreewbKeyState_None)
    {
        return false;
    }

    return current_->processKey(keysym, state);
}

bool StateManager::enterAddPhraseState(bool useClipboardText)
{
    CHECK_ENGINE(*this, add_);
    const bool entered = useClipboardText ? add_.beginFromClipboard() : add_.beginFromHistory();
    if (entered)
    {
        current_ = &add_;
    }
    return entered;
}

bool StateManager::enterDeletePhraseState(const std::string &wordText)
{
    CHECK_ENGINE(*this, del_);
    const bool entered = del_.begin(wordText);
    if (entered)
    {
        current_ = &del_;
    }
    return entered;
}

bool StateManager::enterTempEnglishState(const std::string &commandPrefix)
{
    CHECK_ENGINE(*this, tempEnglish_);
    const bool entered = tempEnglish_.begin(commandPrefix);
    if (entered)
    {
        current_ = &tempEnglish_;
    }
    return entered;
}

void StateManager::reset()
{
    if (current_ == &idle_)
    {
        return;
    }

    current_->cancel();
    current_ = &idle_;
}

} // namespace freewb
