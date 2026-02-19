#pragma once
#include <vsh/vshmain.hpp>
#include <vsh/vshcommon.hpp>
#include <vsh/paf.hpp>
#include <vsh/plugins.hpp>
#include <sys/timer.h>
#include <sys/ppu_thread.h>
#include <string>

class FindActiveGame
{
public:
    FindActiveGame();
    ~FindActiveGame() noexcept = default;

    static void GameProcessThread(uint64_t arg);
    void Initialize();
    void Shutdown();
    uint32_t GetRunningGameProcessId();
    void GetGameName(std::string& outTitleId, std::string& outTitleName);

private:
    void SetRunningGameProcessId(uint32_t pid);
    bool IsGameCodMW2(const std::string& titleId);
    void WhileInGame(uint32_t pid, std::string titleId, std::string titleName);

private:
    sys_ppu_thread_t m_GameProcessPpuThreadId = SYS_PPU_THREAD_ID_INVALID;
    bool m_HasGameInitialized{};
    bool m_GameProcessThreadRunning{};
    uint32_t m_CurrentGamePid{};
};

extern FindActiveGame g_FindActiveGame;