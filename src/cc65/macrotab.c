
//
//                                macrotab.h
//
//             Preprocessor macro table for the cc65 C compiler
//
//
//
// (C) 2000-2011, Ullrich von Bassewitz
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

#include <stdio.h>
#include <string.h>

// common
#include "hashfunc.h"
#include "xmalloc.h"

// cc65
#include "error.h"
#include "output.h"
#include "preproc.h"
#include "macrotab.h"

////////////////////////////////////////////////////////////////////////////////
//                                   data
////////////////////////////////////////////////////////////////////////////////

// The macro hash table
#define MACRO_TAB_SIZE 211
static Macro *MacroTab[MACRO_TAB_SIZE];

// The undefined macros list head
static Macro *UndefinedMacrosListHead;

// Some defines for better readability when calling OutputMacros()
#define USER_MACROS 0
#define PREDEF_MACROS 1
#define NAME_ONLY 0
#define FULL_DEFINITION 1

////////////////////////////////////////////////////////////////////////////////
//                                  helpers
////////////////////////////////////////////////////////////////////////////////

static void OutputMacro(const Macro *M, int Full)
// Output one macro. If Full is true, the replacement is also output.
{
   WriteOutput("#define %s", M->Name);
   int ParamCount = M->ParamCount;
   if (M->ParamCount >= 0) {
      int I;
      if (M->Variadic) {
         CHECK(ParamCount > 0);
         --ParamCount;
      }
      WriteOutput("(");
      for (I = 0; I < ParamCount; ++I) {
         const char *Name = CollConstAt(&M->Params, I);
         WriteOutput("%s%s", (I == 0) ? "" : ",", Name);
      }
      if (M->Variadic) {
         WriteOutput("%s...", (ParamCount == 0) ? "" : ",");
      }
      WriteOutput(")");
   }
   WriteOutput(" ");
   if (Full) {
      WriteOutput("%.*s", SB_GetLen(&M->Replacement),
                  SB_GetConstBuf(&M->Replacement));
   }
   WriteOutput("\n");
}

static void OutputMacros(int Predefined, int Full)
// Output macros to the output file depending on the flags given.
{
   // Note: The Full flag is currently not used by any callers but is left in
   // place for possible future changes.
   unsigned I;
   for (I = 0; I < MACRO_TAB_SIZE; ++I) {
      const Macro *M = MacroTab[I];
      while (M) {
         if ((Predefined != 0) == (M->Predefined != 0)) {
            OutputMacro(M, Full);
         }
         M = M->Next;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
//                                   code
////////////////////////////////////////////////////////////////////////////////

Macro *NewMacro(const char *Name, unsigned char Predefined)
// Allocate a macro structure with the given name. The structure is not
// inserted into the macro table.
{
   // Get the length of the macro name
   unsigned Len = strlen(Name);

   // Allocate the structure
   Macro *M = (Macro *)xmalloc(sizeof(Macro) + Len);

   // Initialize the data
   M->Next = 0;
   M->ParamCount = -1; // Flag: Not a function-like macro
   InitCollection(&M->Params);
   SB_Init(&M->Replacement);
   M->Predefined = Predefined;
   M->Variadic = 0;
   memcpy(M->Name, Name, Len + 1);

   // Return the new macro
   return M;
}

void FreeMacro(Macro *M)
// Delete a macro definition. The function will NOT remove the macro from the
// table, use UndefineMacro for that.
{
   unsigned I;

   for (I = 0; I < CollCount(&M->Params); ++I) {
      xfree(CollAtUnchecked(&M->Params, I));
   }
   DoneCollection(&M->Params);
   SB_Done(&M->Replacement);
   xfree(M);
}

Macro *CloneMacro(const Macro *M)
// Clone a macro definition. The function is not insert the macro into the
// macro table, thus the cloned instance cannot be freed with UndefineMacro.
// Use FreeMacro for that.
{
   Macro *New = NewMacro(M->Name, M->Predefined);
   unsigned I;

   for (I = 0; I < CollCount(&M->Params); ++I) {
      // Copy the parameter
      const char *Param = CollAtUnchecked(&M->Params, I);
      CollAppend(&New->Params, xstrdup(Param));
   }
   New->ParamCount = M->ParamCount;
   New->Variadic = M->Variadic;
   SB_Copy(&New->Replacement, &M->Replacement);

   return New;
}

void DefineNumericMacro(const char *Name, long Val)
// Define a predefined macro for a numeric constant
{
   char Buf[64];

   // Make a string from the number
   sprintf(Buf, "%ld", Val);

   // Handle as text macro
   DefineTextMacro(Name, Buf);
}

void DefineTextMacro(const char *Name, const char *Val)
// Define a predefined macro for a textual constant
{
   // Create a new macro
   Macro *M = NewMacro(Name, 1);

   // Set the value as replacement text
   SB_CopyStr(&M->Replacement, Val);

   // Insert the macro into the macro table
   InsertMacro(M);
}

void InsertMacro(Macro *M)
// Insert the given macro into the macro table.
{
   // Get the hash value of the macro name
   unsigned Hash = HashStr(M->Name) % MACRO_TAB_SIZE;

   // Insert the macro
   M->Next = MacroTab[Hash];
   MacroTab[Hash] = M;
}

Macro *UndefineMacro(const char *Name)
// Search for the macro with the given name, if it exists, remove it from
// the defined macro table and insert it to a list for pending deletion.
// Return the macro if it was found and removed, return 0 otherwise.
// To safely free the removed macro, use FreeUndefinedMacros().
{
   // Get the hash value of the macro name
   unsigned Hash = HashStr(Name) % MACRO_TAB_SIZE;

   // Search the hash chain
   Macro *L = 0;
   Macro *M = MacroTab[Hash];
   while (M) {
      if (strcmp(M->Name, Name) == 0) {

         // Found it
         if (L == 0) {
            // First in chain
            MacroTab[Hash] = M->Next;
         }
         else {
            L->Next = M->Next;
         }

         // Add this macro to pending deletion list
         M->Next = UndefinedMacrosListHead;
         UndefinedMacrosListHead = M;

         // Done
         return M;
      }

      // Next macro
      L = M;
      M = M->Next;
   }

   // Not found
   return 0;
}

void FreeUndefinedMacros(void)
// Free all undefined macros
{
   Macro *Next;

   while (UndefinedMacrosListHead != 0) {
      Next = UndefinedMacrosListHead->Next;

      // Delete the macro
      FreeMacro(UndefinedMacrosListHead);

      UndefinedMacrosListHead = Next;
   }
}

Macro *FindMacro(const char *Name)
// Find a macro with the given name. Return the macro definition or NULL
{
   // Get the hash value of the macro name
   unsigned Hash = HashStr(Name) % MACRO_TAB_SIZE;

   // Search the hash chain
   Macro *M = MacroTab[Hash];
   while (M) {
      if (strcmp(M->Name, Name) == 0) {
         // Check for some special macro names
         if (Name[0] == '_') {
            HandleSpecialMacro(M, Name);
         }
         // Found it
         return M;
      }

      // Next macro
      M = M->Next;
   }

   // Not found
   return 0;
}

int FindMacroParam(const Macro *M, const char *Param)
// Search for a macro parameter. If found, return the index of the parameter.
// If the parameter was not found, return -1.
{
   unsigned I;
   for (I = 0; I < CollCount(&M->Params); ++I) {
      if (strcmp(CollAtUnchecked(&M->Params, I), Param) == 0) {
         // Found
         return I;
      }
   }

   // Not found
   return -1;
}

void AddMacroParam(Macro *M, const char *Param)
// Add a macro parameter.
{
   // Check if we have a duplicate macro parameter, but add it anyway.
   // Beware: Don't use FindMacroParam here, since the actual argument array
   // may not be initialized.
   unsigned I;
   for (I = 0; I < CollCount(&M->Params); ++I) {
      if (strcmp(CollAtUnchecked(&M->Params, I), Param) == 0) {
         // Found
         PPError("Duplicate macro parameter: '%s'", Param);
         break;
      }
   }

   // Add the new parameter
   CollAppend(&M->Params, xstrdup(Param));
   ++M->ParamCount;
}

int MacroCmp(const Macro *M1, const Macro *M2)
// Compare two macros and return zero if both are identical.
{
   int I;

   // Argument count must be identical
   if (M1->ParamCount != M2->ParamCount) {
      return 1;
   }

   // Compare the parameters
   for (I = 0; I < M1->ParamCount; ++I) {
      if (strcmp(CollConstAt(&M1->Params, I), CollConstAt(&M2->Params, I)) !=
          0) {
         return 1;
      }
   }

   // Compare the replacement
   return SB_Compare(&M1->Replacement, &M2->Replacement);
}

void PrintMacroStats(FILE *F)
// Print macro statistics to the given text file.
{
   unsigned I;
   Macro *M;

   fprintf(F, "\n\nMacro Hash Table Summary\n");
   for (I = 0; I < MACRO_TAB_SIZE; ++I) {
      fprintf(F, "%3u : ", I);
      M = MacroTab[I];
      if (M) {
         while (M) {
            fprintf(F, "%s ", M->Name);
            M = M->Next;
         }
         fprintf(F, "\n");
      }
      else {
         fprintf(F, "empty\n");
      }
   }
}

void OutputPredefMacros(void)
// Output all predefined macros to the output file
{
   OutputMacros(PREDEF_MACROS, FULL_DEFINITION);
}

void OutputUserMacros(void)
// Output all user defined macros to the output file
{
   OutputMacros(USER_MACROS, FULL_DEFINITION);
}
