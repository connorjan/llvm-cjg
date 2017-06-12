//===-- CJGMCTargetDesc.cpp - CJG Target Descriptions ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides CJG specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "CJGMCTargetDesc.h"
#include "CJGMCAsmInfo.h"
#include "InstPrinter/CJGInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "CJGGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "CJGGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "CJGGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createCJGMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCJGMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createCJGMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCJGMCRegisterInfo(X, CJG::PC);
  return X;
}

static MCAsmInfo *createCJGMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT) { 
  return new CJGMCAsmInfo(TT);
}

static MCInstPrinter *
createCJGMCInstPrinter(const Triple &TT, unsigned SyntaxVariant,
                       const MCAsmInfo &MAI, const MCInstrInfo &MII,
                       const MCRegisterInfo &MRI) {
  return new CJGInstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeCJGTargetMC() {
  RegisterMCAsmInfoFn X(getTheCJGTarget(), createCJGMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(getTheCJGTarget(), createCJGMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(getTheCJGTarget(), createCJGMCRegisterInfo);
  TargetRegistry::RegisterMCAsmBackend(getTheCJGTarget(), createCJGAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(getTheCJGTarget(), createCJGMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(getTheCJGTarget(), createCJGMCInstPrinter);
}
