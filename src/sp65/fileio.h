////////////////////////////////////////////////////////////////////////////////
//
//                                 fileio.h
//
//              File I/O for the sp65 sprite and bitmap utility
//
//
//
// (C) 1998-2012, Ullrich von Bassewitz
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

#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//                                   Code
////////////////////////////////////////////////////////////////////////////////

void FileSetPos(FILE *F, unsigned long Pos);
// Seek to the given absolute position, fail on errors

unsigned long FileGetPos(FILE *F);
// Return the current file position, fail on errors

unsigned Read8(FILE *F);
// Read an 8 bit value from the file

unsigned Read16(FILE *F);
// Read a 16 bit value from the file

unsigned long Read24(FILE *F);
// Read a 24 bit value from the file

unsigned long Read32(FILE *F);
// Read a 32 bit value from the file

void *ReadData(FILE *F, void *Data, unsigned Size);
// Read data from the file

// End of fileio.h

#endif
