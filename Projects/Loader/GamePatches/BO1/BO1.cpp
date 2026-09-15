#include "BO1.hpp"

#include "Utils/Types.hpp"
#include "Utils/LoaderCommon.hpp"

#include <sys/ppu_thread.h>

#include <libpsutil.h>

/* Assembled from Assembly/Cave.s. Plain labels in .text rather than functions, so
 * these addresses are the instructions themselves and not function descriptors.
 * bo1_cave_params is the same address as bo1_cave_start, named separately because
 * the two immediates the cave opens with are what this file fills in. */
extern "C" u8  bo1_cave_start[];
extern "C" u32 bo1_cave_params[];

namespace BO1
{
    namespace
    {
        /* Stock BO1 derives its Demonware identity by Tiger hashing the PSN online
         * id, which for accounts created or renamed after late 2018 is not the id
         * the server has on file, so stats are read and written under a key that
         * does not exist and nothing persists. Cave.s carries the reasoning; this
         * file resolves the addresses it needs and installs it. */

        /* Scan hints. These are the NPEB00756 addresses and are only the centre of
         * the window - nothing is written at them. Two independent deltas were
         * measured between builds, data globals -0x80 and code in this range -0x238,
         * which is why every address below is scanned for rather than offset. */
        const u32 CounterSiteHint = 0x0027E504;   /* inside sub_27E4C8            */
        const u32 XuidSiteHint    = 0x0027E618;   /* inside sub_27E4C8            */
        const u32 HookSiteHint    = 0x00850B48;   /* sub_850A90's epilogue        */

        /* sub_27E4C8 did not move at all between the builds checked, so it needs no
         * room to speak of. The deserializer moved 0x238, hence the wider window. */
        const u32 AccessorWindow = 0x2000;
        const u32 HookWindow     = 0x8000;

        /* Everything resolved below is a data global, and BO1's data starts well
         * above the end of its text at 0x008C8010. A misresolve - the wrong
         * instruction pair, or a read that came back zero - lands outside this. */
        const u32 DataLow  = 0x00900000;
        const u32 DataHigh = 0x04000000;

        /* The accessor keeps one slot per controller. Slot 0 is the signed in PSN
         * account; guests never had a server identity to correct. The flag array
         * sits immediately after the four u64 slots, in every build checked. */
        const u32 FlagArrayOffset = 0x20;

        /* Inside sub_27E4C8. Both change counters, each built from a lis and an
         * addi, with the counters' own addresses in the wildcarded halves:
         *
         *     +0x00  lis   r4, cached@ha
         *     +0x04  lis   r3, live@ha
         *     +0x08  extsw r0, r31
         *     +0x0C  addi  r8, r4, cached@l
         *     +0x10  addi  r9, r3, live@l                                        */
        const i16 SigCounters[] = {
            0x3C, 0x80,   -1,   -1,
            0x3C, 0x60,   -1,   -1,
            0x7F, 0xE0, 0x07, 0xB4,
            0x39, 0x04,   -1,   -1,
            0x39, 0x23,   -1,   -1
        };

        /* Further down sub_27E4C8, the same idea for the cached XUID array:
         *
         *     +0x00  lis   r7, xuid@ha
         *     +0x04  slwi  r10, r30, 3
         *     +0x08  lbzx  r8, r12, r28
         *     +0x0C  cmpwi cr1, r8, 0
         *     +0x10  beq   cr1, ...
         *     +0x14  addi  r28, r7, xuid@l                                       */
        const i16 SigXuid[] = {
            0x3C, 0xE0,   -1,   -1,
            0x57, 0xCA, 0x18, 0x38,
            0x7D, 0x0C, 0xE0, 0xAE,
            0x2C, 0x88, 0x00, 0x00,
            0x41, 0x86,   -1,   -1,
            0x3B, 0x87,   -1,   -1
        };

        /* sub_850A90's whole epilogue, which the hook site is the first word of.
         * Long and entirely literal on purpose: it is the only thing standing
         * between a patch and the middle of some unrelated function, and its first
         * four words are the four instructions Cave.s replays, so a match proves
         * those too. */
        const i16 SigHook[] = {
            0xEB, 0x61, 0x00, 0xC0,   /* ld    r27, 0xC0(r1)    displaced        */
            0xEB, 0x41, 0x00, 0x80,   /* ld    r26, 0x80(r1)    displaced        */
            0x7F, 0x68, 0x03, 0xA6,   /* mtlr  r27              displaced        */
            0xEB, 0x81, 0x00, 0x90,   /* ld    r28, 0x90(r1)    displaced        */
            0xEB, 0x61, 0x00, 0x88,   /* ld    r27, 0x88(r1)    resume point     */
            0xEB, 0xA1, 0x00, 0x98,
            0xEB, 0xC1, 0x00, 0xA0,
            0xEB, 0xE1, 0x00, 0xA8,
            0x38, 0x21, 0x00, 0xB0,
            0x4E, 0x80, 0x00, 0x20,
            0x88, 0x01, 0x00, 0x78,
            0x7F, 0xC7, 0xF3, 0x78,
            0x80, 0xA1, 0x00, 0x70,
            0x7F, 0x83, 0xE3, 0x78,
            0x38, 0x80, 0x00, 0x80,
            0x7F, 0x66, 0xDB, 0x78,
            0x39, 0x00, 0x00, 0x04,
            0x98, 0x1A, 0x00, 0x04,
            0x4B, 0xFC, 0x79, 0x09,
            0x54, 0x67, 0x06, 0x3E
        };

        /* Filled in before the cave can reach it, and read by Cave.s through the
         * pointer this file patches into its first two instructions. Same order and
         * same five words as the PARAM_ offsets at the top of that file. */
        struct CaveParams
        {
            u32 liveCounter;     /* dword_2000998, bumped when the user changes  */
            u32 cachedCounter;   /* dword_B5C160, the accessor's copy of it      */
            u32 xuidSlot;        /* qword_119AAB8, the cached XUID, slot 0       */
            u32 validFlag;       /* byte_119AAD8, its validity flag, slot 0      */
            u32 resumeAddress;   /* the hook site, past the instructions it took */
        };

        CaveParams g_CaveParams;

        /* The entry jump is lis/ori/mtctr/bctr. A relative branch only carries 32MB
         * and the cave lives in this module, which is nowhere near the game's text,
         * so it has to go through CTR. */
        const u32 HookSize = 16;

        u32 Word(u32 address)
        {
            return *(const volatile u32*)address;
        }

        /* Passing the element count by hand is how the first version of this
         * scanner came to read twice as many bytes as the signature had. Letting
         * the type system count them removes that as a possibility. */
        template<u32 N>
        u32 Scan(u32 hint, const i16 (&signature)[N], u32 window)
        {
            return LoaderCommon::FindFunctionAddress(hint, signature, N, window);
        }

        /* PowerPC cannot load a 32 bit address in one instruction, so the accessor
         * builds each of its globals from a lis and an addi. addi sign extends its
         * immediate and the assembler pre-increments the high half to compensate,
         * so putting the two back together is an add with a signed low half, not an
         * or with an unsigned one. */
        u32 Resolve(u32 lisWord, u32 loWord)
        {
            const u32 high = lisWord & 0xFFFF;
            const i16 low  = (i16)(loWord & 0xFFFF);

            return (high << 16) + low;
        }

        bool Plausible(u32 address)
        {
            return address >= DataLow && address < DataHigh;
        }

        /* Cave.s opens with lis r12, 0 / ori r12, r12, 0 and uses r12 as the base
         * for everything it reads. Those two immediates are the only thing in it
         * that cannot be known until the module has been loaded. */
        bool PointCaveAtParams(u32 params)
        {
            /* lis sign extends, so an address with bit 31 set would be reassembled
             * as 0xFFFFFFFFxxxxxxxx and read from nowhere. Everything the PRX
             * loader hands out sits well below that, but a wrong pointer here is a
             * hang rather than a message. */
            if (params >= 0x80000000u)
            {
                printf("[CODPatch Loader] BO1: parameter block at 0x%08X is out of reach\n",
                    params);
                return false;
            }

            u32 immediates[2] = {
                0x3D800000u | (params >> 16),      /* lis r12, params@h      */
                0x618C0000u | (params & 0xFFFF),   /* ori r12, r12, params@l */
            };

            if (!libpsutil::memory::set((u32)bo1_cave_params, immediates, sizeof(immediates)))
            {
                printf("[CODPatch Loader] BO1: could not point the cave at its parameters\n");
                return false;
            }

            /* Read back before anything is able to branch here. A live hook over a
             * cave that still believes its parameters are at address zero is a
             * crash on the first lobby message, not a patch that quietly did
             * nothing. */
            const volatile u32* cave = (const volatile u32*)bo1_cave_params;

            if (cave[0] != immediates[0] || cave[1] != immediates[1])
            {
                printf("[CODPatch Loader] BO1: cave verify failed, reads %08X %08X\n",
                    cave[0], cave[1]);
                return false;
            }

            return true;
        }

        bool InstallHook(u32 address, u32 cave)
        {
            if (cave >= 0x80000000u)
            {
                printf("[CODPatch Loader] BO1: cave at 0x%08X is out of reach\n", cave);
                return false;
            }

            u32 stub[4] = {
                0x3D600000u | (cave >> 16),        /* lis   r11, cave@h      */
                0x616B0000u | (cave & 0xFFFF),     /* ori   r11, r11, cave@l */
                0x7D6903A6u,                       /* mtctr r11              */
                0x4E800420u,                       /* bctr                   */
            };

            if (!libpsutil::memory::set(address, stub, sizeof(stub)))
            {
                printf("[CODPatch Loader] BO1: could not patch 0x%08X\n", address);
                return false;
            }

            /* Read the site back rather than trusting the write. */
            const volatile u32* site = (const volatile u32*)address;

            printf("[CODPatch Loader] BO1: hook 0x%08X -> 0x%08X, reads %08X %08X %08X %08X\n",
                address, cave, site[0], site[1], site[2], site[3]);

            return true;
        }

        /* Read only, and only so that the console says what the game now thinks it
         * is without anyone having to peek at memory from outside. The cave
         * publishes into the accessor's own cache, so these two globals are both
         * the answer and the proof that it fired.
         *
         * Seeing the derived id here first and the authenticated one a moment later
         * is normal: the ticket arrives late in sign in and something asks for the
         * id before it does. Seeing only the derived one means the cave never ran. */
        void IdentityWatch(u64 arg)
        {
            u64 announced = 0;

            for (;;)
            {
                const u64 identity = *(const volatile u64*)g_CaveParams.xuidSlot;
                const u8  valid    = *(const volatile u8*)g_CaveParams.validFlag;

                if (valid != 0 && identity != announced)
                {
                    announced = identity;

                    printf("[CODPatch Loader] BO1: identity is now 0x%08X%08X\n",
                        (u32)(identity >> 32), (u32)identity);
                }

                libpsutil::sleep(1000);
            }
        }
    }

    void Main()
    {
        // Guard against new threads
        static bool patched = false;
        if (patched)
            return;

        patched = true;

        /* Resolve everything first. Nothing is written until all of it holds up - a
         * half applied patch here hangs the auth thread rather than failing.
         *
         * This doubles as the check that the Loader landed in the right binary. BO1
         * ships a separate campaign executable and the Loader is injected into
         * whichever process is running; in the wrong one these addresses hold
         * unrelated code, the scan comes back empty and nothing is written. */
        const u32 counterSite = Scan(CounterSiteHint, SigCounters, AccessorWindow);
        const u32 xuidSite    = Scan(XuidSiteHint,    SigXuid,     AccessorWindow);
        const u32 hookSite    = Scan(HookSiteHint,    SigHook,     HookWindow);

        if (!counterSite || !xuidSite || !hookSite)
        {
            printf("[CODPatch Loader] BO1: signature scan failed, counters=0x%08X xuid=0x%08X hook=0x%08X\n",
                counterSite, xuidSite, hookSite);
            return;
        }

        g_CaveParams.cachedCounter = Resolve(Word(counterSite + 0x00), Word(counterSite + 0x0C));
        g_CaveParams.liveCounter   = Resolve(Word(counterSite + 0x04), Word(counterSite + 0x10));
        g_CaveParams.xuidSlot      = Resolve(Word(xuidSite + 0x00),    Word(xuidSite + 0x14));
        g_CaveParams.validFlag     = g_CaveParams.xuidSlot + FlagArrayOffset;
        g_CaveParams.resumeAddress = hookSite + HookSize;

        printf("[CODPatch Loader] BO1: cached=0x%08X live=0x%08X xuid=0x%08X flag=0x%08X\n",
            g_CaveParams.cachedCounter, g_CaveParams.liveCounter,
            g_CaveParams.xuidSlot, g_CaveParams.validFlag);

        if (!Plausible(g_CaveParams.cachedCounter) ||
            !Plausible(g_CaveParams.liveCounter) ||
            !Plausible(g_CaveParams.xuidSlot))
        {
            printf("[CODPatch Loader] BO1: refusing to patch, those are not data globals\n");
            return;
        }

        if (!PointCaveAtParams((u32)&g_CaveParams))
            return;

        if (!InstallHook(hookSite, (u32)bo1_cave_start))
            return;

        sys_ppu_thread_t watchThreadId;
        sys_ppu_thread_create(&watchThreadId, IdentityWatch, 0, 1059, 8192,
            SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_BO1Identity");
    }
}
