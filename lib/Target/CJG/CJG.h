//===-- CJG.h - Top-level interface for CJG representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM CJG back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_CJG_H
#define TARGET_CJG_H

#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class CJGTargetMachine;
class MCInst;
class MachineInstr;

FunctionPass *createCJGISelDag(CJGTargetMachine &TM,
                               CodeGenOpt::Level OptLevel);

} // end namespace llvm;

#endif
