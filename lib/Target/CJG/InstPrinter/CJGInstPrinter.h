//== CJGInstPrinter.h - Convert CJG MCInst to assembly syntax -----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file contains the declaration of the CJGInstPrinter class,
/// which is used to print CJG MCInst to a .s file.
///
//===----------------------------------------------------------------------===//

#ifndef CJGINSTPRINTER_H
#define CJGINSTPRINTER_H
#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class TargetMachine;

class CJGInstPrinter : public MCInstPrinter {
public:
  CJGInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                 const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  virtual void printInst(const MCInst *MI, raw_ostream &OS, StringRef Annot,
                         const MCSubtargetInfo &STI) override;

  virtual void printRegName(raw_ostream &OS, unsigned RegNo) const override;
  
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // Autogenerated by tblgen.
  void printInstruction(const MCInst *MI, raw_ostream &O);
  static const char *getRegisterName(unsigned RegNo);

// private:
  // void printCondCode(const MCInst *MI, unsigned OpNum, raw_ostream &O);
  // void printAddrModeMemSrc(const MCInst *MI, unsigned OpNum, raw_ostream &O);
  // void printMemOperand(const MCInst *MI, int opNum, raw_ostream &O);
};
} // end namespace llvm

#endif
