////////////////////////////////////////////////////////////////////////////////
//
//                                 handler.c
//
//               Opcode handler functions for the disassembler
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

#include <inttypes.h>
#include <stdarg.h>

// common
#include "xmalloc.h"
#include "xsprintf.h"

// da65
#include "attrtab.h"
#include "code.h"
#include "error.h"
#include "global.h"
#include "handler.h"
#include "labels.h"
#include "opctable.h"
#include "opc45GS02.h"
#include "output.h"

static unsigned short SubroutineParamSize[0x10000];

////////////////////////////////////////////////////////////////////////////////
//                             Helper functions
////////////////////////////////////////////////////////////////////////////////

static void Mnemonic(const char *M)
// Indent and output a mnemonic
{
   Indent(MCol);
   Output("%s", M);
}

static void OneLine(const OpcDesc *D, const char *Arg, ...)
    attribute((format(printf, 2, 3)));
static void OneLine(const OpcDesc *D, const char *Arg, ...)
// Output one line with the given mnemonic and argument
{
   char Buf[256];
   va_list ap;

   // Mnemonic
   Mnemonic(D->Mnemo);

   // Argument
   va_start(ap, Arg);
   xvsprintf(Buf, sizeof(Buf), Arg, ap);
   va_end(ap);
   Indent(ACol);
   Output("%s", Buf);

   // Add the code stuff as comment
   LineComment(PC, D->Size);

   // End the line
   LineFeed();
}

static const char *GetAbsOverride(unsigned Flags, uint32_t Addr)
// If the instruction requires an abs override modifier, return the necessary
// string, otherwise return the empty string.
{
   if ((Flags & flFarOverride) != 0 && Addr < 0x10000) {
      return "f:";
   }
   else if ((Flags & flAbsOverride) != 0 && Addr < 0x100) {
      return "a:";
   }
   else {
      return "";
   }
}

static const char *GetAddrArg(unsigned Flags, uint32_t Addr)
// Return an address argument - a label if we have one, or the address itself
{
   const char *Label = 0;
   if (Flags & flUseLabel) {
      Label = GetLabel(Addr, PC);
   }
   if (Label) {
      return Label;
   }
   else {
      static char Buf[32];
      if (Addr < 0x100) {
         xsprintf(Buf, sizeof(Buf), "$%02" PRIX32, Addr);
      }
      else {
         xsprintf(Buf, sizeof(Buf), "$%04" PRIX32, Addr);
      }
      return Buf;
   }
}

static void GenerateLabel(unsigned Flags, uint32_t Addr)
// Generate a label in pass one if requested
{
   // Generate labels in pass #1, and only if we don't have a label already
   if (Pass == 1 && !HaveLabel(Addr) &&
       // Check if we must create a label
       ((Flags & flGenLabel) != 0 ||
        ((Flags & flUseLabel) != 0 && Addr >= CodeStart && Addr <= CodeEnd))) {

      // As a special case, handle ranges with tables or similar. Within
      // such a range with a granularity > 1, do only generate dependent
      // labels for all addresses but the first one. Be sure to generate
      // a label for the start of the range, however.
      attr_t Style = GetStyleAttr(Addr);
      unsigned Granularity = GetGranularity(Style);

      if (Granularity == 1) {
         // Just add the label
         AddIntLabel(Addr);
      }
      else {

         // THIS CODE IS A MESS AND WILL FAIL ON SEVERAL CONDITIONS! ###

         // Search for the start of the range or the last non dependent
         // label in the range.
         unsigned Offs;
         attr_t LabelAttr;
         unsigned LabelAddr = Addr;
         while (LabelAddr > CodeStart) {

            if (Style != GetStyleAttr(LabelAddr - 1)) {
               // End of range reached
               break;
            }
            --LabelAddr;
            LabelAttr = GetLabelAttr(LabelAddr);
            if ((LabelAttr & (atIntLabel | atExtLabel)) != 0) {
               // The address has an internal or external label
               break;
            }
         }

         // If the proposed label address doesn't have a label, define one
         if ((GetLabelAttr(LabelAddr) & (atIntLabel | atExtLabel)) == 0) {
            AddIntLabel(LabelAddr);
         }

         // Create the label
         Offs = Addr - LabelAddr;
         if (Offs == 0) {
            AddIntLabel(Addr);
         }
         else {
            AddDepLabel(Addr, atIntLabel, GetLabelName(LabelAddr), Offs);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
//                                   Code
////////////////////////////////////////////////////////////////////////////////

void OH_Illegal(const OpcDesc *D attribute((unused))) { DataByteLine(1); }

void OH_Accumulator(const OpcDesc *D) { OneLine(D, "a"); }

void OH_Implicit(const OpcDesc *D) {
   Mnemonic(D->Mnemo);
   LineComment(PC, D->Size);
   LineFeed();
}

void OH_Implicit_ea_45GS02(const OpcDesc *D)
// handle disassembling EOM prefixed opcodes, $ea $xx
{
   // Get byte after EOM
   unsigned opc = GetCodeByte(PC + 1);
   static const char *mnemonics[8] = {"ora", "and", "eor", "adc",
                                      "sta", "lda", "cmp", "sbc"};

   if ((opc & 0x1f) == 0x12) {
      unsigned zp = GetCodeByte(PC + 2);

      Indent(MCol);
      Output("%s", mnemonics[(opc >> 5) & 7]);

      Indent(ACol);
      Output("[$%02X],z", zp);

      // Add the code stuff as comment
      LineComment(PC, 3);

      // End the line
      LineFeed();
   }
   else {
      OH_Implicit(D);
   }
}

void OH_Implicit_42_45GS02(const OpcDesc *D)
// handle disassembling NEG NEG prefixed opcodes, $42 42 ($ea) $xx
{
   // Get bytes after NEG
   unsigned n1 = GetCodeByte(PC + 1);
   unsigned opc = GetCodeByte(PC + 2);
   // NEG:NEG:NOP (42 42 ea) prefix
   static const char *mnemonics[8] = {"orq", "andq", "eorq", "adcq",
                                      "stq", "ldq",  "cmpq", "sbcq"};

   if (n1 == 0x42) {
      // detected 0x42 0x42
      if (opc == 0xea) {
         // detected 0x42 0x42 0xea
         unsigned opc = GetCodeByte(PC + 3);
         if ((opc & 0x1f) == 0x12) {
            unsigned zp = GetCodeByte(PC + 4);

            Indent(MCol);
            Output("%s", mnemonics[(opc >> 5) & 7]);

            Indent(ACol);
            Output("[$%02X]", zp); // with or without ,z ?

            // Add the code stuff as comment
            LineComment(PC, 5);

            // End the line
            LineFeed();
            return;
         }
      }
      else {
         // use another table for these
         const OpcDesc *NewDesc = &OpcTable_45GS02_extended[opc];
         NewDesc->Handler(NewDesc);
         return;
      }
   }

   // no prefix detected
   OH_Implicit(D);
}

void OH_Immediate(const OpcDesc *D) {
   OneLine(D, "#$%02" PRIX8, GetCodeByte(PC + 1));
}

void OH_Immediate65816M(const OpcDesc *D) {
   if (GetAttr(PC) & atMem16) {
      OneLine(D, "#$%04" PRIX16, GetCodeWord(PC + 1));
   }
   else {
      OneLine(D, "#$%02" PRIX8, GetCodeByte(PC + 1));
   }
}

void OH_Immediate65816X(const OpcDesc *D) {
   if (GetAttr(PC) & atIdx16) {
      OneLine(D, "#$%04" PRIX16, GetCodeWord(PC + 1));
   }
   else {
      OneLine(D, "#$%02" PRIX8, GetCodeByte(PC + 1));
   }
}

void OH_ImmediateWord(const OpcDesc *D) {
   OneLine(D, "#$%04" PRIX16, GetCodeWord(PC + 1));
}

void OH_Direct(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_Direct_Q(const OpcDesc *D) {
   // Get the operand
   unsigned Addr = GetCodeByte(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_DirectX(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s,x", GetAddrArg(D->Flags, Addr));
}

void OH_DirectX_Q(const OpcDesc *D) {
   // Get the operand
   unsigned Addr = GetCodeByte(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s,x", GetAddrArg(D->Flags, Addr));
}

void OH_DirectY(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s,y", GetAddrArg(D->Flags, Addr));
}

void OH_Absolute(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_Absolute_Q(const OpcDesc *D) {
   // Get the operand
   unsigned Addr = GetCodeWord(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteX(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s,x", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteX_Q(const OpcDesc *D) {
   // Get the operand
   unsigned Addr = GetCodeWord(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s,x", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteY(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s,y", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteLong(const OpcDesc *D attribute((unused))) {
   // Get the operand
   uint32_t Addr = GetCodeLongAddr(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteLongX(const OpcDesc *D attribute((unused))) {
   // Get the operand
   uint32_t Addr = GetCodeLongAddr(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s%s,x", GetAbsOverride(D->Flags, Addr),
           GetAddrArg(D->Flags, Addr));
}

void OH_Relative(const OpcDesc *D) {
   // Get the operand
   signed char Offs = GetCodeByte(PC + 1);

   // Calculate the target address
   uint32_t Addr = (((int)PC + 2) + Offs) & 0xFFFF;

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   if (HaveLabel(Addr)) {
      OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
   }
   else {
      // No label -- make a relative address expression
      OneLine(D, "*%+d", (int)Offs + 2);
   }
}

void OH_RelativeLong(const OpcDesc *D attribute((unused))) {
   // Get the operand
   signed short Offs = GetCodeWord(PC + 1);

   // Calculate the target address
   uint32_t Addr = (((int)PC + 3) + Offs) & 0xFFFF;

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_RelativeLong4510(const OpcDesc *D attribute((unused))) {
   // Get the operand
   signed short Offs = GetCodeWord(PC + 1);

   // Calculate the target address
   uint32_t Addr = (((int)PC + 2) + Offs) & 0xFFFF;

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_DirectIndirect(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s)", GetAddrArg(D->Flags, Addr));
}

void OH_DirectIndirectY(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s),y", GetAddrArg(D->Flags, Addr));
}

void OH_DirectIndirectZ(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s),z", GetAddrArg(D->Flags, Addr));
}

void OH_DirectIndirectZ_Q(const OpcDesc *D) {
   // Get the operand
   unsigned Addr = GetCodeByte(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s),z", GetAddrArg(D->Flags, Addr));
}

void OH_DirectXIndirect(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s,x)", GetAddrArg(D->Flags, Addr));
}

void OH_AbsoluteIndirect(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s)", GetAddrArg(D->Flags, Addr));
}

void OH_BitBranch(const OpcDesc *D) {
   char *BranchLabel;

   // Get the operands
   uint32_t TestAddr = GetCodeByte(PC + 1);
   int8_t BranchOffs = GetCodeByte(PC + 2);

   // Calculate the target address for the branch
   uint32_t BranchAddr = (((int)PC + 3) + BranchOffs) & 0xFFFF;

   // Generate labels in pass 1. The bit branch codes are special in that
   // they don't really match the remainder of the 6502 instruction set (they
   // are a Rockwell addon), so we must pass additional flags as direct
   // value to the second GenerateLabel call.
   GenerateLabel(D->Flags, TestAddr);
   GenerateLabel(flLabel, BranchAddr);

   // Make a copy of an operand, so that
   // the other operand can't overwrite it.
   // [GetAddrArg() uses a statically-stored buffer.]
   BranchLabel = xstrdup(GetAddrArg(flLabel, BranchAddr));

   // Output the line
   OneLine(D, "%s,%s", GetAddrArg(D->Flags, TestAddr), BranchLabel);

   xfree(BranchLabel);
}

void OH_BitBranch_m740(const OpcDesc *D)
// <bit> zp, rel
// NOTE: currently <bit> is part of the instruction
{
   // unsigned Bit = GetCodeByte (PC) >> 5;
   uint32_t Addr = GetCodeByte(PC + 1);
   int8_t BranchOffs = (int8_t)GetCodeByte(PC + 2);

   // Calculate the target address for the branch
   uint32_t BranchAddr = (((int)PC + 3) + BranchOffs) & 0xFFFF;

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);
   GenerateLabel(flLabel, BranchAddr);

   // Output the line
   OneLine(D, "%s, %s", GetAddrArg(D->Flags, Addr),
           GetAddrArg(flLabel, BranchAddr));
}

void OH_ImmediateDirect(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 2);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "#$%02X,%s", GetCodeByte(PC + 1), GetAddrArg(D->Flags, Addr));
}

void OH_ImmediateDirectX(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 2);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "#$%02X,%s,x", GetCodeByte(PC + 1), GetAddrArg(D->Flags, Addr));
}

void OH_ImmediateAbsolute(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 2);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "#$%02X,%s%s", GetCodeByte(PC + 1),
           GetAbsOverride(D->Flags, Addr), GetAddrArg(D->Flags, Addr));
}

void OH_ImmediateAbsoluteX(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 2);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "#$%02X,%s%s,x", GetCodeByte(PC + 1),
           GetAbsOverride(D->Flags, Addr), GetAddrArg(D->Flags, Addr));
}

void OH_StackRelative(const OpcDesc *D attribute((unused))) {
   // Output the line
   OneLine(D, "$%02X,s", GetCodeByte(PC + 1));
}

void OH_DirectIndirectLongX(const OpcDesc *D attribute((unused))) {
   Error("Not implemented %s", __FUNCTION__);
}

void OH_StackRelativeIndirectY(const OpcDesc *D attribute((unused))) {
   // Output the line
   OneLine(D, "($%02X,s),y", GetCodeByte(PC + 1));
}

void OH_StackRelativeIndirectY4510(const OpcDesc *D attribute((unused))) {
   // Output the line
   OneLine(D, "($%02X,sp),y", GetCodeByte(PC + 1));
}

void OH_DirectIndirectLong(const OpcDesc *D attribute((unused))) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "[%s]", GetAddrArg(D->Flags, Addr));
}

void OH_DirectIndirectLongY(const OpcDesc *D attribute((unused))) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "[%s],y", GetAddrArg(D->Flags, Addr));
}

void OH_BlockMove(const OpcDesc *D) {
   char *DstLabel;

   // Get source operand
   uint32_t Src = GetCodeWord(PC + 1);
   // Get destination operand
   uint32_t Dst = GetCodeWord(PC + 3);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Src);
   GenerateLabel(D->Flags, Dst);

   // Make a copy of an operand, so that
   // the other operand can't overwrite it.
   // [GetAddrArg() uses a statically-stored buffer.]
   DstLabel = xstrdup(GetAddrArg(D->Flags, Dst));

   // Output the line
   OneLine(D, "%s%s,%s%s,$%04" PRIX16, GetAbsOverride(D->Flags, Src),
           GetAddrArg(D->Flags, Src), GetAbsOverride(D->Flags, Dst), DstLabel,
           GetCodeWord(PC + 5));

   xfree(DstLabel);
}

void OH_BlockMove65816(const OpcDesc *D) {
   // Get source operand
   uint8_t Src = GetCodeByte(PC + 2);
   // Get destination operand
   uint8_t Dst = GetCodeByte(PC + 1);

   // Output the line
   OneLine(D, "#$%02" PRIX8 ", #$%02" PRIX8, Src, Dst);
}

void OH_AbsoluteXIndirect(const OpcDesc *D attribute((unused))) {
   // Get the operand
   uint32_t Addr = GetCodeWord(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "(%s,x)", GetAddrArg(D->Flags, Addr));
}

void OH_DirectImmediate(const OpcDesc *D) {
   // Get the operand
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s, #$%02X", GetAddrArg(D->Flags, Addr), GetCodeByte(PC + 2));
}

void OH_ZeroPageBit(const OpcDesc *D)
// <bit> zp
// NOTE: currently <bit> is part of the instruction
{
   uint32_t Addr = GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   // Output the line
   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_AccumulatorBit(const OpcDesc *D)
// <bit> A
// NOTE: currently <bit> is part of the instruction
{
   // Output the line
   OneLine(D, "a");
}

void OH_AccumulatorBitBranch(const OpcDesc *D)
// <bit> A, rel
// NOTE: currently <bit> is part of the instruction
{
   int8_t BranchOffs = GetCodeByte(PC + 1);

   // Calculate the target address for the branch
   uint32_t BranchAddr = (((int)PC + 3) + BranchOffs) & 0xFFFF;

   // Generate labels in pass 1
   GenerateLabel(flLabel, BranchAddr);

   // Output the line
   OneLine(D, "a, %s", GetAddrArg(flLabel, BranchAddr));
}

void OH_JmpDirectIndirect(const OpcDesc *D) {
   OH_DirectIndirect(D);
   if (NewlineAfterJMP) {
      LineFeed();
   }
   SeparatorLine();
}

void OH_SpecialPage(const OpcDesc *D)
// m740 "special page" address mode
{
   // Get the operand
   uint32_t Addr = 0xFF00 + GetCodeByte(PC + 1);

   // Generate a label in pass 1
   GenerateLabel(D->Flags, Addr);

   OneLine(D, "%s", GetAddrArg(D->Flags, Addr));
}

void OH_Rts(const OpcDesc *D) {
   OH_Implicit(D);
   if (NewlineAfterRTS) {
      LineFeed();
   }
   SeparatorLine();
}

void OH_JmpAbsolute(const OpcDesc *D) {
   OH_Absolute(D);
   if (NewlineAfterJMP) {
      LineFeed();
   }
   SeparatorLine();
}

void OH_JmpAbsoluteIndirect(const OpcDesc *D) {
   OH_AbsoluteIndirect(D);
   if (NewlineAfterJMP) {
      LineFeed();
   }
   SeparatorLine();
}

void OH_JmpAbsoluteXIndirect(const OpcDesc *D) {
   OH_AbsoluteXIndirect(D);
   if (NewlineAfterJMP) {
      LineFeed();
   }
   SeparatorLine();
}

void OH_JsrAbsolute(const OpcDesc *D) {
   unsigned ParamSize = SubroutineParamSize[GetCodeWord(PC + 1)];
   OH_Absolute(D);
   if (ParamSize > 0) {
      uint32_t RemainingBytes;
      uint32_t BytesLeft;
      PC += D->Size;
      RemainingBytes = GetRemainingBytes();
      BytesLeft = (RemainingBytes < ParamSize) ? RemainingBytes : ParamSize;
      while (BytesLeft > 0) {
         uint32_t Chunk = (BytesLeft > BytesPerLine) ? BytesPerLine : BytesLeft;
         DataByteLine(Chunk);
         BytesLeft -= Chunk;
         PC += Chunk;
      }
      PC -= D->Size;
   }
}

void SetSubroutineParamSize(uint32_t Addr, unsigned Size) {
   SubroutineParamSize[Addr] = Size;
}
