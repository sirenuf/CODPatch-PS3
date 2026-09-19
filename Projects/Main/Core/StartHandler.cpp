#include "StartHandler.hpp"
#include "Games/FindActiveGame.hpp"

#define CP_VERSION "v1.0.2-beta"

sys_ppu_thread_t gVshMenuPpuThreadId = SYS_PPU_THREAD_ID_INVALID;

void MainThread(u64 arg)
{
    // Wait for XMB (startup reasons)
    while (!paf::View::Find("explore_plugin") && vsh::GetGameProcessId() <= 0)
        libpsutil::sleep(1000);

    printf("[CODPatch] version %s loaded.\n", CP_VERSION);

    g_FindActiveGame.Initialize();

    sys_ppu_thread_exit(0);
}

void MainThreadStop(u64 arg)
{
    printf("[CODPatch] unload signal received.\n");
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

