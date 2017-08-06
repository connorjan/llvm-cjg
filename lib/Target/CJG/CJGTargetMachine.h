//===-- CJGTargetMachine.h - Define TargetMachine for CJG -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the CJG specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef CJGTARGETMACHINE_H
#define CJGTARGETMACHINE_H

#include "CJG.h"
#include "CJGSubtarget.h"
#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class CJGTargetMachine : public LLVMTargetMachine {
  CJGSubtarget Subtarget;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  CJGTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM, CodeModel::Model CM,
                   CodeGenOpt::Level OL);

  // Pass Pipeline Configuration
  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  virtual const TargetSubtargetInfo *
  getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  
};

} // end namespace llvm

#endif
