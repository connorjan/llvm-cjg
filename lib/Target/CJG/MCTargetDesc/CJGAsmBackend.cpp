//===-- CJGAsmBackend.cpp - CJG Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "MCTargetDesc/CJGFixupKinds.h"
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
    return CJG::NumTargetFixupKinds;
  }

    const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
      const static MCFixupKindInfo Infos[CJG::NumTargetFixupKinds] = {
        // This table *must* be in the order that the fixup_* kinds are defined in
        // CJGFixupKinds.h.
        //
        // Name            Offset (bits) Size (bits)     Flags
        { "fixup_CJG_NONE",   0,  32,   0 },
        { "fixup_CJG_32",     0,  32,   0 },
      };

    if (Kind < FirstTargetFixupKind) {
      return MCAsmBackend::getFixupKindInfo(Kind);
    }

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }
  
  void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value, bool IsPCRel) const override;

  /// processFixupValue - Target hook to process the literal value of a fixup
  /// if necessary.
  void processFixupValue(const MCAssembler &Asm, const MCAsmLayout &Layout,
                         const MCFixup &Fixup, const MCFragment *DF,
                         const MCValue &Target, uint64_t &Value,
                         bool &IsResolved) override;

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

MCObjectWriter *CJGAsmBackend::createObjectWriter(raw_pwrite_stream &OS) const {
  return createCJGELFObjectWriter(OS, OSABI);
}

} // end anonymous namespace

static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext *Ctx = nullptr) {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
    default:
      llvm_unreachable("Unknown fixup kind!");
      case CJG::fixup_CJG_32:
        if (Value > 0xFFFF) {
          llvm_unreachable("Cannot process value larger than 16 bits");
        }
        return Value;
        break;
  } // switch

  return Value;
}

void CJGAsmBackend::processFixupValue(const MCAssembler &Asm,
                                      const MCAsmLayout &Layout,
                                      const MCFixup &Fixup,
                                      const MCFragment *DF,
                                      const MCValue &Target, uint64_t &Value,
                                      bool &IsResolved) {
  // We always have resolved fixups for now.
  IsResolved = true;
  // At this point we'll ignore the value returned by adjustFixupValue as
  // we are only checking if the fixup can be applied correctly.
  (void)adjustFixupValue(Fixup, Value, &Asm.getContext());
}

void CJGAsmBackend::applyFixup(const MCFixup &Fixup, char *Data,
                                 unsigned DataSize, uint64_t Value,
                                 bool IsPCRel) const {
  if (IsPCRel) {
    llvm_unreachable("PC Rel not currently implemented");
  }

  // The value of the fixup (e.g. The jump address in bytes)
  if (!Value) {
  Value = adjustFixupValue(Fixup, Value);
    return; // Doesn't change encoding.
  }

  // The offset of the instruction that needs the fixup
  // (e.g. a jump instruction that was encoded without a destination)
  unsigned Offset = Fixup.getOffset();

  // If the location is not in memory
  assert(Offset <= DataSize && "Invalid fixup offset!");

  // For now we are assuming fixups are only for jump/call addresses
  // FIXME: The arch is currently addressed by words so convert address to words
  assert(Value%4 == 0 && "The destination address is not aligned to a word");

  // Place the address into the instruction
  Data[Offset] = Value & 0xFF;
  Data[Offset+1] = uint8_t((Value >> 8) & 0xFF);

  return;
}

MCAsmBackend *llvm::createCJGAsmBackend(const Target &T,
                                          const MCRegisterInfo &MRI,
                                          const Triple &TT, StringRef CPU,
                                          const MCTargetOptions &Options) {
  const uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new CJGAsmBackend(T, TT.getTriple(), OSABI);
}
