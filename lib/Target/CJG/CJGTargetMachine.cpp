//===-- CJGTargetMachine.cpp - Define TargetMachine for CJG ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about CJG target spec.
//
//===----------------------------------------------------------------------===//

#include "CJGTargetMachine.h"
#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options) {
  // Build the data layout
  // http://llvm.org/docs/LangRef.html#data-layout

  std::string dataLayout = "";

  dataLayout += "e"; // Little-endian
  dataLayout += "-m:e"; // ELF style name mangling
  dataLayout += "-p:32:32"; // Set 32-bit pointer size with 32-bit alignment
  dataLayout += "-i1:8:32"; // Align i1 to a 32-bit word
  dataLayout += "-i8:8:32"; // Align i8 to a 32-bit word
  dataLayout += "-i16:16:32"; // Align i16 to a 32-bit word
  dataLayout += "-i64:32"; // Align i64 to a 32-bit word
  dataLayout += "-f64:32"; // Align f64 to a 32-bit word
  dataLayout += "-a:0:32"; // Align aggregates to a 32-bit word
  dataLayout += "-n32"; // Set native integer width to 32-bits
  
  // "e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-i64:32-f64:32-a:0:32-n32"

  return dataLayout;
}

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    // Default relocation model on Darwin is PIC.
    return TT.isOSBinFormatMachO() ? Reloc::PIC_ : Reloc::Static;

  // DynamicNoPIC is only used on darwin.
  if (*RM == Reloc::DynamicNoPIC && !TT.isOSDarwin())
    return Reloc::Static;

  return *RM;
}

CJGTargetMachine::CJGTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   CodeModel::Model CM, CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(TT, RM), CM, OL),
      Subtarget(TT, CPU, FS, *this),
      TLOF(make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

namespace {
class CJGPassConfig : public TargetPassConfig {
public:
  CJGPassConfig(CJGTargetMachine *TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  CJGTargetMachine &getCJGTargetMachine() const {
    return getTM<CJGTargetMachine>();
  }

  virtual bool addInstSelector() override;
};
}

TargetPassConfig *CJGTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new CJGPassConfig(this, PM);
}

bool CJGPassConfig::addInstSelector() {
  addPass(createCJGISelDag(getCJGTargetMachine(), getOptLevel()));
  return false;
}

// Force static initialization.
extern "C" void LLVMInitializeCJGTarget() {
  RegisterTargetMachine<CJGTargetMachine> X(getTheCJGTarget());
}
