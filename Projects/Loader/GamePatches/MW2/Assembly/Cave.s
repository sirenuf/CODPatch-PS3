/* Code cave for the MW2 multiplayer executable.
 *
 * Stock MW2 derives the stats XUID by Tiger hashing the PSN username, which for
 * newer or renamed accounts does not match the real Demonware account id. This
 * runs on the successful Demonware auth response and copies the authoritative
 * bdAuthTicket.user_id into the existing dwGetOnlineUserID cache, so stats reads
 * and writes both use the server authenticated XUID.
 * (source: https://github.com/jacob-schroeder/IW4Studio/commit/99ff4e35b171f2ad157c22923ab4d70ef576f551)
 *
 * Claude wrote this from the original helper (seen in the commit i linked above)
 * into assembly. I thought this was the best route for readability instead of
 * poking raw byte arrays into memory that I did before, I dont know ppc ISA assembly
 * myself. FTR here is a following write up (not written by me) for how it works:
 * 
 * Everything here is position independent - the only branch is the internal one
 * to the exit label. The branch back into the game depends on where the cave is
 * copied to, so MW2.cpp appends that instruction at run time.
 *
 * Entered by a branch rather than a call, so r1 and r2 are still the game's and
 * the stack offsets below refer to the hooked function's own frame. Clobbers r0,
 * r8, r10 and r11 only, and the early exit path leaves r11 as the game had it.
 */
	.section .text
	.globl mw2_cave_start
	.globl mw2_cave_end

mw2_cave_start:
	lwz    0, 0x98(1)         /* displaced instruction from the hook site   */
	ld     8, 0xB0(1)         /* bdAuthTicket.user_id                       */
	cmpdi  8, 0
	beq    1f                 /* zero keeps the stock fallback              */
	lis    11, 0x73
	lwz    11, -0x4534(11)    /* *(u32*)0x0072BACC, the user id cache       */
	std    8, 8(11)           /* store the server user id...                */
	lwsync                    /* ...and make it visible before the flag     */
	li     10, 1
	stb    10, 4(11)          /* publish valid last                         */
1:
mw2_cave_end:
