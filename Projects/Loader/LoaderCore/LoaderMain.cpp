#include <cellstatus.h>
#include <sys/prx.h>

#include "LoaderEntry.hpp"

#ifndef __INTELLISENSE__
SYS_MODULE_INFO(CODPatch_Loader, 0, 1, 1);
SYS_MODULE_START(module_start);
SYS_MODULE_STOP(module_stop);
#endif

// liblv2's 0x30-byte per-thread header, then room for the process TLS image.
// r13 == block + 0x7000 per the PPC64 ABI, and liblv2's header sits just below it.
// One block per thread, two threads sharing one would stomp each other's TLS.
static u8 s_loaderTls[0x30 + 0x1000] __attribute__((aligned(16)));
static u8 s_stopTls[0x30 + 0x1000]   __attribute__((aligned(16)));

#define SC_PPU_THREAD_CREATE 52
#define SC_PPU_THREAD_START  53

int _sys_ppu_thread_create(sys_ppu_thread_t* tid, sys_ppu_thread_init* init, u64 arg, u64 unk,
    int prio, size_t stacksize, u64 flags, const char* name)
{
    system_call_8(SC_PPU_THREAD_CREATE, (u32)tid, (u32)init, arg, unk,
        prio, stacksize, flags, (u32)name);
    return_to_user_prog(int);
}

int sys_ppu_thread_start(sys_ppu_thread_t tid)
{
    system_call_1(SC_PPU_THREAD_START, tid);
    return_to_user_prog(int);
}

sys_ppu_thread_t mainThreadId = SYS_PPU_THREAD_ID_INVALID;


#define ARGS 0
#define PRIORITY 1059
#define STACK_SIZE 4096

CDECL_BEGIN
int module_start(unsigned int args, void* argp)
{
    sys_ppu_thread_init init;
    init.entry        = (sys_addr_t)(uintptr_t)MainThread;              // OPD, as before
    init.tls_mem_addr = (sys_addr_t)((uintptr_t)s_loaderTls + 0x7030);

    if (_sys_ppu_thread_create(&mainThreadId, &init, (u64)argp, ARGS, PRIORITY, STACK_SIZE,
                               SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_Loader") == SUCCEEDED)
        sys_ppu_thread_start(mainThreadId);

    _sys_ppu_thread_exit(0);
    return SYS_PRX_RESIDENT;
}

int module_stop(unsigned int args, void* argp)
{
    // module_stop runs on a thread lv2 spawned for it, which has no TLS (r13 == 0).
    // liblv2's sys_ppu_thread_create() reads the caller's TLS on some paths, so use
    // the raw syscalls here for the same reason module_start does.
    sys_ppu_thread_t stopPpuThreadId;

    sys_ppu_thread_init init;
    init.entry        = (sys_addr_t)(uintptr_t)MainThreadStop;
    init.tls_mem_addr = (sys_addr_t)((uintptr_t)s_stopTls + 0x7030);

    int ret = _sys_ppu_thread_create(&stopPpuThreadId, &init, 0, 0, 2816, 1024,
                                     SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_Loader");

    if (ret == SUCCEEDED)
    {
        sys_ppu_thread_start(stopPpuThreadId);

        uint64_t exitCode;
        sys_ppu_thread_join(stopPpuThreadId, &exitCode);
    }

    libpsutil::sleep(5);

    // unloading prx from memory
    sys_prx_id_t prxId = _sys_prx_get_my_module_id();
    uint64_t meminfo[5]{ 0x28, 2, 0, 0, 0 };
    _sys_prx_stop_module(prxId, 0, meminfo, NULL, 0, NULL);

    // Exit thread using directly the syscall and not the user mode library or else we will crash
    _sys_ppu_thread_exit(0);

    return 0;
}
CDECL_END