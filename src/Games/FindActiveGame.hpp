#pragma once
#include <vsh/vshmain.hpp>
#include <vsh/vshcommon.hpp>
#include <vsh/paf.hpp>
#include <vsh/plugins.hpp>
#include <sys/timer.h>
#include <sys/ppu_thread.h>
#include <string>

#include "Utils/Types.hpp"
#include "Games/CODCommon.hpp"

class FindActiveGame
{
public:
    FindActiveGame();
    ~FindActiveGame() noexcept = default;

    static void GameProcessThread(u64 arg);
    void Initialize();
    void Shutdown();
    uint32_t GetRunningGameProcessId();

    /**
     * @return The official game name of the currently running game. E.g. Call of Duty: Modern Warfare 2
     */
    std::string GetGameName();
    
    /**
     * @return The official PS3 "GameID" of the currently running game. E.g. BLES123456 
     */
    std::string GetGameID();

    /**
     * @return
     * Gets the name of the currently running SELF binary associated with the game
     * 
     * NOTE: file name may be truncated. e.g. <pid>_default_mp.se
     * 
     * Example output: <pid>_default_mp.se
     */
    std::string GetGameBinaryName();
    
    /**
     * @return
     * Boolean that determines if plugin has been unloaded or game has been exited.
     * Useful in while loops to make sure that it stops executing after game is closed or plugin is unloaded. 
     */
    bool IsGameRunning(CODCommon::CODType gameType);

private:
    bool IsGameCodMW2(const std::string& GameID);
    void WhileInGame(std::string titleId);

private:
    sys_ppu_thread_t m_GameProcessPpuThreadId = SYS_PPU_THREAD_ID_INVALID;
    bool m_GameProcessThreadRunning{};
    uint32_t m_CurrentGamePid{};
};

extern FindActiveGame g_FindActiveGame;