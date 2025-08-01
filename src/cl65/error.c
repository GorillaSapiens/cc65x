////////////////////////////////////////////////////////////////////////////////
//
//                                 global.c
//
//         Error handling for the cl65 compile and link utility
//
//
//
// (C) 1998     Ullrich von Bassewitz
//              Wacholderweg 14
//              D-70597 Stuttgart
// EMail:       uz@musoftware.de
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// common
#include "cmdline.h"

// cl65
#include "global.h"
#include "error.h"

////////////////////////////////////////////////////////////////////////////////
//                                   Code
////////////////////////////////////////////////////////////////////////////////

void Warning(const char *Format, ...)
// Print a warning message
{
   va_list ap;
   va_start(ap, Format);
   fprintf(stderr, "%s: ", ProgName);
   vfprintf(stderr, Format, ap);
   putc('\n', stderr);
   va_end(ap);
}

void Error(const char *Format, ...)
// Print an error message and die
{
   va_list ap;
   va_start(ap, Format);
   fprintf(stderr, "%s: ", ProgName);
   vfprintf(stderr, Format, ap);
   putc('\n', stderr);
   va_end(ap);
   exit(EXIT_FAILURE);
}

void Internal(const char *Format, ...)
// Print an internal error message and die
{
   va_list ap;
   va_start(ap, Format);
   fprintf(stderr, "%s: Internal error: ", ProgName);
   vfprintf(stderr, Format, ap);
   putc('\n', stderr);
   va_end(ap);
   exit(EXIT_FAILURE);
}
