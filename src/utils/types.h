#ifndef _FREEWB_UTILS_TYPES_H_
#define _FREEWB_UTILS_TYPES_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace freewb
{

enum DictionaryReloadMask : std::uint32_t
{
    DictReloadNone = 0,
    DictReloadUserWord = 1u << 0,
    DictReloadWubiTable = 1u << 1,
    DictReloadPinyinTable = 1u << 2,
    DictReloadAutoPhrase = 1u << 3,
    DictReloadMainTables = DictReloadWubiTable | DictReloadPinyinTable,
    DictReloadAll = DictReloadUserWord | DictReloadWubiTable | DictReloadPinyinTable | DictReloadAutoPhrase,
};

struct SpotRectPayload
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct CandidatePayload
{
    std::vector<std::string> fullCodes;
    std::vector<std::string> texts;
    std::vector<std::string> prompts;
    bool hasPrev = false;
    bool hasNext = false;
    int cursor = -1;

    void clearRows()
    {
        fullCodes.clear();
        texts.clear();
        prompts.clear();
    }
};

struct PreeditPayload
{
    std::string text;
    int caret = 0;
};

struct CandidateAuxPayload
{
    std::string text;
};

struct ToolbarPropertiesPayload
{
    std::string engineName;
    bool traditional = false; // true=繁体
    int charSet = 0;          // 0=GB, 1=GBK
    bool fullWidth = false;   // true=全角
    bool chinesePunc = true;  // true=中文标点
};

struct PanelSignalEvent
{
    const char *member = nullptr;
    int index = 0;
    std::string str0;
    std::string str1;
};

using DBusSignalCallback = std::function<void(const PanelSignalEvent &)>;
using CommitCallback = std::function<void(const std::string &text)>;

} // namespace freewb

#endif
