#include "wbzx.h"

#include <cctype>
#include <fstream>
#include <unordered_set>
#include <vector>

#include "log.h"
#include "settings.h"
#include "utils.h"

namespace freewb
{

WbzxEngine::WbzxEngine()
{
    clearMbLoadState();
    loadDictionary();
    initSingleHanziPrimaryCodeFromMbTable();
}

WbzxEngine::~WbzxEngine() = default;

const char *WbzxEngine::name() const
{
    return "engine:wbzx";
}

bool WbzxEngine::available() const
{
    return true;
}

void WbzxEngine::changeAvailable()
{
    available_ = !available_;
}

void WbzxEngine::toggleCharset()
{
    gb2312Filter_.changeAvailable();
}

void WbzxEngine::fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const
{
    const std::size_t n = payload.texts.size();
    const std::size_t rawLen = preedit.size();
    payload.prompts.assign(n, std::string{});

    for (std::size_t i = 0; i < n; ++i)
    {
        const std::string &fullCode =
            (i < payload.fullCodes.size() && !payload.fullCodes[i].empty()) ? payload.fullCodes[i] : preedit;
        if (fullCode.size() > rawLen)
        {
            const std::size_t suffixLen = fullCode.size() - rawLen;
            payload.prompts[i].reserve(suffixLen + 1);
            payload.prompts[i].push_back(' ');
            payload.prompts[i].append(fullCode, rawLen, std::string::npos);
        }
    }
}

bool WbzxEngine::isPreeditOverflow(const std::string &full) const
{
    if (full.empty())
    {
        return false;
    }

    return !hasVisibleMainDictCandidate(full) && !userDict_.hasEntryStartingWithPrefix(full);
}

void WbzxEngine::putKey(const char *strCode)
{
    result_.clearRows();

    if (strCode == nullptr)
    {
        return;
    }

    const std::string prefix(strCode);
    userDict_.appendCandidatesForPrefix(prefix, result_);
    const std::size_t userCandidateCount = result_.texts.size();
    mbTable_.appendCandidatesForPrefix(prefix, result_);
    filterMainDictCandidates(result_, userCandidateCount);
    fillCandidatePayloadPrompts(prefix, result_);
}

const CandidatePayload &WbzxEngine::getResult() const
{
    return result_;
}

void WbzxEngine::clearMbLoadState()
{
    mbTable_.clear();
    singleHanziPrimaryCode_.clear();
    result_.clearRows();
}

void WbzxEngine::initSingleHanziPrimaryCodeFromMbTable()
{
    /* 仅首字节是字母的编码参与；剔除 /xxx 等特殊键。
     * 对已入选者仅当新编码更长时替换；等长时保留先记录的。 */
    singleHanziPrimaryCode_.clear();
    for (const auto &kv : mbTable_.singleCharLexicon())
    {
        const std::string &code = kv.first;
        if (code.empty() || !std::isalpha(static_cast<unsigned char>(code[0])))
        {
            continue;
        }
        for (const std::string &hz : kv.second)
        {
            if (MbDictionaryTable::utf8CharCount(hz) != 1U)
            {
                continue;
            }
            const auto it = singleHanziPrimaryCode_.find(hz);
            if (it == singleHanziPrimaryCode_.end())
            {
                singleHanziPrimaryCode_.emplace(hz, code);
            }
            else if (code.size() > it->second.size())
            {
                it->second = code;
            }
        }
    }
}

std::string WbzxEngine::primaryWubiCodeForSingleHanziUtf8(const std::string &hz) const
{
    if (hz.empty())
    {
        return {};
    }
    const auto it = singleHanziPrimaryCode_.find(hz);
    if (it == singleHanziPrimaryCode_.end())
    {
        return {};
    }
    return it->second;
}

std::string WbzxEngine::calculateWubiPhraseCode(const std::string &phrase) const
{
    if (phrase.empty() || mbTable_.phraseEncodeRules().empty())
    {
        return {};
    }

    const uint32_t codeLength = mbTable_.codeLength();
    if (codeLength == 0)
    {
        return {};
    }

    const std::size_t charCount = MbDictionaryTable::utf8CharCount(phrase);
    if (charCount < 2)
    {
        return {};
    }

    uint8_t ruleFlag = 0;
    uint8_t ruleWords = 0;
    if (charCount >= codeLength)
    {
        ruleWords = static_cast<uint8_t>(codeLength);
        ruleFlag = 1;
    }
    else
    {
        ruleWords = static_cast<uint8_t>(charCount);
        ruleFlag = 0;
    }

    const EngineRuleBlock *block = nullptr;
    for (const EngineRuleBlock &candidate : mbTable_.phraseEncodeRules())
    {
        if (candidate.iWords == ruleWords && candidate.iFlag == ruleFlag)
        {
            block = &candidate;
            break;
        }
    }
    if (block == nullptr || block->cells.size() < codeLength)
    {
        return {};
    }

    std::string code;
    code.reserve(codeLength);
    for (uint32_t k = 0; k < codeLength; ++k)
    {
        const EngineRuleCell &cell = block->cells[k];
        std::string hz;
        if (cell.iFlag != 0)
        {
            if (cell.iWhich == 0)
            {
                return {};
            }
            hz = MbDictionaryTable::utf8CharAt(phrase, static_cast<std::size_t>(cell.iWhich - 1));
        }
        else
        {
            if (cell.iWhich == 0 || charCount < cell.iWhich)
            {
                return {};
            }
            hz = MbDictionaryTable::utf8CharAtFromEnd(phrase, cell.iWhich);
        }
        if (hz.empty())
        {
            return {};
        }

        const std::string singleCode = primaryWubiCodeForSingleHanziUtf8(hz);
        if (singleCode.empty() || cell.iIndex == 0 || singleCode.size() < cell.iIndex)
        {
            return {};
        }
        code.push_back(singleCode[cell.iIndex - 1]);
    }
    return code;
}

void WbzxEngine::loadDictionary()
{
    const std::string &relPath = settings::instance().get_wubiTable();
    if (relPath.empty())
    {
        FREEWB_WARN("wubi table path is empty");
        return;
    }

    const std::string path = userFreewbPath() + "/data/mb/" + relPath;
    if (path.empty())
    {
        FREEWB_WARN("wubi table path is invalid: {}", path);
        return;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        FREEWB_WARN("Cannot open wubi dictionary: {}", path);
        return;
    }

    if (!mbTable_.loadFromStream(in, nullptr))
    {
        mbTable_.clear();
        singleHanziPrimaryCode_.clear();
        result_.clearRows();
        return;
    }
}

void WbzxEngine::reset()
{
    inputCodes_.clear();
    result_.clearRows();
}

void WbzxEngine::reloadMainDictionary()
{
    clearMbLoadState();
    loadDictionary();
    initSingleHanziPrimaryCodeFromMbTable();
}

void WbzxEngine::reloadUserDictionary()
{
    userDict_.reload();
}

void WbzxEngine::filterMainDictCandidates(CandidatePayload &payload, const std::size_t userCandidateCount) const
{
    if (userCandidateCount >= payload.texts.size())
    {
        return;
    }

    CandidatePayload filtered;
    filtered.texts.reserve(payload.texts.size());
    filtered.fullCodes.reserve(payload.fullCodes.size());
    filtered.prompts.reserve(payload.prompts.size());

    for (std::size_t i = 0; i < payload.texts.size(); ++i)
    {
        if (i >= userCandidateCount)
        {
            const std::string &code = (i < payload.fullCodes.size()) ? payload.fullCodes[i] : std::string{};
            if (userDict_.isDeleted(code, payload.texts[i]) || gb2312Filter_.needFilt(payload.texts[i]))
            {
                continue;
            }
        }

        filtered.texts.push_back(payload.texts[i]);
        if (i < payload.fullCodes.size())
        {
            filtered.fullCodes.push_back(payload.fullCodes[i]);
        }
        if (i < payload.prompts.size())
        {
            filtered.prompts.push_back(payload.prompts[i]);
        }
    }

    payload.texts = std::move(filtered.texts);
    payload.fullCodes = std::move(filtered.fullCodes);
    payload.prompts = std::move(filtered.prompts);
}

bool WbzxEngine::hasVisibleMainDictCandidate(const std::string &prefix) const
{
    if (prefix.empty())
    {
        return false;
    }

    const auto scanPrefix = [this, &prefix](const std::unordered_map<std::string, std::vector<std::string>> &dict) -> bool
    {
        for (const auto &kv : dict)
        {
            const std::string &key = kv.first;
            if (key.size() < prefix.size() || key.compare(0, prefix.size(), prefix) != 0)
            {
                continue;
            }
            for (const std::string &hz : kv.second)
            {
                if (!gb2312Filter_.needFilt(hz) && !userDict_.isDeleted(key, hz))
                {
                    return true;
                }
            }
        }
        return false;
    };

    return scanPrefix(mbTable_.singleCharLexicon()) || scanPrefix(mbTable_.multiCharLexicon());
}

bool WbzxEngine::addUserWord(const std::string &code, const std::string &text)
{
    if (code.empty() || text.empty())
    {
        FREEWB_ERROR("WbzxEngine::addUserWord: empty code or text");
        return false;
    }

    bool changed = false;
    if (mbTable_.hasEntry(code, text))
    {
        // 主码表词条：恢复候选，不写 UserWord
        if (userDict_.hasUserEntry(code, text))
        {
            changed = userDict_.removeUserEntry(code, text);
        }
        if (userDict_.isDeleted(code, text))
        {
            changed = userDict_.removeDeletedEntry(code, text) || changed;
        }
    }
    else if (userDict_.hasUserEntry(code, text) && !userDict_.isDeleted(code, text))
    {
        return true;
    }
    else
    {
        // 自造词：写入 UserWord
        if (!userDict_.hasUserEntry(code, text))
        {
            if (!userDict_.addUserEntry(code, text))
            {
                FREEWB_WARN("WbzxEngine::addUserWord: add failed code={} text={}", code, text);
                return false;
            }
            changed = true;
        }
        if (userDict_.isDeleted(code, text))
        {
            changed = userDict_.removeDeletedEntry(code, text) || changed;
        }
    }

    if (!changed)
    {
        return true;
    }

    const bool saved = userDict_.save();
    userDict_.reload();
    if (!saved)
    {
        FREEWB_ERROR("WbzxEngine::addUserWord: save failed code={} text={}", code, text);
    }
    return saved;
}

bool WbzxEngine::deleteUserWord(const std::string &code, const std::string &text)
{
    if (code.empty() || text.empty())
    {
        FREEWB_ERROR("WbzxEngine::deleteUserWord: empty code or text");
        return false;
    }

    if (userDict_.hasUserEntry(code, text))
    {
        if (!userDict_.removeUserEntry(code, text))
        {
            FREEWB_ERROR("WbzxEngine::deleteUserWord: remove user entry failed code={} text={}", code, text);
            return false;
        }
        (void)userDict_.removeDeletedEntry(code, text);
    }
    else if (userDict_.isDeleted(code, text))
    {
        return true;
    }
    else if (!mbTable_.hasEntry(code, text))
    {
        FREEWB_ERROR("WbzxEngine::deleteUserWord: entry not found in user dict or main table code={} text={}", code, text);
        return false;
    }
    else if (!userDict_.markDeleted(code, text))
    {
        return true;
    }

    const bool saved = userDict_.save();
    userDict_.reload();
    if (!saved)
    {
        FREEWB_ERROR("WbzxEngine::deleteUserWord: save failed code={} text={}", code, text);
    }
    return saved;
}

bool WbzxEngine::shouldProcessKey(const char *key) const
{
    return mbTable_.strInputCode().find(key) != std::string::npos;
}

bool WbzxEngine::isExactDictionaryKey(const std::string &preedit) const
{
    if (preedit.empty())
    {
        return false;
    }
    if (userDict_.contains(preedit))
    {
        return true;
    }
    for (std::size_t i = 0; i < result_.texts.size(); ++i)
    {
        const std::string &fullCode = (i < result_.fullCodes.size()) ? result_.fullCodes[i] : std::string{};
        if (fullCode == preedit)
        {
            return true;
        }
    }
    return false;
}

} // namespace freewb
