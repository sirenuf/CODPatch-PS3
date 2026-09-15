#pragma once
#include <sys/ppu_thread.h>
#include <sys/process.h>

#include "Utils/SystemCalls.hpp"
#include "Utils/Types.hpp"

#include <libpsutil.h>

extern sys_ppu_thread_t mainThreadId;

void MainThread(u64 arg);
void MainThreadStop(u64 arg);