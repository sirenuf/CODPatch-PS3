#include "FindActiveGame.hpp"

#include "Games/Common.hpp"
#include "Games/COD/MW2/MW2.hpp"
#include "Games/COD/MW3/MW3.hpp"
#include "Games/COD/BO1/BO1.hpp"
#include "Games/COD/BO2/BO2.hpp"

#include "Utils/SystemCalls.hpp"

#include <libpsutil.h>

FindActiveGame g_FindActiveGame;

void FindActiveGame::Initialize()
{
    /* Static initialisers are not run for this plugin, so the member initialisers
     * in the class definition never fire and this object arrives as plain zeroed
     * memory. Everything it needs has to be set by hand, here. */
    m_GameProcessPpuThreadId = SYS_PPU_THREAD_ID_INVALID;
    m_CurrentGamePid = 0;

    // Cache GameIDs array because of non-standard SDK behaviour.
    MW2::GetGameIDs();
    MW3::GetGameIDs();
    BO1::GetGameIDs();
    BO2::GetGameIDs();

    // Set before the thread exists so it can never read this as false and exit early.
    m_GameProcessThreadRunning = true;

    sys_ppu_thread_create(&m_GameProcessPpuThreadId, GameProcessThread, 0, 1109, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatchGameProcess");
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

        m_GameProcessPpuThreadId = SYS_PPU_THREAD_ID_INVALID;
    }
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

// unused
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

// unused
std::string FindActiveGame::GetGameBinaryName()
{
    char buffer[256]{};
    ps3mapi_get_process_name_by_pid(vsh::GetGameProcessId(), buffer);

    return std::string(buffer);
}

void FindActiveGame::FindOutWhatCod(std::string GameID)
{
    using Globals::Game;

    auto& mw2gid = MW2::GetGameIDs();
    auto& mw3gid = MW3::GetGameIDs();
    auto& bo1gid = BO1::GetGameIDs();
    auto& bo2gid = BO2::GetGameIDs();
    bool MW2Running = mw2gid.find(GameID) != mw2gid.end();
    bool MW3Running = mw3gid.find(GameID) != mw3gid.end();
    bool BO1Running = bo1gid.find(GameID) != bo1gid.end();
    bool BO2Running = bo2gid.find(GameID) != bo2gid.end();

    Game game = Game::None;

    if (MW2Running)
        game = Game::MW2;
    else if (MW3Running)
        game = Game::MW3;
    else if (BO1Running)
        game = Game::BO1;
    else if (BO2Running)
        game = Game::BO2;

    // Game isn't COD.
    if (game == Game::None)
        return;

    Common::LoadSprx(game);
}

void FindActiveGame::SetCurrentGamePid(u32 pid)
{
    m_CurrentGamePid = pid;
}

u32 FindActiveGame::GetCurrentGamePid()
{
    return m_CurrentGamePid;
}

void FindActiveGame::GameProcessThread(u64 arg)
{
    // Initialize() owns this flag and starts this thread; Shutdown() clears it.
    while (g_FindActiveGame.m_GameProcessThreadRunning)
    {
        u32 gameProcessID = vsh::GetGameProcessId();

        if (gameProcessID != 0 && g_FindActiveGame.GetCurrentGamePid() != gameProcessID)
        {
            g_FindActiveGame.SetCurrentGamePid(gameProcessID);

            g_FindActiveGame.FindOutWhatCod(g_FindActiveGame.GetGameID());
        }
        else if (gameProcessID == 0)
        {
            g_FindActiveGame.SetCurrentGamePid(0);
        }

        libpsutil::sleep(100);
    }

    sys_ppu_thread_exit(0);
}