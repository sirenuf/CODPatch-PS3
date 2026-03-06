#include "FindActiveGame.hpp"

#include "Utils/SystemCalls.hpp"
#include "Utils/StringFunctions.hpp"

#include "Games/COD/MW2.hpp"
#include "Games/COD/MW3.hpp"

#include <libpsutil.h>

using CODCommon::CODType;

FindActiveGame g_FindActiveGame;

FindActiveGame::FindActiveGame()
{
    sys_ppu_thread_create(&m_GameProcessPpuThreadId, GameProcessThread, 0, 1109, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatchGameProcess");
}

void FindActiveGame::Initialize()
{

}

void FindActiveGame::Shutdown()
{
    if (m_GameProcessPpuThreadId != SYS_PPU_THREAD_ID_INVALID)
    {
        m_GameProcessThreadRunning = false;

        // Prevent unload too fast (give time to other threads to finish)
        sys_ppu_thread_yield();
        sys_timer_sleep(1);
        u64 exitCode;
        sys_ppu_thread_join(m_GameProcessPpuThreadId, &exitCode);
    }
}

bool FindActiveGame::IsGameRunning(CODType gameType)
{
    if (!m_GameProcessThreadRunning)
        return false;

    std::string currentGameID = GetGameID();
    bool gameRunning = false;

    switch (gameType)
    {
    case CODCommon::MW2:
            gameRunning = IsGameCodMW2(currentGameID);
            break;

    case CODCommon::MW3:
            gameRunning = IsGameCodMW3(currentGameID);
            break;
    }

    return gameRunning;
}

u32 FindActiveGame::GetRunningGameProcessId()
{
    return vsh::GetGameProcessId();
}

std::string FindActiveGame::GetGameID()
{
    paf::View* gamePlugin = paf::View::Find("game_plugin");
    if (!gamePlugin)
        return "";

    vsh::GamePluginInterface* gameInterface = gamePlugin->GetInterface<vsh::GamePluginInterface*>(1);
    if (!gameInterface)
        return "";

    vsh::GamePluginInterface::gameInfo info;
    gameInterface->GameInfo(info);
    return info.titleid;
}

std::string FindActiveGame::GetGameName()
{
    paf::View* gamePlugin = paf::View::Find("game_plugin");
    if (!gamePlugin)
        return "";

    vsh::GamePluginInterface* gameInterface = gamePlugin->GetInterface<vsh::GamePluginInterface*>(1);
    if (!gameInterface)
        return "";

    vsh::GamePluginInterface::gameInfo info;
    gameInterface->GameInfo(info);
    return info.titlename;
}

std::string FindActiveGame::GetGameBinaryName()
{
    char buffer[256]{};
    ps3mapi_get_process_name_by_pid(vsh::GetGameProcessId(), buffer);

    return std::string(buffer);
}

// TODO: Rewrite this to a unified function
bool FindActiveGame::IsGameCodMW2(const std::string& GameID)
{
    auto& gid = MW2::GetGameIDs();
    bool gameIdFound = gid.find(GameID) != gid.end();

    if (!gameIdFound)
        return false;

    bool isMultiplayer = StringFunctions::IsInString(GetGameBinaryName(), "default_mp");
    return isMultiplayer; // Multiplayer must be launched.
}

bool FindActiveGame::IsGameCodMW3(const std::string& GameID)
{
    auto& gid = MW3::GetGameIDs();
    bool gameIdFound = gid.find(GameID) != gid.end();

    if (!gameIdFound)
        return false;

    bool isMultiplayer = StringFunctions::IsInString(GetGameBinaryName(), "default_mp");
    return isMultiplayer; // Multiplayer must be launched.
}

void FindActiveGame::WhileInGame(std::string GameID)
{
    if (IsGameCodMW2(GameID))
        MW2::Run();
    else if (IsGameCodMW3(GameID))
        MW3::Run();
}

void FindActiveGame::GameProcessThread(u64 arg)
{
    g_FindActiveGame.m_GameProcessThreadRunning = true;
    while (g_FindActiveGame.m_GameProcessThreadRunning)
    {
        u32 gameProcessID = vsh::GetGameProcessId();

        if (gameProcessID != 0)
            g_FindActiveGame.WhileInGame(g_FindActiveGame.GetGameID());

        libpsutil::sleep(2000);
    }

    sys_ppu_thread_exit(0);
}