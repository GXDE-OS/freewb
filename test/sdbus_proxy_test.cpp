#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <fcitx-utils/event.h>

#include "log.h"
#include "sdbus_proxy.h"
#include "types.h"

namespace
{

/** 与旧 emitUpdateCandidate 等价的调用顺序，供演示/测试。 */
void emitCandidateFrame(freewb::ipc::SDBusProxy &proxy, const freewb::SpotRectPayload &spot, const freewb::CandidatePayload &cand,
                        const freewb::PreeditPayload &preedit, const freewb::CandidateAuxPayload &aux)
{
    proxy.callPanelUpdateSpotRect(spot);
    proxy.callPanelUpdateCandidate(cand);
    proxy.callPanelUpdatePreeditText(preedit);
    proxy.callPanelUpdatePreeditCaret(preedit.caret);
    proxy.callPanelUpdateAux(aux);
}

struct DemoState
{
    fcitx::EventLoop *loop = nullptr;
    freewb::ipc::SDBusProxy *proxy = nullptr;
    std::string stdinBuf;

    int cbSignalCount = 0;
    int cbUnknownCount = 0;
    int cbSelectCount = 0;
    int cbPageUpCount = 0;
    int cbPageDownCount = 0;
    int cbReloadCount = 0;

    freewb::SpotRectPayload spotRect{100, 100, 1, 1};
    freewb::CandidatePayload candidate;
    freewb::PreeditPayload preedit;
    freewb::CandidateAuxPayload aux;
};

DemoState *gDemoState = nullptr;

std::vector<std::string> splitTokens(const std::string &line)
{
    std::istringstream iss(line);
    std::vector<std::string> out;
    std::string token;
    while (iss >> token)
    {
        out.push_back(token);
    }
    return out;
}

void initDemoPayloads(DemoState &s)
{
    s.candidate.fullCodes = {};
    s.candidate.texts = {"ni", "hao", "ma"};
    s.candidate.prompts = {"", "", ""};
    s.candidate.hasPrev = false;
    s.candidate.hasNext = true;
    s.candidate.cursor = 0;
    s.candidate.layout = freewb::Horizontal;

    s.preedit.text = "demo preedit";
    s.preedit.caret = 4;
    s.preedit.show = true;

    s.aux.text = "demo aux";
    s.aux.show = true;
}

void onDBusSignalCallback(const char *member, int index)
{
    if (!member || !*member)
    {
        return;
    }
    DemoState *state = gDemoState;
    if (!state)
    {
        return;
    }
    ++state->cbSignalCount;
    if (std::strcmp(member, "SelectCandidate") == 0)
    {
        ++state->cbSelectCount;
        std::cout << "[callback] SelectCandidate index=" << index << " (count=" << state->cbSelectCount << ")\n";
        FREEWB_WARN("callback SelectCandidate index={} count={}", index, state->cbSelectCount);
    }
    else if (std::strcmp(member, "LookupTablePageUp") == 0)
    {
        ++state->cbPageUpCount;
        std::cout << "[callback] LookupTablePageUp (count=" << state->cbPageUpCount << ")\n";
        FREEWB_WARN("callback LookupTablePageUp count={}", state->cbPageUpCount);
    }
    else if (std::strcmp(member, "LookupTablePageDown") == 0)
    {
        ++state->cbPageDownCount;
        std::cout << "[callback] LookupTablePageDown (count=" << state->cbPageDownCount << ")\n";
        FREEWB_WARN("callback LookupTablePageDown count={}", state->cbPageDownCount);
    }
    else if (std::strcmp(member, "ReloadConfig") == 0)
    {
        ++state->cbReloadCount;
        std::cout << "[callback] ReloadConfig (count=" << state->cbReloadCount << ")\n";
        FREEWB_WARN("callback ReloadConfig count={}", state->cbReloadCount);
    }
    else
    {
        ++state->cbUnknownCount;
        std::cout << "[callback] unknown signal: " << member << '\n';
        FREEWB_WARN("callback unknown panel signal: {}", member);
    }
}

bool setStdinNonBlocking()
{
    const int fd = STDIN_FILENO;
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

void handleCommand(DemoState &s, const std::string &cmd)
{
    freewb::ipc::SDBusProxy &proxy = *s.proxy;
    const std::vector<std::string> tokens = splitTokens(cmd);

    if (!tokens.empty() && (tokens[0] == "settings" || tokens[0] == "s"))
    {
        if (tokens.size() == 1 || tokens[1] == "help")
        {
            std::cout << "settings|s <action> [args]\n"
                      << "  dict <text>\n"
                      << "  add <flg> <wordText> <wordCode>\n"
                      << "  del <flg> <wordText> <wordCode>\n"
                      << "  switch_input_mode <imState>\n"
                      << "  switch_skin | switch_vk <flg>\n"
                      << "  switch_smart_punc | switch_charset\n"
                      << "  switch_recode_proof | switch_uncommon <wordText> <flg>\n"
                      << "  switch_chttrans | switch_cap_state\n"
                      << "  open_sys_conf | show_version | open_prof_conf\n"
                      << "  mod_quick | mod_user | mod_wubi | mod_pinyin\n"
                      << "  open_conf_dir | close_vk | switch_table\n"
                      << "  switch_char_width switch_punctuation\n"
                      << "  clipboard\n";
            return;
        }

        const std::string &action = tokens[1];
        auto toInt = [](const std::string &v, int fallback = 0) -> int
        {
            try
            {
                return std::stoi(v);
            }
            catch (...)
            {
                return fallback;
            }
        };

        if (action == "dict" && tokens.size() >= 3)
            proxy.callDictQueryMethod(tokens[2]);
        else if (action == "add" && tokens.size() >= 5)
            proxy.callAddUsrParseMethod(toInt(tokens[2]), tokens[4], tokens[3]);
        else if (action == "del" && tokens.size() >= 5)
            proxy.callDeleteUsrParseMethod(toInt(tokens[2]), tokens[4], tokens[3]);
        else if (action == "switch_input_mode" && tokens.size() >= 3)
            proxy.callPanelSwitchInputModeMethod(tokens[2]);
        else if (action == "switch_skin")
            proxy.callSwitchSkinMethod();
        else if (action == "switch_vk" && tokens.size() >= 3)
            proxy.callSwitchVirtualKeyboardModeMethod(toInt(tokens[2]));
        else if (action == "switch_charset")
            proxy.callPanelSwitchCharSetMethod();
        else if (action == "switch_recode_proof")
            proxy.callSwitchRecodeProofMethod();
        else if (action == "switch_uncommon" && tokens.size() >= 4)
            proxy.callSwitchUncommonParseStateMethod(tokens[2], toInt(tokens[3]));
        else if (action == "switch_chttrans")
            proxy.callPanelSwitchChttransMethod();
        else if (action == "open_sys_conf")
            proxy.callOpenUiSettingMethod();
        else if (action == "show_version")
            proxy.callShowVersionInfoMethod();
        else if (action == "open_prof_conf")
            proxy.callOpenProfessionalSettingMethod();
        else if (action == "mod_quick")
            proxy.callModQuickTableMethod();
        else if (action == "mod_user")
            proxy.callModUserTableMethod();
        else if (action == "mod_wubi")
            proxy.callModWubiTableMethod();
        else if (action == "mod_pinyin")
            proxy.callModPinyinTableMethod();
        else if (action == "open_conf_dir")
            proxy.callOpenConfDirMethod();
        else if (action == "close_vk")
            proxy.callCloseVkBoardMethod();
        else if (action == "switch_table")
            proxy.callSwitchTableMethod();
        else if (action == "switch_char_width")
            proxy.callPanelSwitchCharWidthMethod();
        else if (action == "switch_punctuation")
            proxy.callPanelSwitchPunctuationModeMethod();
        else if (action == "clipboard")
        {
            const std::string text = proxy.callGetClipboardMethod();
            std::cout << "clipboard: " << text << '\n';
            return;
        }
        else if (action == "switch_cap_state")
            proxy.callPanelToggleCapsStateMethod();
        else
        {
            std::cout << "unknown settings action, use: s help\n";
            return;
        }

        FREEWB_WARN("settings command executed: {}", cmd);
        std::cout << "ok\n";
        return;
    }

    if (cmd == "0")
    {
        FREEWB_WARN("cmd=0 before emitCandidateFrame: cand texts={} prompts={} preedit=\"{}\" aux=\"{}\"",
                    s.candidate.texts.size(), s.candidate.prompts.size(), s.preedit.text, s.aux.text);
        emitCandidateFrame(proxy, s.spotRect, s.candidate, s.preedit, s.aux);
        FREEWB_WARN("cmd=0 after emitCandidateFrame (full)");
        std::cout << "ok\n";
    }
    else if (cmd == "1")
    {
        freewb::CandidatePayload emptyCand;
        freewb::PreeditPayload pe{};
        freewb::CandidateAuxPayload ax{};
        pe.show = false;
        ax.show = false;
        emitCandidateFrame(proxy, s.spotRect, emptyCand, pe, ax);
        FREEWB_WARN("cmd=1 emitCandidateFrame (hide)");
        std::cout << "hidden\n";
    }
    else if (cmd == "2")
    {
        freewb::PreeditPayload pe{};
        freewb::CandidateAuxPayload ax{};
        emitCandidateFrame(proxy, s.spotRect, s.candidate, pe, ax);
        FREEWB_WARN("cmd=2 emitCandidateFrame (candidate only)");
        std::cout << "candidate shown\n";
    }
    else if (cmd == "3")
    {
        proxy.callPanelShowToolbar();
        FREEWB_WARN("cmd=3 emitShowToolbar");
        std::cout << "UpdateProperty Freewb\n";
    }
    else if (cmd == "4")
    {
        proxy.callPanelHideToolbar();
        FREEWB_WARN("cmd=4 emitHideToolbar");
        std::cout << "UpdateProperty us\n";
    }
    else if (cmd == "5")
    {
        freewb::ToolbarPropertiesPayload im;
        im.uniqueName = "Freewb";
        im.name = "freewb-test";
        proxy.callPanelUpdateProperties(im);

        freewb::ToolbarPropertiesPayload fw;
        fw.uniqueName = "fullwidth";
        fw.shortDescription = "Half";
        fw.longDescription = "Half width";
        fw.active = false;
        proxy.callPanelUpdateProperties(fw);

        freewb::ToolbarPropertiesPayload punc;
        punc.uniqueName = "punc";
        punc.shortDescription = "EN";
        punc.longDescription = "English punctuation";
        punc.active = false;
        proxy.callPanelUpdateProperties(punc);

        FREEWB_WARN("cmd=5 emitUpdateProperties x3 (half/EN punct)");
        std::cout << "three RegisterProperties signals sent\n";
    }
    else if (cmd == "6")
    {
        freewb::ToolbarPropertiesPayload im;
        im.uniqueName = "Freewb";
        im.name = "freewb-test";
        proxy.callPanelUpdateProperties(im);

        freewb::ToolbarPropertiesPayload fw;
        fw.uniqueName = "fullwidth";
        fw.shortDescription = "Full";
        fw.longDescription = "Full width";
        fw.active = true;
        proxy.callPanelUpdateProperties(fw);

        freewb::ToolbarPropertiesPayload punc;
        punc.uniqueName = "punc";
        punc.shortDescription = "CN";
        punc.longDescription = "Chinese punctuation";
        punc.active = true;
        proxy.callPanelUpdateProperties(punc);

        FREEWB_WARN("cmd=6 emitUpdateProperties x3 (full/CN punct)");
        std::cout << "three RegisterProperties signals sent\n";
    }
    else if (cmd == "7")
    {
        // settings no-arg methods smoke test
        proxy.callSwitchSkinMethod();
        proxy.callPanelSwitchCharSetMethod();
        proxy.callOpenUiSettingMethod();
        proxy.callShowVersionInfoMethod();
        proxy.callOpenProfessionalSettingMethod();
        proxy.callModQuickTableMethod();
        proxy.callModUserTableMethod();
        proxy.callModWubiTableMethod();
        proxy.callModPinyinTableMethod();
        proxy.callOpenConfDirMethod();
        proxy.callCloseVkBoardMethod();
        proxy.callSwitchTableMethod();
        proxy.callPanelToggleCapsStateMethod();
        FREEWB_WARN("cmd=7 settings no-arg smoke methods sent");
        std::cout << "settings no-arg methods sent\n";
    }
    else if (cmd == "8")
    {
        // settings arg methods smoke test（callAddUsrParseMethod(flg, wordCode, wordText)）
        proxy.callAddUsrParseMethod(0, "cs", u8"\u6d4b\u8bd5\u8bcd");
        proxy.callDeleteUsrParseMethod(0, "cs", u8"\u6d4b\u8bd5\u8bcd");
        proxy.callDictQueryMethod(u8"\u6d4b\u8bd5");
        proxy.callPanelSwitchInputModeMethod("engine:wbpy");
        proxy.callSwitchVirtualKeyboardModeMethod(0);
        proxy.callSwitchRecodeProofMethod();
        proxy.callSwitchUncommonParseStateMethod(u8"\u6d4b\u8bd5\u8bcd", 1);
        proxy.callPanelSwitchChttransMethod();
        proxy.callPanelSwitchCharWidthMethod();
        proxy.callPanelSwitchPunctuationModeMethod();
        FREEWB_WARN("cmd=8 settings arg methods sent");
        std::cout << "settings arg methods sent\n";
    }
    else if (cmd == "9")
    {
        const std::string text = proxy.callGetClipboardMethod();
        FREEWB_WARN("cmd=9 getClipboard size={}", text.size());
        std::cout << "clipboard: " << text << '\n';
    }
    else if (cmd == "c")
    {
        std::cout << "[stats] onDBusSignal=" << s.cbSignalCount << " unknown=" << s.cbUnknownCount
                  << " onSelectCandidate=" << s.cbSelectCount << " onPageUp=" << s.cbPageUpCount
                  << " onPageDown=" << s.cbPageDownCount << " onReloadConfig=" << s.cbReloadCount << '\n';
        FREEWB_WARN("callback stats signal={} unknown={} select={} up={} down={} reload={}", s.cbSignalCount, s.cbUnknownCount,
                    s.cbSelectCount, s.cbPageUpCount, s.cbPageDownCount, s.cbReloadCount);
    }
    else if (cmd == "q" || cmd == "quit")
    {
        s.loop->exit();
    }
    else if (!cmd.empty())
    {
        std::cout << "unknown: " << cmd << '\n';
    }
}

/** stdin 与 sd-bus 同处于 Fcitx5 EventLoop：避免 getline 阻塞导致面板信号迟迟不被处理。 */
bool onStdinReadable(DemoState &s, int fd)
{
    char chunk[512];
    for (;;)
    {
        const ssize_t n = read(fd, chunk, sizeof chunk);
        if (n > 0)
        {
            s.stdinBuf.append(chunk, static_cast<size_t>(n));
            continue;
        }
        if (n == 0)
        {
            s.loop->exit();
            return false;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            break;
        }
        std::cerr << "stdin read: " << std::strerror(errno) << '\n';
        s.loop->exit();
        return false;
    }

    for (;;)
    {
        const auto nl = s.stdinBuf.find('\n');
        if (nl == std::string::npos)
        {
            break;
        }
        std::string line = s.stdinBuf.substr(0, nl);
        s.stdinBuf.erase(0, nl + 1U);
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        handleCommand(s, line);
    }
    return true;
}

} // namespace

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    FreewbLog log("/tmp/freewb-test.log");
    FREEWB_WARN("proxy demo started (Fcitx5 EventLoop: stdin + D-Bus 同环)");

    fcitx::EventLoop eventLoop;
    if (std::strcmp(eventLoop.implementation(), "sd-event") != 0)
    {
        std::cerr << "freewb-sdbusproxy-test needs Fcitx5 Utils built with sd-event (got: " << eventLoop.implementation()
                  << "). libuv backend cannot attach sd-bus here.\n";
        return 1;
    }
    void *const nativeLoop = eventLoop.nativeHandle();
    if (!nativeLoop)
    {
        std::cerr << "Fcitx5 EventLoop nativeHandle() is null.\n";
        return 1;
    }

    DemoState state;
    gDemoState = &state;
    state.loop = &eventLoop;

    freewb::ipc::SDBusProxy proxy(nativeLoop, 0);
    state.proxy = &proxy;

    if (!proxy.available())
    {
        std::cerr << "SDBusProxy unavailable (bus open or sd_bus_attach_event failed).\n";
        return 1;
    }

    initDemoPayloads(state);

    const bool bound = proxy.bindDBusSignalCallback(&onDBusSignalCallback);
    FREEWB_WARN("proxy.bindDBusSignalCallback = {}", bound);
    std::cout << "proxy.bindDBusSignalCallback=" << (bound ? "true" : "false") << '\n';
    std::cout << "stdin is non-blocking; same Fcitx5 EventLoop drives D-Bus (panel clicks should log without typing a command "
                 "first).\n";
    std::cout << "With panel running: panel signals are passed through to one callback; callback decides how to parse/handle.\n";
    if (!bound)
    {
        std::cout << "bind failed, panel signals may not be received.\n";
    }

    std::cout << "0 show candidate+preedit+aux  1 hide input UI  2 candidate only\n"
                 "3 show toolbar (Freewb)  4 hide toolbar (us)\n"
                 "5 register im+half+EN punct  6 register im+full+CN punct\n"
                 "7 settings no-arg smoke  8 settings arg smoke  9 get clipboard\n"
                 "settings <action> [args]  (type: settings help)\n"
                 "c print callback hit counts  q quit\n";

    if (!setStdinNonBlocking())
    {
        std::cerr << "fcntl(STDIN) O_NONBLOCK failed.\n";
        return 1;
    }

    std::unique_ptr<fcitx::EventSourceIO> stdinWatch =
        eventLoop.addIOEvent(STDIN_FILENO, fcitx::IOEventFlag::In,
                             [&state](fcitx::EventSourceIO * /*src*/, int fd, fcitx::IOEventFlags /*flags*/)
                             { return onStdinReadable(state, fd); });

    (void)stdinWatch;
    eventLoop.exec();

    FREEWB_WARN("proxy demo finished");
    return 0;
}
