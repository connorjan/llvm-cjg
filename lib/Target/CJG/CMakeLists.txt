set(LLVM_TARGET_DEFINITIONS CJG.td)

tablegen(LLVM CJGGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CJGGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CJGGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM CJGGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM CJGGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM CJGGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CJGGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CJGGenDAGISel.inc -gen-dag-isel)

add_public_tablegen_target(CJGCommonTableGen)

add_llvm_target(CJGCodeGen
  CJGAsmPrinter.cpp
  CJGFrameLowering.cpp
  CJGInstrInfo.cpp
  CJGISelDAGToDAG.cpp
  CJGISelLowering.cpp
  CJGMCInstLower.cpp
  CJGRegisterInfo.cpp
  CJGSubtarget.cpp
  CJGTargetMachine.cpp
  CJGMachineFunctionInfo.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
