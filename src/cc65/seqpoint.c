////////////////////////////////////////////////////////////////////////////////
//
//                                 seqpoint.h
//
//                     Stuff involved in sequence points
//
//
//
// Copyright 2022 The cc65 Authors
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

// cc65
#include "seqpoint.h"

////////////////////////////////////////////////////////////////////////////////
//                                   data
////////////////////////////////////////////////////////////////////////////////

// Remeber if __A__, __AX__ and __EAX__ are being used
unsigned PendingSqpFlags = SQP_KEEP_NONE;

////////////////////////////////////////////////////////////////////////////////
//                                   code
////////////////////////////////////////////////////////////////////////////////

void SetSQPFlags(unsigned Flags)
// Set the SQP_KEEP_* flags for the deferred operations in the statement
{
   PendingSqpFlags = Flags;
}

unsigned GetSQPFlags(void)
// Get the SQP_KEEP_* flags for the deferred operations in the statement
{
   return PendingSqpFlags;
}
