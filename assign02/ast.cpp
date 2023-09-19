#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint()
{
}

ASTTreePrint::~ASTTreePrint()
{
}

std::string ASTTreePrint::node_tag_to_string(int tag) const
{
  switch (tag)
  {
  case AST_ADD:
    return "ADD";
  case AST_SUB:
    return "SUB";
  case AST_MULTIPLY:
    return "MULTIPLY";
  case AST_DIVIDE:
    return "DIVIDE";
  case AST_VARREF:
    return "VARREF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  case AST_UNIT:
    return "UNIT";
  case AST_STATEMENT:
    return "STATEMENT";
  case AST_VARDEF:
    return "VARDEF";
  case AST_ASSIGN:
    return "ASSIGN";
  case AST_LOGICAL_OR:
    return "LOGICAL_OR";
  case AST_LOGICAL_AND:
    return "LOGICAL_AND";
  case AST_EQ:
    return "EQ";
  case AST_NEQ:
    return "NEQ";
  case AST_LT:
    return "LT";
  case AST_LTE:
    return "LTE";
  case AST_GT:
    return "GT";
  case AST_GTE:
    return "GTE";
  case AST_IF:
    return "IF";
  case AST_ELSE:
    return "ELSE";
  case AST_FUNCTION:
    return "FUNCTION";
  case AST_WHILE:
    return "WHILE";
  case AST_PARAMETER_LIST:
    return "PARAMETER_LIST";
  case AST_STATEMENT_LIST:
    return "STATEMENT_LIST";
  case AST_FNCALL:
    return "FNCALL";
  case AST_ARGUMENT_LIST:
    return "ARGLIST";
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
