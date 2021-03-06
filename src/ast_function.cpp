#include "ast/ast_function.hpp"
#include "ast/ast_variable.hpp"
#include "ast/ast_list.hpp"

Function::Function(std::string *_decSpec, NodePtr _d, NodePtr _s)
    : returnType(*_decSpec), statements(_s), decl(_d) {
  delete _decSpec;
};
void Function::pyPrint(std::ostream &os) {
  os << "def ";
  if (decl)
    decl->pyPrint(os);
  for (int i = 0; i < globalVarNames.size(); i++) {
    os << "\tglobal " << globalVarNames[i] << "\n";
  }
  if (statements)
    statements->pyPrint(os);
  os << "\n";
}
void Function::mipsPrint() {
  std::string f_name = decl->getName();
  return_to = "__ret_" + f_name;
  //decl->registerVariables();
  LOG << "entered function: " << returnType << " " << f_name << "\n";
  printPreamble(f_name);
  decl->registerVariables();

  if (statements) {
    function_scope = 1;
    global_context->normalize_argument_chunks();
    statements->mipsPrint();
  }
  printEnd(f_name);
  global_context->new_frame();
}
void Function::printPreamble(std::string &f_name) {
  *global_context->get_stream() << ".text\n"
                                << ".align 2\n"
                                << ".globl " << f_name << "\n"
                                << f_name << ":\n"
                                << "\taddiu\t$sp,$sp,-8\n" //-8
                                << "\tsw\t$fp,4($sp)\n"
                                << "\tmove\t$fp,$sp\n";
}
void Function::printEnd(std::string &f_name) {
  *global_context->get_stream() << "\tmove $sp,$fp\n"
                                << "\tlw $fp,4($sp)\n"
                                << "\taddiu $sp,$sp,8\n" //8
                                << "\tjr $31\n"
                                << "\tnop\n";
}

FunctionCall::FunctionCall(NodePtr _exp, NodePtr _arg)
    : functionName(_exp), arguments(_arg){};

void FunctionCall::pyPrint(std::ostream &os) {
  if (dynamic_cast<Variable *>(functionName)) {
    functionName->pyPrint(os);
    os << "(";
    if (arguments)
      arguments->pyPrint(os);
    os << ")";
  } else {
    functionName->pyPrint(os);
  }
}

void FunctionCall::mipsPrint(){
  std::cout<<"entered mipsPrint for fcall\n";
  std::string f_name = functionName->getName();
  LOG << "entered function call for: " << f_name << "\n";
  TypePtr integer_type = std::make_shared<PrimitiveType>();
  auto res = global_context->register_chunk(makeUNQ("__fcall"), integer_type);
  int resReg = res->load();
  *global_context->get_stream() << "\tmove\t$"<<resReg<<",\t$31\n";
  res->store();
  std::vector<ChunkPtr> v;
  if(dynamic_cast<List*>(arguments)) (dynamic_cast<List*>(arguments))->passArguments(v);
  global_context->pass_args(v);
  *global_context->get_stream() << "\tjal\t" << f_name << "\n"
                                << "\tnop\n";
  resReg = res->load();
  *global_context->get_stream() << "\tmove\t$31,\t$"<<resReg<<"\n";
  *global_context->get_stream() << "\tmove\t$"<<resReg<<",\t$2\n";
  res->store();
}

void FunctionCall::mipsPrint(ChunkPtr res){
  std::string f_name = functionName->getName();
  LOG << "entered function call for: " << f_name << "\n";
  int resReg = res->load();
  *global_context->get_stream() << "\tmove\t$"<<resReg<<",\t$31\n";
  res->store();
  std::vector<ChunkPtr> v;
  if(dynamic_cast<List*>(arguments)) (dynamic_cast<List*>(arguments))->passArguments(v);
  global_context->pass_args(v);
  *global_context->get_stream() << "\tjal\t" << f_name << "\n"
                                << "\tnop\n";
  resReg = res->load();
  *global_context->get_stream() << "\tmove\t$31,\t$"<<resReg<<"\n";
  *global_context->get_stream() << "\tmove\t$"<<resReg<<",\t$2\n";

  res->store();
  *global_context->get_stream() << "\taddiu\t$sp,\t$sp,\t-4\n";

}
// 1. calculate size in bytes
// 2. sp = sp - size
// 3. save fp at top of the frame
// 4. overwrite framepointer with stack pointer
// 5. save all parameters from registers to stack

// to get size I need to store sizes of all the functions in context for ease of
// access - use the size from context when calculating size of each function
// function declared set to -1 to indicate that it has to be updated once
// function is calculated
//
