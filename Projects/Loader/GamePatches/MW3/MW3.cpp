#include "MW3.hpp"

#include "Utils/LoaderCommon.hpp"
#include "Utils/Types.hpp"

#include <sys/ppu_thread.h>

#include <libpsutil.h>

/* Assembled from Assembly/Cave.s and Assembly/FrameCave.s. Plain labels in .text rather than
 * functions, so these addresses are the instructions themselves and not function
 * descriptors. */
extern "C" u8 mw3_cave_start[];
extern "C" u8 mw3_frame_cave_start[];

namespace MW3
{
    namespace
    {
        /* Stock MW3 identifies itself with Tiger(PSN online id), which is what
         * Demonware issued while online ids were permanent. Accounts created or
         * renamed after Sony added name changes in late 2018 are issued an id derived
         * from the account id instead, so client and server disagree about who you
         * are and every profile lookup naming you comes back BD_INVALID_USER_ID -
         * which kicks you out of a lobby and crashes the stock host you were joining.
         *
         * sub_2F16F0 writes the derived id into local client record 0 exactly once,
         * while the connection is coming up and before Demonware has answered, and
         * nothing ever revisits it. Two caves fix that between them:
         *
         *   Cave.s         makes dwGetOnlineUserID answer from the auth ticket
         *   FrameCave.s    corrects the record and its copies, all in one frame
         *
         * Both files carry the reasoning for their own contents. This one installs
         * them and watches for copies they do not know about. */

        /* dwGetOnlineUserID(int controllerIndex, u64* out). All 43 callers read the
         * id through here. */
        const u32 DwGetOnlineUserIDAddress = 0x00344EB4;

        /* The per frame system pump. The correction has to happen at its entry, so
         * that nothing further down it ever sees a half changed identity. */
        const u32 FrameAddress = 0x002F2BC4;

        /* Profile lookup completion. Treats any non zero task error as fatal and
         * calls Com_Error at ERR_DROP. Your own account no longer needs this, but it
         * stops somebody else's broken account taking your lobby down when they join
         * it - worth turning on for anything you host. */
        const u32 DwFetchPerformanceValuesCompleteAddress = 0x00340C8C;

        /* Static signature is all that is needed */
        const i16 dwFetchPerformanceCompleteSignature[] = {
            0xF8, 0x21, 0xFF, 0x11, 0x7C, 0x08, 0x02, 0xA6, 0xF8, 0x01, 0x01, 0x00, 0xFB, 0x61, 0x00, 0xC8,
            0x2C, 0x03, 0x00, 0x00, 0xFB, 0xC1, 0x00, 0xE0, 0xFB, 0xA1, 0x00, 0xD8, 0x3B, 0x60, 0x00, 0x02,
            0x60, 0x7D, 0x00, 0x00, 0xFB, 0xE1, 0x00, 0xE8, 0x60, 0x9E, 0x00, 0x00, 0xFB, 0x81, 0x00, 0xD0,
            0xFB, 0x41, 0x00, 0xC0, 0xFB, 0x21, 0x00, 0xB8, 0xFB, 0x01, 0x00, 0xB0, 0x60, 0xBF, 0x00, 0x00
        };
        const u32 dwFetchPerformanceCompleteSignatureLength = 64; // 0x40 bytes

        const bool EnableProfileLookupDetour = false;

        /* bdAuthTicket::m_userID for local client 0, zero until the auth response
         * lands, and local client record 0 + 0x28, which the frame cave corrects from
         * it. Between them they say whether the correction has happened yet. */
        const u32 AuthTicketUserId = 0x01C488E8;
        const u32 LocalClientUserId = 0x01BBBC50;

        /* The two writable data segments taken as one range - the few bytes between
         * them are mapped too. Contiguous, and it holds no code, so a sweep cannot
         * land on an instruction. */
        const u32 DataStart = 0x00727000;
        const u32 DataEnd = 0x0229D018;

        /* Empty passes before the sweep gives up for the session. */
        const u32 SweepQuietPasses = 3;

        void ToHex(u64 value, char* out)
        {
            static const char digits[] = "0123456789abcdef";

            for (int i = 15; i >= 0; i--)
            {
                out[i] = digits[value & 0xF];
                value >>= 4;
            }
        }

        /* Replaces every copy of the derived id with the authenticated one, in both
         * the forms the game keeps: the raw 64 bit value, and the 16 character
         * lowercase hex string. Stepping 4 bytes at a time is enough - every copy
         * ever seen is 8 byte aligned - and it avoids unaligned 64 bit loads. */
        u32 SweepDerivedId(u64 derived, u64 authenticated)
        {
            char derivedHex[16];
            char authenticatedHex[16];

            ToHex(derived, derivedHex);
            ToHex(authenticated, authenticatedHex);

            const u32 derivedHi = (u32)(derived >> 32);
            const u32 derivedLo = (u32)derived;
            const u32 authenticatedHi = (u32)(authenticated >> 32);
            const u32 authenticatedLo = (u32)authenticated;

            u32 replaced = 0;

            for (u32 address = DataStart; address <= DataEnd - 16; address += 4)
            {
                u32* word = (u32*)address;

                if (word[0] == derivedHi && word[1] == derivedLo)
                {
                    word[0] = authenticatedHi;
                    word[1] = authenticatedLo;
                    replaced++;

                    printf("[CODPatch Loader] MW3:   0x%08X value\n", address);
                    continue;
                }

                char* text = (char*)address;

                if (text[0] != derivedHex[0])
                    continue;

                u32 i = 1;
                while (i < 16 && text[i] == derivedHex[i])
                    i++;

                if (i < 16)
                    continue;

                for (i = 0; i < 16; i++)
                    text[i] = authenticatedHex[i];

                replaced++;

                printf("[CODPatch Loader] MW3:   0x%08X text\n", address);
            }

            return replaced;
        }

        /* The frame cave handles everything that has to change together. This is the
         * backstop: MW3 keeps five roster tables of 18 entries each, and a flow we
         * have not exercised could leave the derived id somewhere the cave does not
         * know about. Finding nothing is the outcome we want from it. */
        void UserIdWatch(u64 arg)
        {
            u64 derived = 0;
            u32 quiet = 0;
            bool announced = false;

            for (;;)
            {
                const u64 authenticated = *(volatile u64*)AuthTicketUserId;
                const u64 current = *(volatile u64*)LocalClientUserId;

                /* What sub_2F16F0 derived, taken while the ticket is still missing.
                 * Once it lands the frame cave replaces it within a frame, so this is
                 * the only window in which it can be read. */
                if (authenticated == 0)
                {
                    if (current != 0)
                        derived = current;

                    libpsutil::sleep(30);
                    continue;
                }

                /* The frame cave has not acted yet, or is not installed. */
                if (current != authenticated)
                {
                    libpsutil::sleep(30);
                    continue;
                }

                if (!announced)
                {
                    announced = true;

                    printf("[CODPatch Loader] MW3: identity is now 0x%08X%08X\n",
                        (u32)(authenticated >> 32), (u32)authenticated);
                }

                if (derived != 0 && quiet < SweepQuietPasses)
                {
                    const u32 replaced = SweepDerivedId(derived, authenticated);

                    if (replaced == 0)
                    {
                        quiet++;
                    }
                    else
                    {
                        quiet = 0;

                        printf("[CODPatch Loader] MW3: took back %u copies the caves missed\n",
                            replaced);
                    }

                    libpsutil::sleep(250);
                    continue;
                }

                libpsutil::sleep(1000);
            }
        }

        /* A relative branch only carries 32MB and the caves are in this module, so the
         * entry goes through CTR. Both r11 and CTR are volatile at a function entry,
         * and each site is a function entry, so nothing can land in the middle of the
         * sequence. */
        bool InstallStub(u32 address, const u8* cave, const char* what)
        {
            const u32 target = (u32)cave;

            /* lis sign extends, so this only reaches the low half of the address
             * space. Everything the PRX loader hands out sits well below that, but a
             * wrong jump here hangs the console rather than logging anything. */
            if (target >= 0x80000000u)
            {
                printf("[CODPatch Loader] MW3: %s cave at 0x%08X is out of reach\n",
                    what, target);
                return false;
            }

            u32 stub[4] = {
                0x3D600000u | (target >> 16),    /* lis   r11, cave@h      */
                0x616B0000u | (target & 0xFFFF), /* ori   r11, r11, cave@l */
                0x7D6903A6u,                     /* mtctr r11              */
                0x4E800420u,                     /* bctr                   */
            };

            if (!libpsutil::memory::set(address, stub, sizeof(stub)))
            {
                printf("[CODPatch Loader] MW3: could not patch %s at 0x%08X\n", what, address);
                return false;
            }

            /* Read the site back rather than trusting the write. */
            const volatile u32* site = (const volatile u32*)address;

            printf("[CODPatch Loader] MW3: %s 0x%08X -> 0x%08X, reads %08X %08X %08X %08X\n",
                what, address, target, site[0], site[1], site[2], site[3]);

            return true;
        }
    }

    /* Runs when the player's stats come back from Demonware. A brand new account has
     * nothing to hand back, which the game turns into a login error, so it gets to do
     * nothing instead. */
    int DwFetchPerformanceValuesComplete(int task, int playerRanks, int* numPlayerRanks)
    {
        return 0;
    }

    void Main()
    {
        // Guard in case there is a new thread. Arguably redundant.
        static bool patched = false;
        if (patched)
            return;

        patched = true;

        /* Make sure we find at least the dw function we're meant to replace.
         * All MW3 builds are guaranteed to have this signature. So if one doesn't, it isn't the right candidate.
         * scan interval isn't actually needed and shouldn't be required. */
        u32 dwFunctionFound = LoaderCommon::FindFunctionAddress(
            DwFetchPerformanceValuesCompleteAddress, dwFetchPerformanceCompleteSignature,
            dwFetchPerformanceCompleteSignatureLength, 512
        );

        if (dwFunctionFound == 0)
        {
            printf("[CODPatch Loader] MW3: DwFetchPerformanceValuesComplete signature could not be located, which means we're quitting for this executable.\n");
            return;
        }

        if (EnableProfileLookupDetour)
        {
            /* Never freed on purpose. The detour puts the original instructions back
             * in its destructor, so it has to outlive this call. */
            new libpsutil::memory::detour(DwFetchPerformanceValuesCompleteAddress,
                DwFetchPerformanceValuesComplete);

            printf("[CODPatch Loader] MW3: profile lookup errors ignored\n");
        }

        InstallStub(DwGetOnlineUserIDAddress, mw3_cave_start, "dwGetOnlineUserID");
        InstallStub(FrameAddress, mw3_frame_cave_start, "frame pump");

        sys_ppu_thread_t watchThreadId;
        sys_ppu_thread_create(&watchThreadId, UserIdWatch, 0, 1059, 8192,
            SYS_PPU_THREAD_CREATE_JOINABLE, "CODPatch_MW3UserId");
    }
}
