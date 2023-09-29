#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind
{
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  AST_VARDEF,
  AST_ASSIGN,
  AST_LOGICAL_OR,
  AST_LOGICAL_AND,
  AST_EQ,
  AST_NEQ,
  AST_LT,
  AST_LTE,
  AST_GT,
  AST_GTE,
  AST_IF,
  AST_ELSE,
  AST_FUNCTION,
  AST_WHILE,
  AST_PARAMETER_LIST,
  AST_STATEMENT_LIST,
  AST_FNCALL,
  AST_ARGUMENT_LIST,
  AST_STRING_LITERAL
};

class ASTTreePrint : public TreePrint
{
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
