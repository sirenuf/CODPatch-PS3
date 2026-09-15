#include "LoaderEntry.hpp"

#include "Utils/LoaderCommon.hpp"
#include "Utils/Globals.hpp"

void MainThread(u64 _arg)
{
    printf("[CODPatch Loader] Current PID: 0x%08X\n", (u32)sys_process_getpid());

    Globals::Game gameType = LoaderCommon::GetGameType();
    bool patchGame = true;
    LoaderCommon::HandleGames(gameType, patchGame);
    
    // This thread's TLS block is a static buffer owned by module_start, not one
    // liblv2 handed out. liblv2's sys_ppu_thread_exit() would try to free it
    // (it does free(r13 - 0x7030)) and wreck the process heap, so exit via the
    // raw syscall instead.
    _sys_ppu_thread_exit(0);
}

void MainThreadStop(u64 arg)
{
    printf("[CODPatch Loader] Unloading module\n");

    // Prevent unload too fast (give time to other threads to finish)
    sys_ppu_thread_yield();
    libpsutil::sleep(1000);

    if (mainThreadId != SYS_PPU_THREAD_ID_INVALID)
    {
        u64 exitCode;
        sys_ppu_thread_join(mainThreadId, &exitCode);
    }

    // Same as MainThread: TLS is ours, so liblv2 must not free it.
    _sys_ppu_thread_exit(0);
}
