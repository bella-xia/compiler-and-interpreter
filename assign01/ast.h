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
  AST_GTE
};

class ASTTreePrint : public TreePrint
{
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
