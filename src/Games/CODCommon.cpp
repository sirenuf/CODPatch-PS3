#include "CODCommon.hpp"

#include <vsh/vshtask.hpp>
#include <vsh/stdc.hpp>
#include <libpsutil.h>

#include "Memory/Memory.hpp"
#include "Games/FindActiveGame.hpp"

namespace CODCommon
{
	extern const char* CODTypeS[] = { "mw2" };

	/**
	 * Verifies that the right folders and files exist.
	 * @return
	 * Boolean determines if file exists or not.
	 */
	bool VerifyFilesystem(CODType gameType)
	{
		libpsutil::filesystem::create_directory("/dev_hdd0/tmp/CODPatch");
		libpsutil::filesystem::create_directory(std::string("/dev_hdd0/tmp/CODPatch/") + CODTypeS[gameType]);

		bool storageFileExists;

		storageFileExists = libpsutil::filesystem::file_exists(std::string("/dev_hdd0/tmp/CODPatch/") + CODTypeS[gameType] + std::string("/data"));
		if (!storageFileExists) {
			libpsutil::filesystem::write_file(std::string("/dev_hdd0/tmp/CODPatch/") + CODTypeS[gameType] + std::string("/data"), "");
		}

		return storageFileExists;
	}


	/**
	 * Loads saved stats on filesystem.
	 * Intended to be ran on startup only.
	 * @return
	 */
	void LoadSavedStats(CODType gameType)
	{
		// where the fuck is this overload: https://github.com/skiff/libpsutil/blob/master/libpsutil/system/filesystem.cpp#L28
		// Fixed: https://github.com/skiff/libpsutil/pull/7
		std::string input = libpsutil::filesystem::read_file(std::string("/dev_hdd0/tmp/CODPatch/") + CODTypeS[gameType]  + std::string("/data"));

		const char* pos = input.c_str();

		// This is really autistic
		// the stdc++ lib is barely functional.
		// Do 1. and 2. until it reaches EOF.

		while (*pos != '\0')
		{
			// 1. Store the 1st line (address) as a size_t
			// pos is pointing to the beginning of the first line rn.
			// strchr with \n finds the first occurrence of \n infront of the cursor (pos)
			const char* lineEnd = vsh::strchr(pos, '\n');
			// Note this isn't supposed to happen as it means that
			// the amount of lines isn't a product of two
			if (lineEnd == nullptr) break;

			char line1[32]{};
			vsh::strncpy(line1, pos, lineEnd - pos);
			pos = lineEnd + 1; // Switches to next line.

			int base = (line1[0] == '0' && line1[1] == 'x') ? 16 : 10;
			size_t address = (size_t)vsh::strtoul(line1, nullptr, base);

			// 2. Load 2nd line (data) to a vector with bytes (uint8_t)
			const char* bytesStart = pos;
			lineEnd = vsh::strchr(pos, '\n');

			if (lineEnd == nullptr)
				lineEnd = pos + vsh::strlen(pos); // use end of string instead of breaking
			pos = lineEnd + 1;

			// Parse directly from the content string without copying
			std::vector<uint8_t> bytes;
			const char* p = bytesStart;

			while (p < lineEnd)
			{
				// Skip spaces
				while (p < lineEnd && *p == ' ')
					p++;

				if (p < lineEnd)
				{
					bytes.push_back((uint8_t)vsh::atoi(p));
					// Skip digits
					while (p < lineEnd && *p != ' ')
						p++;
				}
			}

			vshtask::Notify("Writing to memory");
			WriteProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)address, &bytes[0], bytes.size());
		}
	}

	bool IsGameReady(CODType gameType)
	{
		char buffer[8]{};
		if (gameType == MW2)													 // This is the address for the NAT type.
			ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)0x1C103AF, buffer, sizeof(buffer));

		// Need to convert to std::string
		std::string status = buffer;		

		// When "Your NAT Type" in the menu has a value
		// Then the game has connected.
		return 
			libpsutil::string::begins_with(status, "Open") ||
			libpsutil::string::begins_with(status, "Moderate") ||
			libpsutil::string::begins_with(status, "Strict");
	}

};