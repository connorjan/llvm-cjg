//===-- CJGInstPrinter.cpp - Convert CJG MCInst to asm syntax -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an CJG MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "CJGInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#include "CJGGenAsmWriter.inc"

void CJGInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot, const MCSubtargetInfo &STI) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void CJGInstPrinter::printRegName(raw_ostream &O, unsigned RegNo) const {
  O << StringRef(getRegisterName(RegNo));
}

void CJGInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);

  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    O << Op.getImm();
    return;
  }

  llvm_unreachable("Unknown operand");
  // assert(Op.isExpr() && "Unknown operand kind in printOperand");
  // Op.getExpr()->print(O, &MAI);
}

// Print a memsrc (defined in CJGInstrInfo.td)
// This is an operand which defines a location for loading or storing which
// is a register offset by an immediate value
void CJGInstPrinter::printMemSrcOperand(const MCInst *MI, unsigned OpNo,
                                           raw_ostream &O) {
  const MCOperand &BaseAddr = MI->getOperand(OpNo);
  const MCOperand &Offset = MI->getOperand(OpNo + 1);
  
  assert(Offset.isImm() && "Expected immediate in displacement field");

  O << "M[";
  printRegName(O, BaseAddr.getReg());
  unsigned OffsetVal = Offset.getImm();
  if (OffsetVal) {
    O << "+" << Offset.getImm();
  }
  O << "]";
}
