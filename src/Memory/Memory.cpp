#include "Memory.hpp"

#include <string.h>

u32 GetCurrentToc()
{
    u32* entry_point = *reinterpret_cast<uint32_t**>(0x1001C); // ElfHeader->e_entry 
    return entry_point[1];
}

void PatchInJump(int Address, int Destination) {
    int FuncBytes[4];

    Destination = *(int*)Destination;

    FuncBytes[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF);
    if (Destination & 0x8000)
        FuncBytes[0] += 1;
    FuncBytes[1] = 0x396B0000 + (Destination & 0xFFFF);
    FuncBytes[2] = 0x7D6903A6;
    FuncBytes[3] = 0x4E800420;

    memcpy((void*)Address, FuncBytes, 4 * 4);
}

void HookFunctionStart(int Address, int saveStub, int Destination) {
    saveStub = *(int*)saveStub;

    int BranchtoAddress = Address + (4 * 4);
    int StubData[8];

    StubData[0] = 0x3D600000 + ((BranchtoAddress >> 16) & 0xFFFF);
    if (BranchtoAddress & 0x8000)
        StubData[0] += 1;
    StubData[1] = 0x396B0000 + (BranchtoAddress & 0xFFFF);
    StubData[2] = 0x7D6903A6;

    memcpy(&StubData[3], (void*)Address, 4 * 4);
    StubData[7] = 0x4E800420;
    
    memcpy((void*)saveStub, StubData, 8 * 4);
    PatchInJump(Address, Destination);
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