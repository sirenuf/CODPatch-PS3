#include "Memory.hpp"

u32 GetCurrentToc()
{
    u32* entry_point = *reinterpret_cast<uint32_t**>(0x1001C); // ElfHeader->e_entry 
    return entry_point[1];
}

int WriteProcessMemory(u32 pid, void* address, const void* data, size_t size)
{
    // We don't need this, at least not now
    /*
    static bool useHenSyscalls = false;

    if (!useHenSyscalls)
    {
        int canWrite = sys_dbg_write_process_memory(pid, address, data, size);
        if (canWrite == SUCCEEDED)
        {
            return canWrite;
        }
    }

    useHenSyscalls = true;
    */
    return PS3MAPISetMemory(pid, address, data, size);
}

int ReadProcessMemory(u32 pid, void* address, void* data, size_t size)
{
    // We don't need this, at least not now
    /*
    static bool useHenSyscalls = false;

    if (!useHenSyscalls)
    {
        int canRead = sys_dbg_read_process_memory(pid, address, data, size);
        if (canRead == SUCCEEDED)
        {
            return canRead;
        }
    }

    useHenSyscalls = true;
    */
    return PS3MAPIGetMemory(pid, address, data, size);
}