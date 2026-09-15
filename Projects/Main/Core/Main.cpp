#include <cellstatus.h>
#include <sys/prx.h>

#include <vsh/newDelete.hpp>

#include "Utils/Types.hpp"
#include "Utils/SystemCalls.hpp"
#include "Core/StartHandler.hpp"

// Hides these PS3 SDK macros from intellisense so it shuts the fuck up
#ifndef __INTELLISENSE__
SYS_MODULE_INFO(CODPatch, 0, 1, 1);
SYS_MODULE_START(module_start);
SYS_MODULE_STOP(module_stop);
#endif

CDECL_BEGIN
int module_start(unsigned int args, void* argp)
{
    sys_ppu_thread_create(&gVshMenuPpuThreadId, MainThread, 0, 1059, 4096, SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_MainThread");

    // Exit thread using directly the syscall and not the user mode library or else we will crash
    // System freezes without this.
    _sys_ppu_thread_exit(0);
    return SYS_PRX_RESIDENT;
}

int module_stop(unsigned int args, void* argp)
{
    sys_ppu_thread_t stopPpuThreadId;
    int ret = sys_ppu_thread_create(&stopPpuThreadId, MainThreadStop, 0, 2816, 1024, SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_MainThreadStop");

    if (ret == SUCCEEDED)
    {
        u64 exitCode;
        sys_ppu_thread_join(stopPpuThreadId, &exitCode);
    }

    libpsutil::sleep(5);

    // unloading prx from memory
    sys_prx_id_t prxId = _sys_prx_get_my_module_id();
    u64 meminfo[5]{ 0x28, 2, 0, 0, 0 };
    _sys_prx_stop_module(prxId, 0, meminfo, NULL, 0, NULL);

    // Exit thread using directly the syscall and not the user mode library or else we will crash
    _sys_ppu_thread_exit(0);

    return 0;
}
CDECL_END