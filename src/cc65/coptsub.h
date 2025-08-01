////////////////////////////////////////////////////////////////////////////////
//
//                                 coptsub.h
//
//                      Optimize subtraction sequences
//
//
//
// (C) 2001-2006, Ullrich von Bassewitz
//                Roemerstrasse 52
//                D-70794 Filderstadt
// EMail:         uz@cc65.org
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

#ifndef COPTSUB_H
#define COPTSUB_H

// cc65
#include "codeseg.h"

////////////////////////////////////////////////////////////////////////////////
//                           Optimize subtractions
////////////////////////////////////////////////////////////////////////////////

unsigned OptSub1(CodeSeg *S);
// Search for the sequence
//
// sbc     ...
// bcs     L
// dex
// L:
//
// and remove the handling of the high byte if X is not used later.

unsigned OptSub2(CodeSeg *S);
// Search for the sequence
//
// lda     xx
// sec
// sta     tmp1
// lda     yy
// sbc     tmp1
// sta     yy
//
// and replace it by
//
// sec
// lda     yy
// sbc     xx
// sta     yy

unsigned OptSub3(CodeSeg *S);
// Search for a call to decaxn and replace it by an 8 bit sub if the X register
// is not used later.

// End of coptsub.h

#endif
