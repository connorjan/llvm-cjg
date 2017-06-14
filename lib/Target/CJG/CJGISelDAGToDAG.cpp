//===-- CJGISelDAGToDAG.cpp - A dag to dag inst selector for CJG ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the CJG target.
//
//===----------------------------------------------------------------------===//

#include "CJG.h"
#include "CJGTargetMachine.h"
#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Function.h" // To access function attributes.
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "cjg-isel"

// CJG-specific code to select CJG machine instructions for
// SelectionDAG operations.
namespace {
class CJGDAGToDAGISel : public SelectionDAGISel {

public:
  explicit CJGDAGToDAGISel(CJGTargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  StringRef getPassName() const override {
    return "CJG DAG->DAG Pattern Instruction Selection";
  }

  void Select(SDNode *Node) override;

  bool SelectAddr(SDValue Addr, SDValue &BaseAddr, SDValue &Offset);

// Include the pieces autogenerated from the target description.
#include "CJGGenDAGISel.inc"
};
} // end anonymous namespace

void CJGDAGToDAGISel::Select(SDNode *Node) {
  // Dump information about the Node being selected.
  DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we have already selected
  if (Node->isMachineOpcode()) {
    DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  // Select the default instruction.
  SelectCode(Node);
}

/// SelectAddr - returns true if it is able pattern match an addressing mode.
/// It returns the operands which make up the maximal addressing mode it can
/// match by reference.
bool CJGDAGToDAGISel::SelectAddr(SDValue Addr, SDValue &BaseAddr, SDValue &Offset) {
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    EVT PtrVT = getTargetLowering()->getPointerTy(CurDAG->getDataLayout());
    BaseAddr = CurDAG->getTargetFrameIndex(FIN->getIndex(), PtrVT);
    Offset = CurDAG->getTargetConstant(0, Addr, MVT::i32);
    return true;
  }
  
  if (Addr.getOpcode() == ISD::TargetExternalSymbol ||
      Addr.getOpcode() == ISD::TargetGlobalAddress ||
      Addr.getOpcode() == ISD::TargetGlobalTLSAddress) {
    return false; // direct calls.
  }

  BaseAddr = Addr;
  Offset = CurDAG->getTargetConstant(0, Addr, MVT::i32);
  return true;
}

/// createCJGISelDag - This pass converts a legalized DAG into a
/// CJG-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createCJGISelDag(CJGTargetMachine &TM,
                                     CodeGenOpt::Level OptLevel) {
  return new CJGDAGToDAGISel(TM, OptLevel);
}
