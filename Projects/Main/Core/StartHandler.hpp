#pragma once

#include <sys/ppu_thread.h>

#include <vsh/paf.hpp>
#include <vsh/vshtask.hpp>

#include <libpsutil.h>

// Used to create and stop MainThread
extern sys_ppu_thread_t gVshMenuPpuThreadId;

void MainThread(uint64_t arg);
void MainThreadStop(uint64_t arg);