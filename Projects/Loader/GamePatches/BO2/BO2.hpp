#pragma once

namespace BO2
{
	/*
	 * NOPs the debug lines as covered by setsid:
	 * https://github.com/setsid/bo2-ps3-psn-freeze-fix
	 * all credit goes to him.
	 * 
	 * I guess you could still try to chase the XUID
	 * missmatch error, as that is highly the case
	 * on why this actually happens but for BO2 there
	 * have been absolutely zero reasons to do that
	 * over just doing this as far as I know right now.
	 * @returns
	 */
	void PerformNOP();

	void Main();
}