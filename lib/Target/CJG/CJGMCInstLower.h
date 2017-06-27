//===-- CJGMCInstLower.h - Lower MachineInstr to MCInst --------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef CJGMCINSTLOWER_H
#define CJGMCINSTLOWER_H
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class AsmPrinter;

/// \brief This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY CJGMCInstLower {
  MCContext &Ctx;
  AsmPrinter &Printer;

public:
  CJGMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
  	: Ctx(Ctx), Printer(Printer) {}
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  MCSymbol *GetGlobalAddressSymbol(const MachineOperand &MO) const;
};
}

#endif
