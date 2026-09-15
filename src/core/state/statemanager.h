#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ifreewb.h"
#include "keysym.h"
#include "types.h"

namespace freewb
{

class Freewb;
class StateManager;

class IState
{
public:
    virtual ~IState() = default;

    virtual bool processKey(FreewbKeySym keysym, FreewbKeyState state) = 0;
    virtual std::vector<std::string> uninterestedEngines() const = 0;
    virtual void cancel() = 0;
};

/** 保留 BMP 汉字（含扩展 A），去掉其余字符。造词/删词共用。 */
std::string filterNonHanziContent(const std::string &text);

class IdleState : public IState, public IFreewb
{
public:
    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    std::vector<std::string> uninterestedEngines() const override;
    void cancel() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;
};

class AddUserPhraseState : public IState, public IFreewb
{
public:
    AddUserPhraseState(StateManager *manager, Freewb *freewb);

    bool beginFromHistory();
    bool beginFromClipboard();

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    std::vector<std::string> uninterestedEngines() const override;
    void cancel() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

private:
    static std::string utf8SuffixSkipChars(const std::string &text, int skipChars);
    void refreshPhrase();
    void updatePhraseInfoToUI();

private:
    bool available_ = true;
    StateManager *manager_ = nullptr;
    Freewb *freewb_ = nullptr;
    std::string originalText_;   // 造词源串
    std::string wordText_;       // 词组文本
    std::string wordCode_;       // 词组编码
    int phraseLen_ = 2;          // 词组长度
    int sourceCharCount_ = 0;    // 源串 UTF-8 字数
    bool fromClipboard_ = false; // 是否从剪贴板获取文本
    static constexpr int kPhraseMaxLength = 128;
    static constexpr int kDefaultPhraseLen = 2;
};

class DeleteUserPhraseState : public IState, public IFreewb
{
public:
    DeleteUserPhraseState(StateManager *manager, Freewb *freewb);

    bool begin(const std::string &wordText, const std::string &wordCode);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    std::vector<std::string> uninterestedEngines() const override;
    void cancel() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

private:
    bool available_ = true;
    StateManager *manager_ = nullptr;
    Freewb *freewb_ = nullptr;
    std::string wordText_;
    std::string wordCode_;
};

/** 临时英文 + 快捷命令（dos. / aa. / dd. 等） */
class TempEnglishState : public IState, public IFreewb
{
public:
    TempEnglishState(StateManager *manager, Freewb *freewb);

    bool begin(const std::string &commandPrefix);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    std::vector<std::string> uninterestedEngines() const override;
    void cancel() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

private:
    using QuickCommandHandler = std::function<bool()>;

    void registerQuickCommands();
    bool dispatchQuickCommand(const std::string &commandBodyWithoutPrefix);

    bool handleAddPhrase();
    bool handleDeletePhrase();
    bool handleDictQuery();
    bool handleToggleChttrans();
    bool handleToggleCharSet();
    bool handleSwitchVirtualKeyboard();

    /** 按 inputCode 刷新临时英文快捷格式化候选。 */
    void refreshQuickFormatCandidates();

private:
    bool available_ = true;
    StateManager *manager_ = nullptr;
    Freewb *freewb_ = nullptr;
    FreewbKeySym secondRecodeKey_ = FreewbKey_None;
    FreewbKeySym thirdRecodeKey_ = FreewbKey_None;
    std::unordered_map<std::string, QuickCommandHandler> quickCommands_;
};

/** 临时拼音：进入时切到五笔拼音引擎，退出时还原。 */
class TempPinyinState : public IState, public IFreewb
{
public:
    TempPinyinState(StateManager *manager, Freewb *freewb);

    bool begin(const std::string &lead);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    std::vector<std::string> uninterestedEngines() const override;
    void cancel() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

private:
    void refreshCandidates();
    void restoreEngine();

private:
    bool available_ = true;
    StateManager *manager_ = nullptr;
    Freewb *freewb_ = nullptr;
    std::string restoreEngineName_;
};

class StateManager
{
public:
    explicit StateManager(Freewb *freewb);

    bool isUserPhraseState() const
    {
        return current_ == &add_ || current_ == &del_;
    }

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);

    bool enterAddPhraseState(bool useClipboardText = false);
    bool enterDeletePhraseState(const std::string &wordText, const std::string &wordCode = "");
    bool enterTempEnglishState(const std::string &commandPrefix);
    bool enterTempPinyinState(const std::string &lead);

    void enterIdleState()
    {
        current_ = &idle_;
    }

    void reset();

private:
    friend class TempEnglishState;
    friend class TempPinyinState;
    friend class AddUserPhraseState;
    friend class DeleteUserPhraseState;

    bool isTempEnglish() const
    {
        return current_ == &tempEnglish_;
    }

private:
    Freewb *freewb_ = nullptr;
    IdleState idle_;
    AddUserPhraseState add_;
    DeleteUserPhraseState del_;
    TempEnglishState tempEnglish_;
    TempPinyinState tempPinyin_;
    IState *current_;
};

} // namespace freewb

#endif
