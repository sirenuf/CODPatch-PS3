#pragma once
#include <stdint.h>
#include <stdio.h>
#include <ppu_asm_intrinsics.h> // __ALWAYS_INLINE
#include <sys/process.h>
#include "Utils/SystemCalls.hpp"
#include "Utils/Types.hpp"

struct opd_s
{
	u32 func;
	u32 toc;
};

struct importStub_s
{
	i16 ssize;
	i16 header1;
	i16 header2;
	i16 imports;
	i32 zero1;
	i32 zero2;
	const char* name;
	u32* fnid;
	opd_s** stub;
	i32 zero3;
	i32 zero4;
	i32 zero5;
	i32 zero6;
};

struct exportStub_s
{
	i16 ssize;
	i16 header1;
	i16 header2;
	i16 exports; // number of exports
	i32 zero1;
	i32 zero2;
	const char* name;
	u32* fnid;
	opd_s** stub;
};

u32 GetCurrentToc();

void PatchInJump(int Address, int Destination);
void HookFunctionStart(int Address, int saveStub, int Destination);

/** Writes memory to a process.
 * @param pid Process ID of the process you want to poke in.
 * @param address The memory offset.
 * @param data The data that you want to write. May be a sequence of bytes in an array.
 * @param size How many bytes you want to write, ideally should be sizeof(data).
 * @return Status code.
 */
int WriteProcessMemory(u32 pid, void* address, const void* data, size_t size);

/** Reads a process' memory.
 * @param pid Process ID of the process you want to peek in.
 * @param address The memory offset.
 * @param data The object which you want the data that gets read to be stored in. E.g. uint8_t* data
 * @param size How many bytes you want to read. E.g. sizeof(data)
 * @return Status code.
 */
int ReadProcessMemory(u32 pid, void* address, void* data, size_t size);

template<typename T>
inline T VshGetMem(u32 address)
{
	T data;
	ReadProcessMemory(sys_process_getpid(), (void*)address, &data, sizeof(T));
	return data;
}

template<typename T>
inline void VshSetMem(u32 address, T data)
{
	WriteProcessMemory(sys_process_getpid(), (void*)address, &data, sizeof(T));
}

template <typename R, typename... Args>
__ALWAYS_INLINE R CallByAddr(u32 addr, Args... args)
{
	volatile opd_s opd = { addr, GetCurrentToc() };
	R(*fn)(Args...) = (R(*)(Args...)) & opd;
	return fn(args...);
}

template <typename R, typename... Args>
__ALWAYS_INLINE R CallByOpd(opd_s opd, Args... args)
{
	if (opd.sub && opd.toc)
	{
		R(*fn)(Args...) = (R(*)(Args...)) & opd;
		return fn(args...);
	}
	return (R)0;
}

template<typename R, typename... Args>
__ALWAYS_INLINE R CallVmtMethodByClassAddr(u32 addr, int index, Args... args)
{
	opd_s** opd = reinterpret_cast<opd_s**>(addr);
	R(*fn)(Args...) = (R(*)(Args...))opd[index];
	return fn(args...);
}