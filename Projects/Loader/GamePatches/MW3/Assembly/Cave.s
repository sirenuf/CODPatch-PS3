/* Code cave for the MW3 multiplayer executable.
 *
 * This shit is way more complicated in MW3 to get it working properly, rather
 * than just making the locally computed tiger hash always return your correct
 * XUID.
 *
 * Stock MW3 identifies itself with Tiger(PSN online id), which is what Demonware
 * issued back when online ids were permanent. Accounts created or renamed after
 * Sony added name changes in late 2018 are issued an id derived from the account
 * id instead, so client and server disagree about who you are, every profile
 * lookup naming you comes back BD_INVALID_USER_ID, which is what kicks you out of
 * a lobby and crashes the stock host you were joining.
 *
 * Same defect as MW2, but the two games need different handling, and the reason
 * is a race rather than a mechanism.
 *
 * MW2s cave fires on the Demonware auth response and fills the dwGetOnlineUserID
 * cache there. That is enough because auth lands before anything asks, so the
 * first call hits a cache that is already valid and the Tiger hash is never
 * computed at all - scan a patched MW2 and the Tiger value is nowhere in memory.
 *
 * MW3 asks first and authenticates second, and it cannot be made to do otherwise.
 * Refusing to answer until the ticket exists does suppress the derive - the Tiger
 * value then appears nowhere in memory, exactly as in MW2 - but it also kills the
 * connection, because the id the derive produces is what the auth request itself
 * is built from. Hand that zero and Demonware never answers.
 *
 * So MW3 gets the stock answer until the ticket lands, and FrameCave.s corrects
 * what was written in the meantime. That is the part MW2 does not need: by the time
 * MW3 knows who it really is, the derived id has already been written into the
 * local client record and copied into the lobby roster, and correcting this
 * function alone reaches none of that.
 *
 * The authenticated id lives at a fixed address. 0x01C488B0 is an array of 4 per
 * local client structures of 0xE0 bytes, built by sub_2FEAC4 and torn down by
 * sub_2FED68, each holding a bdAuthTicket at +0x18. m_userID is at +0x20 of the
 * ticket, so local client 0 has it at 0x01C488E8, with m_username right behind it
 * at 0x01C488F0. Nothing is derived here - an account old enough that its id
 * really is Tiger(online id) gets that same value back from the server, so this is
 * correct for both populations, which computing it locally would not be.
 *
 * The stock cache at 0x00731E30 and its flag at 0x00731E38 are kept in step the
 * way MW2s cave does, for anything reading those globals directly rather than
 * calling through. It doubles as a read out - if 0x00731E30 holds the
 * authenticated id then this has run.
 *
 * Entered by a branch rather than a call, so LR is still the callers, the early
 * return can just blr, and the replayed mflr still sees the right value. The entry
 * costs 4 instructions because it goes through CTR, hence the 4 replayed below and
 * the jump back to 0x00344EC4 rather than 0x00344EB8. Clobbers r9, r10, r11 and
 * CR0, all volatile at a function entry. Local client 0 is deliberate - stock
 * caches a single id in one global for every controller index, so this matches it.
 */
	.section .text
	.globl mw3_cave_start

mw3_cave_start:
	lis    11, 0x01C5
	ld     11, -0x7718(11)    /* 0x01C488E8, bdAuthTicket.m_userID            */
	cmpdi  11, 0
	beq    1f
	lis    10, 0x73
	std    11, 0x1E30(10)     /* keep the stock cache in step, as MW2 does    */
	li     9, 1
	stb    9, 0x1E38(10)      /* ...and its valid flag                        */
	std    11, 0(4)           /* *out = the server authenticated id           */
	li     3, 1               /* ...and report it valid                       */
	blr

1:	stdu   1, -0xA0(1)        /* no ticket yet, so fall through to stock      */
	mflr   0
	std    0, 0xB0(1)
	std    30, 0x90(1)
	lis    11, 0x34
	ori    11, 11, 0x4EC4     /* back into the stock body at 0x00344EC4       */
	mtctr  11
	bctr
