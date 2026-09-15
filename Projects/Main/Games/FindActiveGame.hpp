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
    /* No constructor on purpose. Static initialisers are not run for this plugin,
     * so anything a constructor did would silently never happen. Initialize() is
     * the setup step and must be called explicitly. */
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
    
private:
    void FindOutWhatCod(std::string titleId);
    void SetCurrentGamePid(u32 pid);
    u32 GetCurrentGamePid();

private:
    /* Deliberately no initialisers here. They would only run from a constructor,
     * and constructors of globals are never called in this plugin, so they would
     * read as working code that silently does nothing. Initialize() sets these. */
    sys_ppu_thread_t m_GameProcessPpuThreadId;
    bool m_GameProcessThreadRunning;
    u32 m_CurrentGamePid;
};

extern FindActiveGame g_FindActiveGame;