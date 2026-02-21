#pragma once
#include <vsh/vshmain.hpp>
#include <vsh/vshcommon.hpp>
#include <vsh/paf.hpp>
#include <vsh/plugins.hpp>
#include <sys/timer.h>
#include <sys/ppu_thread.h>
#include <string>

#include "Utils/Types.hpp"

class FindActiveGame
{
public:
    FindActiveGame();
    ~FindActiveGame() noexcept = default;

    static void GameProcessThread(u64 arg);
    void Initialize();
    void Shutdown();
    uint32_t GetRunningGameProcessId();

    /* Gets the PS3 GameID and official game name */
    void GetGameName(std::string& outTitleId, std::string& outTitleName);
    
    /** Gets the name of the currently running SELF binary associated with the game
     * 
     * NOTE: file extension is truncated to .se instead of .self 
     * 
     * Example output: <pid>_default_mp.se
     */
    void GetGameBinaryName(std::string& binaryName);
    
    /**
     * @return
     * Boolean that determines if plugin has been unloaded or game has been exited.
     * Useful in while loops to make sure that it stops executing after game is closed or plugin is unloaded. 
     */
    bool IsStillActive();

private:
    void SetRunningGameProcessId(u32 pid);
    bool IsGameCodMW2(const std::string& titleId);
    void WhileInGame(u32 pid, std::string titleId, std::string titleName);

private:
    sys_ppu_thread_t m_GameProcessPpuThreadId = SYS_PPU_THREAD_ID_INVALID;
    bool m_HasGameInitialized{};
    bool m_GameProcessThreadRunning{};
    uint32_t m_CurrentGamePid{};
};

extern FindActiveGame g_FindActiveGame;