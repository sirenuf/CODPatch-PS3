#include "FindActiveGame.hpp"

#include "Utils/SystemCalls.hpp"
#include "Games/COD/MW2.hpp"

#include <libpsutil.h>

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

bool FindActiveGame::IsGameRunning(CODCommon::CODType gameType)
{
    std::string currentGameID = GetGameID();
    bool gameRunning = false;

    switch (gameType) {
        case CODCommon::MW2:
            gameRunning = IsGameCodMW2(currentGameID);
            break;
    }

    return (m_GameProcessThreadRunning && gameRunning);
}

u32 FindActiveGame::GetRunningGameProcessId()
{
    return vsh::GetGameProcessId();
}

std::string FindActiveGame::GetGameID()
{
    paf::View* gamePlugin = paf::View::Find("game_plugin");
    if (!gamePlugin)
        return;

    vsh::GamePluginInterface* gameInterface = gamePlugin->GetInterface<vsh::GamePluginInterface*>(1);
    if (!gameInterface)
        return;

    vsh::GamePluginInterface::gameInfo info;
    gameInterface->GameInfo(info);
    return info.titleid;
}

std::string FindActiveGame::GetGameName()
{
    paf::View* gamePlugin = paf::View::Find("game_plugin");
    if (!gamePlugin)
        return;

    vsh::GamePluginInterface* gameInterface = gamePlugin->GetInterface<vsh::GamePluginInterface*>(1);
    if (!gameInterface)
        return;

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

bool FindActiveGame::IsGameCodMW2(const std::string& GameID)
{
    auto MW2GameIDs = MW2::GetGameIDs();
    auto it = MW2::GetGameIDs().find(GameID);
    if (it == MW2GameIDs.end())
        return false;

    bool isMultiplayer = libpsutil::string::ends_with(GetGameBinaryName(), "default_mp.se");
    return isMultiplayer; // Multiplayer must be launched.
}

void FindActiveGame::WhileInGame(std::string titleId)
{
    if (IsGameCodMW2(titleId))
        MW2::Run();
}

void FindActiveGame::GameProcessThread(u64 arg)
{
    g_FindActiveGame.m_GameProcessThreadRunning = true;
    while (g_FindActiveGame.m_GameProcessThreadRunning)
    {
        u32 gameProcessID = vsh::GetGameProcessId();

        if (gameProcessID != 0)
        {
            std::string gameId = g_FindActiveGame.GetGameID();
            g_FindActiveGame.WhileInGame(gameId);
        }

        // Not sure if I still need this.
        // Good Bye CPU! Smallest sleep because we need to execute the patches as fast as possible 
        sys_timer_usleep(1668);
        sys_ppu_thread_yield();
    }

    sys_ppu_thread_exit(0);
}