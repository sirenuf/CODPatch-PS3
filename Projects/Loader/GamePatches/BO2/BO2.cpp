#include "BO2.hpp"

#include "Utils/LoaderCommon.hpp"
#include "Utils/Types.hpp"
#include "Memory/Memory.hpp"

#include <libpsutil.h> 

namespace BO2
{
	void PerformNOP()
	{
		u8 functionOverwrite[] = { 0x60, 0x00, 0x00, 0x00 };
		WriteProcessMemory(sys_process_getpid(), (void*)0x4A80F0, functionOverwrite, sizeof(functionOverwrite));
		WriteProcessMemory(sys_process_getpid(), (void*)0x50B414, functionOverwrite, sizeof(functionOverwrite));
	}

	void Main()
	{
		printf("[CODPatch Loader] overwriting with NOP to memory addresses.\n");

		// No signature check because this needs to be done everywhere.
		PerformNOP();
	}
}