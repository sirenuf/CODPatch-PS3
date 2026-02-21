#include "FindActiveGame.hpp"
#include "CODMW2.hpp"
#include "Utils/SystemCalls.hpp"

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

// this function feels clutterred now because
// I feel like it is detatched from the loop.
// TODO: Come back later
bool FindActiveGame::IsStillActive()
{
    // If PID is not the same, game has exited. 
    return (m_HasGameInitialized && vsh::GetGameProcessId() == GetRunningGameProcessId());
}

u32 FindActiveGame::GetRunningGameProcessId()
{
    return m_CurrentGamePid;
}

void FindActiveGame::SetRunningGameProcessId(u32 pid)
{
    m_CurrentGamePid = pid;
}

void FindActiveGame::GetGameName(std::string& outTitleId, std::string& outTitleName)
{
    paf::View* gamePlugin = paf::View::Find("game_plugin");
    if (!gamePlugin)
        return;

    vsh::GamePluginInterface* gameInterface = gamePlugin->GetInterface<vsh::GamePluginInterface*>(1);
    if (!gameInterface)
        return;

    vsh::GamePluginInterface::gameInfo info;
    gameInterface->GameInfo(info);
    outTitleId = info.titleid;
    outTitleName = info.titlename;
}

void FindActiveGame::GetGameBinaryName(std::string& binaryName)
{
    if (m_CurrentGamePid == 0)
        return;

    char buffer[256]{};
    ps3mapi_get_process_name_by_pid(m_CurrentGamePid, buffer);

    binaryName = buffer;
}

bool FindActiveGame::IsGameCodMW2(const std::string& titleId)
{
    if (titleId == "BLUS30450" || titleId == "BLUS30377" || titleId == "BLUS30337" || titleId == "BLUS30429"
        || titleId == "BLES00683" || titleId == "BLES00691" || titleId == "BLES00690" || titleId == "BLES00686"
        || titleId == "BLES00685" || titleId == "BLES00684" || titleId == "BLES00687" || titleId == "NPEB00731"
        || titleId == "BLJM61006" || titleId == "BLJM60191")
    {
        std::string binaryName;
        GetGameBinaryName(binaryName);

        // Makes sure that multiplayer has been launched.
        if (libpsutil::string::ends_with(binaryName, "default_mp.se"))
            return true;
    }

    return false;
}

void FindActiveGame::WhileInGame(u32 pid, std::string titleId, std::string titleName)
{
    // If game is already initialised return.
    if (m_HasGameInitialized)
        return;

    if (IsGameCodMW2(titleId))
    {
        m_HasGameInitialized = true;
        CODMW2::Initialize();
    }
}

void FindActiveGame::GameProcessThread(u64 arg)
{
    g_FindActiveGame.m_GameProcessThreadRunning = true;
    u32 gameProcessID = 0;
    while (g_FindActiveGame.m_GameProcessThreadRunning)
    {
        gameProcessID = vsh::GetGameProcessId();

        if (gameProcessID != 0)
        {
            libpsutil::sleep(1000);

            g_FindActiveGame.SetRunningGameProcessId(gameProcessID);
            if (g_FindActiveGame.GetRunningGameProcessId())
            {
                std::string gameTitleId;
                std::string gameTitleName;
                g_FindActiveGame.GetGameName(gameTitleId, gameTitleName);
                g_FindActiveGame.WhileInGame(g_FindActiveGame.GetRunningGameProcessId(), gameTitleId, gameTitleName);
            }
        }
        else
        {
            // disconnect from game process
            if (g_FindActiveGame.GetRunningGameProcessId())
            {
                g_FindActiveGame.SetRunningGameProcessId(0);
                g_FindActiveGame.m_HasGameInitialized = false;
            }
        }

        // Not sure if I still need this. Plugin is not meant to be unloaded in actuality though.
        // Good Bye CPU! Smallest sleep because we need to execute the patches as fast as possible 
        sys_timer_usleep(1668);
        sys_ppu_thread_yield();
    }

    sys_ppu_thread_exit(0);
}