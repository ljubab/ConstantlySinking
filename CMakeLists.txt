add_llvm_library( LLVMConstantlySinking MODULE BUILDTREE_ONLY
  CombinedPass.cpp
  CodeSinking.cpp
  ConstPropagation.cpp
  ConstPropagationInstruction.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
