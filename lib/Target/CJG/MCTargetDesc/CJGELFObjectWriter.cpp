//===-- CJGELFObjectWriter.cpp - CJG ELF Writer ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
  class CJGELFObjectWriter : public MCELFObjectTargetWriter {
  public:
    CJGELFObjectWriter(uint8_t OSABI);

    virtual ~CJGELFObjectWriter();

    unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                          const MCFixup &Fixup, bool IsPCRel) const override;
    };
}

CJGELFObjectWriter::CJGELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI,
                              /*ELF::EM_CJG*/ ELF::EM_CJG,
                              /*HasRelocationAddend*/ false) {}

CJGELFObjectWriter::~CJGELFObjectWriter() {}

unsigned CJGELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  llvm_unreachable("CJGELFObjectWriter: invalid fixup kind");
}

MCObjectWriter *llvm::createCJGELFObjectWriter(raw_pwrite_stream &OS,
                                                uint8_t OSABI) {
  MCELFObjectTargetWriter *MOTW = new CJGELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS, /*IsLittleEndian=*/true);
}
