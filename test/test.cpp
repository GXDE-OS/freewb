#include <iostream>
#include <string>

#include "sdbus_proxy.h"
#include "log.h"
#include "types.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    FreewbLog log("/tmp/freewb-test.log");
    FREEWB_WARN("proxy demo started");

    freewb::ipc::SDBusProxy proxy(nullptr, 0);

    int cbSelectCount = 0;
    int cbPageUpCount = 0;
    int cbPageDownCount = 0;
    int cbReloadCount = 0;

    freewb::ipc::DBusCallbacks callbacks;
    callbacks.onSelectCandidate = [&](int index)
    {
        ++cbSelectCount;
        std::cout << "[callback] onSelectCandidate index=" << index << " (count=" << cbSelectCount << ")\n";
        FREEWB_WARN("callback onSelectCandidate index={} count={}", index, cbSelectCount);
    };
    callbacks.onPageUp = [&]()
    {
        ++cbPageUpCount;
        std::cout << "[callback] onPageUp (count=" << cbPageUpCount << ")\n";
        FREEWB_WARN("callback onPageUp count={}", cbPageUpCount);
    };
    callbacks.onPageDown = [&]()
    {
        ++cbPageDownCount;
        std::cout << "[callback] onPageDown (count=" << cbPageDownCount << ")\n";
        FREEWB_WARN("callback onPageDown count={}", cbPageDownCount);
    };
    callbacks.onReloadConfig = [&]()
    {
        ++cbReloadCount;
        std::cout << "[callback] onReloadConfig (count=" << cbReloadCount << ")\n";
        FREEWB_WARN("callback onReloadConfig count={}", cbReloadCount);
    };

    const bool bound = proxy.bindDBusCallbacks(callbacks);
    FREEWB_WARN("proxy.bindDBusCallbacks = {}", bound);
    std::cout << "proxy.bindDBusCallbacks=" << (bound ? "true" : "false") << '\n';
    std::cout << "With panel running: pick a candidate / page keys / reload triggers the callbacks above.\n";
    if (!bound)
    {
        std::cout << "bind failed, panel signals may not be received." << '\n';
    }

    freewb::SpotRectPayload spotRect{100, 100, 1, 1};
    freewb::CandidatePayload candidate;
    candidate.labels = {"1.", "2.", "3."};
    candidate.texts = {"ni", "hao", "ma"};
    candidate.attrs = {"", "", ""};
    candidate.hasPrev = false;
    candidate.hasNext = true;
    candidate.cursor = 0;
    candidate.layout = freewb::Horizontal;

    freewb::CandidatePreeditPayload preedit;
    preedit.text = "demo preedit";
    preedit.caret = 4;
    preedit.show = true;

    freewb::CandidateAuxPayload aux;
    aux.text = "demo aux";
    aux.show = true;

    std::cout << "0 show candidate+preedit+aux  1 hide input UI  2 candidate only\n"
                 "3 show toolbar (Freewb)  4 hide toolbar (us)\n"
                 "5 register im+half+EN punct  6 register im+full+CN punct\n"
                 "c print callback hit counts  q quit\n";

    for (std::string cmd; std::getline(std::cin, cmd);)
    {
        if (cmd == "0")
        {
            proxy.emitUpdateCandidate(spotRect, candidate, preedit, aux);
            FREEWB_WARN("cmd=0 emitUpdateCandidate (full)");
            std::cout << "ok\n";
        }
        else if (cmd == "1")
        {
            freewb::CandidatePayload emptyCand;
            freewb::CandidatePreeditPayload pe{};
            freewb::CandidateAuxPayload ax{};
            pe.show = false;
            ax.show = false;
            proxy.emitUpdateCandidate(spotRect, emptyCand, pe, ax);
            FREEWB_WARN("cmd=1 emitUpdateCandidate (hide)");
            std::cout << "hidden\n";
        }
        else if (cmd == "2")
        {
            freewb::CandidatePreeditPayload pe{};
            freewb::CandidateAuxPayload ax{};
            proxy.emitUpdateCandidate(spotRect, candidate, pe, ax);
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
            std::cout << "[stats] onSelectCandidate=" << cbSelectCount << " onPageUp=" << cbPageUpCount << " onPageDown=" << cbPageDownCount << " onReloadConfig=" << cbReloadCount << '\n';
            FREEWB_WARN("callback stats select={} up={} down={} reload={}", cbSelectCount, cbPageUpCount, cbPageDownCount, cbReloadCount);
        }
        else if (cmd == "q" || cmd == "quit")
        {
            break;
        }
        else if (!cmd.empty())
        {
            std::cout << "unknown: " << cmd << '\n';
        }
    }

    FREEWB_WARN("proxy demo finished");
    return 0;
}
