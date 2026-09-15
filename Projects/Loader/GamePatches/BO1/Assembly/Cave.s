/* Code cave for the Black Ops 1 multiplayer executable.
 *
 * Stock BO1 identifies itself to Demonware with Tiger(lowercase PSN online id),
 * truncated to 8 bytes and byte reversed - sub_865538, which is the SDKs own
 * bdAuthUtility::getUserID linked into the EBOOT. That is what Demonware issued
 * while online ids were permanent. Accounts created or renamed after Sony added
 * name changes in late 2018 are issued an id derived from the account id instead,
 * so the client reads and writes its stats under a key the server has never heard
 * of and nothing ever persists.
 *
 * The correct number CAN*** be computed on the console. The correct computation is
 * Tiger(decimal(accountid)). More on this in my research repository on my profile.
 * The reason why we dont just do it locally is because of portability. If we did it
 * locally it would mean old accounts who use this patch suddenly get affected.
 * It is generally the best idea to just use the hash retrieved from the server. It is
 * verified to be 100% accurate and related to the account in question. No double checks.
 *
 * sub_850A90 is the ticket deserializer. It is hooked rather than
 * bdAuthService::handleReply because several message types carry a ticket and
 * which one arrives is the servers choice - breakpointing the one handleReply
 * case that obviously parses a ticket never fired on a real sign in. Every exit
 * path in the deserializer, success and early out alike, converges on the top of
 * its epilogue, and r29 has held the destination pointer since before the first
 * branch and is never rewritten. So one hook there sees every caller with a live
 * dest pointer, without depending on the stack layout at all.
 *
 * The parsed bdAuthTicket:
 *
 *     +0x00  u32   magic, 0xEFBDADDE
 *     +0x04  u8    type
 *     +0x08  u32   title / build id
 *     +0x0C  u32   time issued
 *     +0x10  u32   time expires        (issued + 8 hours)
 *     +0x18  u64   license id
 *     +0x20  u64   m_userID            <- the whole point of this file
 *     +0x28  char  username[64]
 *     +0x68  u8    sessionKey[24]
 *
 * The deserializer has already byte reversed every multi byte field, so the id is
 * read with a plain ld and needs no swapping.
 *
 * Publishing it means satisfying sub_27E4C8, the accessor all 21 XUID call sites
 * go through. Its fast path is gated on two conditions, not one:
 *
 *     if (cachedCounter[i] != liveCounter[i]) { cachedCounter[i] = liveCounter[i];
 *                                               validFlag[i] = 0; }
 *     if (validFlag[i]) { *out = cachedXuid[i]; return 1; }
 *     ...otherwise recompute the Tiger hash over the top.
 *
 * Writing the id and setting the flag is therefore not enough on its own. If the
 * counters still disagree on the next call the accessor clears the flag first and
 * hashes straight over what was just published, which is exactly why plain memory
 * editing never worked on this game. Syncing the counter is what makes it stick.
 *
 * Slot 0 only, deliberately. That is the signed in PSN account; guests never had a
 * server identity to correct.
 *
 * Entered by a branch from the top of the epilogue, so this is not a function entry
 * and the usual "everything volatile is dead" argument has to be made for the
 * specific site: r8 through r12, CR0 and CTR are volatile and the rest of the
 * epilogue touches none of them, r3 already holds the return value and is left
 * alone, and LR is only written by the replayed mtlr, which is the stock code's own
 * doing. r29 is still dest here - it is restored from the frame further down, after
 * the point this cave returns to.
 *
 * The five addresses this needs all move between builds, so LoaderCommon's
 * signature scanner resolves them out of the game's own instructions and BO1.cpp
 * leaves them in the parameter block below. The two immediates at the top are the
 * block's address, filled in at install time: a PRX does not know its own load
 * address when it is assembled, and r2 on entry belongs to the game rather than to
 * this module, so its TOC is no use either.
 */
	.section .text
	.globl bo1_cave_start
	.globl bo1_cave_params

/* Offsets into BO1::CaveParams. Same order, same five words - keep both in step. */
	.set PARAM_LIVE_COUNTER,   0
	.set PARAM_CACHED_COUNTER, 4
	.set PARAM_XUID_SLOT,      8
	.set PARAM_VALID_FLAG,    12
	.set PARAM_RESUME,        16

	.align 2

bo1_cave_start:
bo1_cave_params:
	lis    12, 0                       /* patched: &CaveParams, high half      */
	ori    12, 12, 0                   /* patched: ...and low                  */

	/* sub_850A90 is generic and deserializes several kinds of message, so the
	 * magic is what says this one is a ticket. Nothing below runs otherwise. */
	lwz    9, 0(29)                    /* bdAuthTicket.magic                   */
	lis    10, 0xEFBD
	ori    10, 10, 0xADDE
	cmpw   0, 9, 10
	bne    1f

	ld     8, 0x20(29)                 /* bdAuthTicket.m_userID                */
	cmpdi  8, 0
	beq    1f                          /* a zero id is not worth publishing    */

	/* Agree with the live change counter, or the accessor invalidates the flag
	 * on its next call and hashes over everything written below. */
	lwz    9, PARAM_LIVE_COUNTER(12)
	lwz    9, 0(9)
	lwz    10, PARAM_CACHED_COUNTER(12)
	stw    9, 0(10)

	lwz    10, PARAM_XUID_SLOT(12)
	std    8, 0(10)                    /* the server authenticated XUID        */
	lwsync                             /* ...visible before the flag it gates  */
	lwz    10, PARAM_VALID_FLAG(12)
	li     9, 1
	stb    9, 0(10)                    /* published last                       */

	/* The four instructions the 16 byte entry jump displaced. They are the same
	 * bytes in every build seen, and the hook signature BO1.cpp matches on opens
	 * with exactly these four words, so a scan that succeeded has already proved
	 * them. */
1:	ld     27, 0xC0(1)
	ld     26, 0x80(1)
	mtlr   27
	ld     28, 0x90(1)

	lwz    11, PARAM_RESUME(12)        /* back into the epilogue at hook + 16  */
	mtctr  11
	bctr
