#include "StartHandler.hpp"
#include "Games/FindActiveGame.hpp"

#include "Games/CODCommon.hpp"

sys_ppu_thread_t gVshMenuPpuThreadId = SYS_PPU_THREAD_ID_INVALID;

void MainThread(u64 arg)
{
    // Freeze while XMB is not loaded, or if game is not running.
    do
        libpsutil::sleep(1000);
    while (!paf::View::Find("explore_plugin") && vsh::GetGameProcessId() <= 0);

    g_FindActiveGame = FindActiveGame();

    vshtask::Notify("CODPatch loaded!");

    sys_ppu_thread_exit(0);
}

void MainThreadStop(u64 arg)
{
    vshtask::Notify("Unloading CODPatch!");
    // Prevent unload too fast (give time to other threads to finish)
    sys_ppu_thread_yield();
    libpsutil::sleep(1000);

    g_FindActiveGame.Shutdown();

    if (gVshMenuPpuThreadId != SYS_PPU_THREAD_ID_INVALID)
    {
        u64 exitCode;
        sys_ppu_thread_join(gVshMenuPpuThreadId, &exitCode);
    }

    sys_ppu_thread_exit(0);
}

