//===-- CJGAsmBackend.cpp - CJG Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CJGMCTargetDesc.h"
// #include "MCTargetDesc/CJGFixupKinds.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class CJGAsmBackend : public MCAsmBackend {
  uint8_t OSABI;

public:
  CJGAsmBackend(const Target &T, const StringRef TT, uint8_t OSABI)
     : MCAsmBackend(), OSABI(OSABI) {}
  
  ~CJGAsmBackend() {}

  MCObjectWriter *createObjectWriter(raw_pwrite_stream &OS) const override;

  bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override;

  unsigned getNumFixupKinds() const override {
//    return CJG::NumTargetFixupKinds;
      return 1;
  }
  
  void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value, bool IsPCRel) const override;

  bool mayNeedRelaxation(const MCInst &Inst) const override {
    return false;
  }
  
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    return false;
  }

  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {
    llvm_unreachable("CJGAsmBackend::relaxInstruction() unimplemented");
  }
};

bool CJGAsmBackend::writeNopData(uint64_t Count, MCObjectWriter *OW) const {
  // If we need to write nop data something went wrong
  if (Count == 0) {
      return true;
    }
  return false;
}

void CJGAsmBackend::applyFixup(const MCFixup &Fixup, char *Data,
                                 unsigned DataSize, uint64_t Value,
                                 bool IsPCRel) const {
  llvm_unreachable("CJGAsmBackend::applyFixup() unimplemented");
  return;
}

MCObjectWriter *CJGAsmBackend::createObjectWriter(raw_pwrite_stream &OS) const {
  return createCJGELFObjectWriter(OS, OSABI);
}

} // end anonymous namespace

MCAsmBackend *llvm::createCJGAsmBackend(const Target &T,
                                          const MCRegisterInfo &MRI,
                                          const Triple &TT, StringRef CPU,
                                          const MCTargetOptions &Options) {
  const uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new CJGAsmBackend(T, TT.getTriple(), OSABI);
}
