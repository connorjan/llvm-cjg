//===-- CJGISelLowering.cpp - CJG DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that CJG uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "CJGISelLowering.h"
#include "CJG.h"
#include "CJGRegisterInfo.h"
#include "CJGSubtarget.h"
#include "CJGTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "cjg-lower"

const char *CJGTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((CJGISD::NodeType)Opcode) {
    case CJGISD::FIRST_NUMBER:            break;
    case CJGISD::CMP:                     return "CJGISD::CMP";
    case CJGISD::RET_FLAG:                return "CJGISD::RET_FLAG";
  }
  return nullptr;
}

CJGTargetLowering::CJGTargetLowering(const CJGTargetMachine &TM,
                                     const CJGSubtarget &STI)
    : TargetLowering(TM) {

  // Set up the register classes.
  addRegisterClass(MVT::i32, &CJG::GPRegsRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(CJG::SP);

  // TODO: add all necessary setOperationAction calls

  setBooleanContents(ZeroOrOneBooleanContent);

  // // Function alignments (log2)
  // setMinFunctionAlignment(3);
  // setPrefFunctionAlignment(3);

  // // inline memcpy() for kernel to see explicit copy
  // MaxStoresPerMemset = MaxStoresPerMemsetOptSize = 128;
  // MaxStoresPerMemcpy = MaxStoresPerMemcpyOptSize = 128;
  // MaxStoresPerMemmove = MaxStoresPerMemmoveOptSize = 128;
}

SDValue CJGTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("unimplemented operand");
  }
}

// Calling Convention Implementation
#include "CJGGenCallingConv.inc"

// Transform physical registers into virtual registers
SDValue CJGTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  if (IsVarArg) {
    llvm_unreachable("VarArg not supported");
  }

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_CJG);

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) {
      case MVT::i32: {
        const unsigned VReg =
            RegInfo.createVirtualRegister(&CJG::GPRegsRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        InVals.push_back(ArgIn);
        break;
      }
      default:
        DEBUG(dbgs() << "LowerFormalArguments Unhandled argument type: "
                     << RegVT.getEVTString() << "\n");
        llvm_unreachable("unhandled argument type");
      }
    } else {
      llvm_unreachable("Defined with too many args");
    }
  }
  return Chain;
}

SDValue
CJGTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  if (IsVarArg) {
    report_fatal_error("VarArg not supported");
  }

  // Stores the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // Info about the registers and stack slot
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_CJG);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(CJGISD::RET_FLAG, DL, MVT::Other, RetOps);
}
