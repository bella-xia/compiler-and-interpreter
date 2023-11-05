// Copyright (c) 2021, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef NODE_BASE_H
#define NODE_BASE_H

#include <memory>
#include "type.h"
#include "symtab.h"
#include "instruction.h"
#include "literal_value.h"

// The Node class will inherit from this type, so you can use it
// to define any attributes and methods that Node objects should have
// (constant value, results of semantic analysis, code generation info,
// etc.)
class NodeBase
{
private:
  // TODO: fields (pointer to Type, pointer to Symbol, etc.)
  std::shared_ptr<Type> m_type, m_function_type;
  Symbol *m_symbol;
  LiteralValue m_literal;
  bool m_is_lvalue, m_has_literal, m_has_operand, m_has_function_operand, m_has_function_type;
  Operand m_operand, m_function_operand;

  // copy ctor and assignment operator not supported
  NodeBase(const NodeBase &);
  NodeBase &operator=(const NodeBase &);

public:
  NodeBase();
  virtual ~NodeBase();

  void set_symbol(Symbol *symbol);
  void set_type(const std::shared_ptr<Type> &type);
  void set_function_type(const std::shared_ptr<Type> &type);
  void set_is_lvalue(const bool &lvalue);
  bool has_symbol() const;
  bool check_is_lvalue() const;
  Symbol *get_symbol() const;
  std::shared_ptr<Type> get_type() const;
  std::shared_ptr<Type> get_function_type() const;

  void set_literal_value(const LiteralValue &lit_val);
  LiteralValue get_literal_value();

  void set_operand(const Operand &operand);
  Operand get_operand() const;
  bool has_operand() const { return m_has_operand; }

  void set_function_operand(const Operand &operand);
  Operand get_function_operand() const;
  bool has_function_operand() const { return m_has_function_operand; }

  // TODO: add member functions
};

#endif // NODE_BASE_H
