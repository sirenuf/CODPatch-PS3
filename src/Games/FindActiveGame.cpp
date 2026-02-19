#include "FindActiveGame.hpp"
#include "CODMW2.hpp"
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

        uint64_t exitCode;
        sys_ppu_thread_join(m_GameProcessPpuThreadId, &exitCode);
    }
}

uint32_t FindActiveGame::GetRunningGameProcessId()
{
    return m_CurrentGamePid;
}

void FindActiveGame::SetRunningGameProcessId(uint32_t pid)
{
    m_CurrentGamePid = pid;
}

void FindActiveGame::GetGameName(std::string& outTitleId, std::string& outTitleName)//char outTitleId[16], char outTitleName[64])
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
    
    //vsh::snprintf(outTitleId, 10, "%s", _gameInfo + 0x04);
    //vsh::snprintf(outTitleName, 63, "%s", _gameInfo + 0x14);
}

bool FindActiveGame::IsGameCodMW2(const std::string& titleId)
{
    if (titleId == "BLUS30450" || titleId == "BLUS30377" || titleId == "BLUS30337" || titleId == "BLUS30429"
        || titleId == "BLES00683" || titleId == "BLES00691" || titleId == "BLES00690" || titleId == "BLES00686"
        || titleId == "BLES00685" || titleId == "BLES00684" || titleId == "BLES00687" || titleId == "NPEB00731"
        || titleId == "BLJM61006" || titleId == "BLJM60191")
        return true;

    return false;
}

void FindActiveGame::WhileInGame(uint32_t pid, std::string titleId, std::string titleName)
{
    if (IsGameCodMW2(titleId))
    {
        if (!m_HasGameInitialized)
        {
            CODMW2::Initialize();
            m_HasGameInitialized = true;
        }
    }

}

void FindActiveGame::GameProcessThread(uint64_t arg)
{
    g_FindActiveGame.m_GameProcessThreadRunning = true;
    uint32_t gameProcessID = 0;
    while (g_FindActiveGame.m_GameProcessThreadRunning)
    {
        gameProcessID = vsh::GetGameProcessId();

        if (gameProcessID != 0)
        {
            for (int x = 0; x < (10 * 100); x++) // 10 second delay
            {
                sys_timer_usleep(10000);
                sys_ppu_thread_yield();
            }

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

        // Good Bye CPU! Smallest sleep because we need to execute the patches as fast as possible 
        sys_timer_usleep(1668);
        sys_ppu_thread_yield();
    }

    sys_ppu_thread_exit(0);
}