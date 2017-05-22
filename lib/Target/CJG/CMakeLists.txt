add_llvm_target(CJGCodeGen
  CJGTargetMachine.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
