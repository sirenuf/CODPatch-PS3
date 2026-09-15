/* Code cave on sub_2F2BC4, the MW3 multiplayer per frame system pump.
 *
 * The identity fix lives in Cave.s, on dwGetOnlineUserID. This is only about
 * when the correction happens, and on whose thread.
 *
 * MW3 writes its identity once, in sub_2F16F0, from dwGetOnlineUserID, while the
 * connection is coming up and before Demonware has answered. Nothing ever revisits
 * it, so the derived value has to be corrected from outside afterwards. Two things
 * make that awkward:
 *
 * Correcting the record from a worker thread races the rest of this function,
 * which reads it, so it can be caught mid change.
 *
 * And asking the game to redo the write itself, by disagreeing with
 * dword_1BBBC28 and letting sub_2F2224 queue a sign in transition, works, but a
 * lobby that is already up reacts to that transition by putting you in a party of
 * one. Which is fine at the main menu and not fine if Play Online was pressed
 * before the ticket landed.
 *
 * Doing it here solves both. This runs at the top of the frame, on the same thread
 * the game runs it on, before the body below does any comparing, so nothing is ever
 * seen half changed and no state transition is involved at all. The game is simply
 * told a different answer than the one it wrote, between frames.
 *
 * Everything that has to change together changes here: the record id, its hex form
 * at 0x01BBBC58, and the two copies the id had already reached. One of those is
 * roster entry 0 in the table sub_2FCD9C walks every frame, further down this same
 * function - leaving it stale for even a couple of frames is enough for the game to
 * decide the local player is not in the lobby and put you in a party of one. The
 * identity blob is left alone entirely, see below.
 *
 * Entered by a branch rather than a call, so LR is still the callers and the
 * replayed mflr sees the right value. Clobbers r6 through r11, CTR and CR0, all
 * volatile at a function entry; r3 carries this functions argument and is untouched.
 */
	.section .text
	.globl mw3_frame_cave_start

mw3_frame_cave_start:
	lis    11, 0x01C5
	ld     11, -0x7718(11)    /* 0x01C488E8, bdAuthTicket.m_userID            */
	cmpdi  11, 0
	beq    1f                 /* no ticket yet, nothing to correct            */

	lis    10, 0x01BC
	ld     9, -0x43B0(10)     /* 0x01BBBC50, local client record 0 id         */
	cmpd   0, 9, 11           /* ps3ppuas wants the CR field written out      */
	beq    1f                 /* already agrees, this is the usual case       */

	std    11, -0x43B0(10)    /* record 0 id = the authenticated id           */

	/* And record 0 + 0x30, the same value as 16 lowercase hex characters. Done
	 * here rather than from the watch thread so the id and its text form can
	 * never be caught disagreeing, sub_2EE750 and sub_2EE7F4 exist only to hand
	 * that text out, and their callers are menu code. Filled back to front, low
	 * nibble first; the +0x27 turns 0x3A..0x3F into 'a'..'f'. */
	addi   8, 10, -0x4398     /* 0x01BBBC68, one past the end of the field    */
	mr     6, 11
	li     7, 16
	mtctr  7
2:	andi.  7, 6, 15
	addi   7, 7, 0x30
	cmpwi  0, 7, 0x39
	ble    3f
	addi   7, 7, 39
3:	stbu   7, -1(8)
	srdi   6, 6, 4
	bdnz   2b

	/* The two copies the sweep kept finding, corrected here so they change in the
	 * same frame as the record rather than whenever the watch thread gets round to
	 * them. 0x01C12508 is byte_1C12500 + 8, and that structure belongs to
	 * sub_2FCD9C, which this very function calls further down, so leaving it
	 * stale means sub_2FCD9C spends a few frames being told the local player is
	 * someone who is not in the lobby.
	 *
	 * r9 still holds what the record said before we overwrote it, so each one is
	 * only touched where it still carries that exact value. Anything else is not
	 * ours to write. */
	lis    8, 0x008B
	ld     7, -0x7DE0(8)      /* 0x008A8220                                   */
	cmpd   0, 7, 9
	bne    4f
	std    11, -0x7DE0(8)

4:	lis    8, 0x01C1
	ld     7, 0x2508(8)       /* 0x01C12508, byte_1C12500 + 8                 */
	cmpd   0, 7, 9
	bne    1f
	std    11, 0x2508(8)

	/* The identity blob at 0x01C0F418 is deliberately left alone. It stays zero
	 * for the whole of a stock session, its only writers, sub_2F92C8 and
	 * sub_35C4D0, never run, which means the record search further down this
	 * function never finds a match and always takes sub_363364. Filling it in
	 * makes that search succeed and sends the game down sub_3655C4 instead, a
	 * branch it would otherwise never reach here. Writing it was a defensive
	 * guess and it cost a party. */

1:	stdu   1, -0x540(1)       /* the 4 displaced instructions                 */
	mflr   0
	std    0, 0x550(1)
	std    31, 0x538(1)
	lis    11, 0x2F
	ori    11, 11, 0x2BD4     /* back into the stock body at 0x002F2BD4       */
	mtctr  11
	bctr
