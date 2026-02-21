#pragma once 
#include <vsh/vshtask.hpp>
#include <vsh/stdc.hpp>
#include <libpsutil.h>

#include "Utils/Std.hpp"
#include "Memory/Memory.hpp"
#include "Games/CODCommon.hpp"

namespace CODMW2
{
	using CODCommon::MW2;

	static void Initialize()
	{
		vshtask::Notify("In multiplayer.");

		while (!CODCommon::IsGameReady(MW2))
		{
			// Check if this works by going in n out of game later
			if (!g_FindActiveGame.IsStillActive())
				return;
			libpsutil::sleep(200);
		}

		bool dataSaveFileExists = CODCommon::VerifyFilesystem(MW2);
		if (dataSaveFileExists)
			CODCommon::LoadSavedStats(MW2);

		while (g_FindActiveGame.IsStillActive())
		{
			//SaveCurrentStats();
			vshtask::Notify("hai");
			libpsutil::sleep(8000);
		}
		vshtask::Notify("Exiting this.");

		//ReadCurrentStats();
	}
}