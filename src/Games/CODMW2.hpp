#pragma once 
#include <vsh/vshtask.hpp>

/* TODO: Make sure it only runs when the _mp.bin binary is loaded. */
namespace CODMW2
{
	static void Initialize()
	{
		vshtask::Notify("MW2 loaded.");
	}
}