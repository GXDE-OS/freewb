#ifndef USERPHRASE_H
#define USERPHRASE_H

#include <functional>
#include <string>

#include "keysym.h"
#include "idbus.h"

namespace freewb
{

/** UTF-8 词组 → 五笔造词编码；失败返回空串。 */
using CalculateWubiPhraseCodeCallback = std::function<std::string(const std::string &phraseUtf8)>;
/** 在线造词：取最近 @p charCount 个上屏单字组成的词组。 */
using PhraseFromHistoryCallback = std::function<std::string(int charCount)>;

class IUserPhraseState
{
public:
    virtual ~IUserPhraseState() = default;

    virtual bool processKey(FreewbKeySym keysym, FreewbKeyState state) = 0;
    virtual void cancel() = 0;
};

class IdleState : public IUserPhraseState
{
public:
    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    void cancel() override;
};

class AddUserPhraseState : public IUserPhraseState
{
public:
    AddUserPhraseState(ipc::IDBus *dbusProxy, PhraseFromHistoryCallback phraseFromHistory,
                       CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode);

    bool beginFromHistory();
    bool beginFromClipboard();

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    void cancel() override;

private:
    static constexpr int kPhraseMaxLength = 128;
    static constexpr int kDefaultPhraseLen = 2;

    static std::string utf8SuffixSkipChars(const std::string &text, int skipChars);
    /** 过滤非汉字内容（与旧版 hzLastInput 只记候选汉字一致）。 */
    static std::string filterNonHanziContent(const std::string &text);

    void refreshPhrase();
    void updatePhraseInfoToUI();

    ipc::IDBus *dbusProxy_;
    std::string originalText_; // 造词源串
    std::string wordText_; // 词组文本
    std::string wordCode_; // 词组编码
    int phraseLen_ = 2; // 词组长度
    int sourceCharCount_ = 0; // 源串 UTF-8 字数
    bool fromClipboard_ = false; // 是否从剪贴板获取文本
    CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode_;
    PhraseFromHistoryCallback phraseFromHistoryCallback_;
};

class DeleteUserPhraseState : public IUserPhraseState
{
public:
    DeleteUserPhraseState(ipc::IDBus *dbusProxy, CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode);

    bool begin(const std::string &wordText);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state) override;
    void cancel() override;

private:
    ipc::IDBus *dbusProxy_;
    std::string wordText_;
    std::string wordCode_;
    CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode_;
};

class UserPhrase
{
public:
    UserPhrase(ipc::IDBus *dbusProxy, CalculateWubiPhraseCodeCallback calculateWubiPhraseCode,
               PhraseFromHistoryCallback phraseFromHistory);

    bool isActive() const { return current_ != &idle_; }

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);

    void enterAddPhraseState(bool useClipboardText = false);
    void enterDeletePhraseState(const std::string &wordText);
    void reset();

private:
    CalculateWubiPhraseCodeCallback calculateWubiPhraseCodeCallback_;
    IdleState idle_;
    AddUserPhraseState add_;
    DeleteUserPhraseState del_;
    IUserPhraseState *current_;
};

} // namespace freewb

#endif // USERPHRASE_H
