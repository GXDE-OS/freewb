#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include <fcitx-utils/event.h>

#include "sdbus_proxy.h"
#include "log.h"
#include "types.h"

namespace
{

struct DemoState
{
    fcitx::EventLoop *loop = nullptr;
    freewb::ipc::SDBusProxy *proxy = nullptr;
    std::string stdinBuf;

    int cbSelectCount = 0;
    int cbPageUpCount = 0;
    int cbPageDownCount = 0;
    int cbReloadCount = 0;

    freewb::SpotRectPayload spotRect{100, 100, 1, 1};
    freewb::CandidatePayload candidate;
    freewb::CandidatePreeditPayload preedit;
    freewb::CandidateAuxPayload aux;
};

void initDemoPayloads(DemoState &s)
{
    s.candidate.labels = {"1.", "2.", "3."};
    s.candidate.texts = {"ni", "hao", "ma"};
    s.candidate.attrs = {"", "", ""};
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

    if (cmd == "0")
    {
        FREEWB_WARN(
            "cmd=0 before emitUpdateCandidate: cand labels={} texts={} preedit=\"{}\" aux=\"{}\"",
            s.candidate.labels.size(),
            s.candidate.texts.size(),
            s.preedit.text,
            s.aux.text);
        proxy.emitUpdateCandidate(s.spotRect, s.candidate, s.preedit, s.aux);
        FREEWB_WARN("cmd=0 after emitUpdateCandidate (full)");
        std::cout << "ok\n";
    }
    else if (cmd == "1")
    {
        freewb::CandidatePayload emptyCand;
        freewb::CandidatePreeditPayload pe{};
        freewb::CandidateAuxPayload ax{};
        pe.show = false;
        ax.show = false;
        proxy.emitUpdateCandidate(s.spotRect, emptyCand, pe, ax);
        FREEWB_WARN("cmd=1 emitUpdateCandidate (hide)");
        std::cout << "hidden\n";
    }
    else if (cmd == "2")
    {
        freewb::CandidatePreeditPayload pe{};
        freewb::CandidateAuxPayload ax{};
        proxy.emitUpdateCandidate(s.spotRect, s.candidate, pe, ax);
        FREEWB_WARN("cmd=2 emitUpdateCandidate (candidate only)");
        std::cout << "candidate shown\n";
    }
    else if (cmd == "3")
    {
        proxy.emitShowToolbar();
        FREEWB_WARN("cmd=3 emitShowToolbar");
        std::cout << "UpdateProperty Freewb\n";
    }
    else if (cmd == "4")
    {
        proxy.emitHideToolbar();
        FREEWB_WARN("cmd=4 emitHideToolbar");
        std::cout << "UpdateProperty us\n";
    }
    else if (cmd == "5")
    {
        freewb::ToolbarPropertiesPayload im;
        im.uniqueName = "Freewb";
        im.name = "freewb-test";
        proxy.emitUpdateProperties(im);

        freewb::ToolbarPropertiesPayload fw;
        fw.uniqueName = "fullwidth";
        fw.shortDescription = "Half";
        fw.longDescription = "Half width";
        fw.active = false;
        proxy.emitUpdateProperties(fw);

        freewb::ToolbarPropertiesPayload punc;
        punc.uniqueName = "punc";
        punc.shortDescription = "EN";
        punc.longDescription = "English punctuation";
        punc.active = false;
        proxy.emitUpdateProperties(punc);

        FREEWB_WARN("cmd=5 emitUpdateProperties x3 (half/EN punct)");
        std::cout << "three RegisterProperties signals sent\n";
    }
    else if (cmd == "6")
    {
        freewb::ToolbarPropertiesPayload im;
        im.uniqueName = "Freewb";
        im.name = "freewb-test";
        proxy.emitUpdateProperties(im);

        freewb::ToolbarPropertiesPayload fw;
        fw.uniqueName = "fullwidth";
        fw.shortDescription = "Full";
        fw.longDescription = "Full width";
        fw.active = true;
        proxy.emitUpdateProperties(fw);

        freewb::ToolbarPropertiesPayload punc;
        punc.uniqueName = "punc";
        punc.shortDescription = "CN";
        punc.longDescription = "Chinese punctuation";
        punc.active = true;
        proxy.emitUpdateProperties(punc);

        FREEWB_WARN("cmd=6 emitUpdateProperties x3 (full/CN punct)");
        std::cout << "three RegisterProperties signals sent\n";
    }
    else if (cmd == "c")
    {
        std::cout << "[stats] onSelectCandidate=" << s.cbSelectCount << " onPageUp=" << s.cbPageUpCount << " onPageDown=" << s.cbPageDownCount << " onReloadConfig=" << s.cbReloadCount << '\n';
        FREEWB_WARN("callback stats select={} up={} down={} reload={}", s.cbSelectCount, s.cbPageUpCount, s.cbPageDownCount, s.cbReloadCount);
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
    state.loop = &eventLoop;

    freewb::ipc::SDBusProxy proxy(nativeLoop, 0);
    state.proxy = &proxy;

    if (!proxy.available())
    {
        std::cerr << "SDBusProxy unavailable (bus open or sd_bus_attach_event failed).\n";
        return 1;
    }

    initDemoPayloads(state);

    freewb::ipc::DBusCallbacks callbacks;
    callbacks.onSelectCandidate = [&state](int index)
    {
        ++state.cbSelectCount;
        std::cout << "[callback] onSelectCandidate index=" << index << " (count=" << state.cbSelectCount << ")\n";
        FREEWB_WARN("callback onSelectCandidate index={} count={}", index, state.cbSelectCount);
    };
    callbacks.onPageUp = [&state]()
    {
        ++state.cbPageUpCount;
        std::cout << "[callback] onPageUp (count=" << state.cbPageUpCount << ")\n";
        FREEWB_WARN("callback onPageUp count={}", state.cbPageUpCount);
    };
    callbacks.onPageDown = [&state]()
    {
        ++state.cbPageDownCount;
        std::cout << "[callback] onPageDown (count=" << state.cbPageDownCount << ")\n";
        FREEWB_WARN("callback onPageDown count={}", state.cbPageDownCount);
    };
    // callbacks.onReloadConfig = [&state]()
    // {
    //     ++state.cbReloadCount;
    //     std::cout << "[callback] onReloadConfig (count=" << state.cbReloadCount << ")\n";
    //     FREEWB_WARN("callback onReloadConfig count={}", state.cbReloadCount);
    // };

    const bool bound = proxy.bindDBusCallbacks(callbacks);
    FREEWB_WARN("proxy.bindDBusCallbacks = {}", bound);
    std::cout << "proxy.bindDBusCallbacks=" << (bound ? "true" : "false") << '\n';
    std::cout << "stdin is non-blocking; same Fcitx5 EventLoop drives D-Bus (panel clicks should log without typing a command first).\n";
    std::cout << "With panel running: pick a candidate / page keys / reload triggers the callbacks above.\n";
    if (!bound)
    {
        std::cout << "bind failed, panel signals may not be received.\n";
    }

    std::cout << "0 show candidate+preedit+aux  1 hide input UI  2 candidate only\n"
                 "3 show toolbar (Freewb)  4 hide toolbar (us)\n"
                 "5 register im+half+EN punct  6 register im+full+CN punct\n"
                 "c print callback hit counts  q quit\n";

    if (!setStdinNonBlocking())
    {
        std::cerr << "fcntl(STDIN) O_NONBLOCK failed.\n";
        return 1;
    }

    std::unique_ptr<fcitx::EventSourceIO> stdinWatch = eventLoop.addIOEvent(
        STDIN_FILENO,
        fcitx::IOEventFlag::In,
        [&state](fcitx::EventSourceIO * /*src*/, int fd, fcitx::IOEventFlags /*flags*/) { return onStdinReadable(state, fd); });

    (void)stdinWatch;
    eventLoop.exec();

    FREEWB_WARN("proxy demo finished");
    return 0;
}
