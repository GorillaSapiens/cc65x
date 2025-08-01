////////////////////////////////////////////////////////////////////////////////
//
//                                paravirt.h
//
//                Paravirtualization for the sim65 6502 simulator
//
//
//
// (C) 2013-2013 Ullrich von Bassewitz
//               Roemerstrasse 52
//               D-70794 Filderstadt
// EMail:        uz@cc65.org
//
//
// This software is provided 'as-is', without any expressed or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source
//    distribution.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef PARAVIRT_H
#define PARAVIRT_H

#include "6502.h"

////////////////////////////////////////////////////////////////////////////////
//                                   Data
////////////////////////////////////////////////////////////////////////////////

#define PARAVIRT_BASE 0xFFF1
// Lowest address used by a paravirtualization hook

#define PV_PATH_SIZE 1024
// Maximum path size supported by PVOpen/PVSysRemove

////////////////////////////////////////////////////////////////////////////////
//                                   Code
////////////////////////////////////////////////////////////////////////////////

void ParaVirtInit(unsigned aArgStart, unsigned char aSPAddr);
// Initialize the paravirtualization subsystem

void ParaVirtHooks(CPURegs *Regs);
// Potentially execute paravirtualization hooks

// End of paravirt.h

#endif
