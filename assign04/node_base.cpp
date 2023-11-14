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

#include <cassert>
#include "node_base.h"

NodeBase::NodeBase() : m_type(nullptr), m_function_type(nullptr), m_symbol(nullptr), m_literal(LiteralValue()),
                       m_is_lvalue(true), m_has_literal(false), m_has_operand(false),
                       m_has_function_operand(false), m_has_function_type(false),
                       m_operand(Operand()), m_function_operand(Operand())
{
}

NodeBase::~NodeBase()
{
}

void NodeBase::set_symbol(Symbol *symbol)
{
    assert(!has_symbol());
    if (m_type != nullptr)
    {
        assert(symbol->get_type()->is_same(m_type.get()));
        m_type = nullptr;
    }
    m_symbol = symbol;
}

void NodeBase::set_type(const std::shared_ptr<Type> &type)
{
    assert(!has_symbol());
    assert(!m_type);
    m_type = type;
}

void NodeBase::set_function_type(const std::shared_ptr<Type> &type)
{
    m_function_type = type;
    m_has_function_type = true;
}

void NodeBase::set_is_lvalue(const bool &is_lvalue)
{
    this->m_is_lvalue = is_lvalue;
}

bool NodeBase::has_symbol() const
{
    return m_symbol != nullptr;
}

Symbol *NodeBase::get_symbol() const
{
    return m_symbol;
}

std::shared_ptr<Type> NodeBase::get_type() const
{
    // this shouldn't be called unless there is actually a type
    // associated with this node

    if (has_symbol())
        return m_symbol->get_type(); // Symbol will definitely have a valid Type
    else
    {
        assert(m_type); // make sure a Type object actually exists
        return m_type;
    }
}

std::shared_ptr<Type> NodeBase::get_function_type() const
{
    assert(m_has_function_type);
    return m_function_type;
}

bool NodeBase::check_is_lvalue() const
{
    return m_is_lvalue;
}

void NodeBase::set_literal_value(const LiteralValue &lit_val)
{
    m_literal = lit_val;
    m_has_literal = true;
}

LiteralValue NodeBase::get_literal_value()
{
    return m_literal;
}

void NodeBase::set_operand(const Operand &operand)
{
    m_operand = operand;
    m_has_operand = true;
}

Operand NodeBase::get_operand() const
{
    // either it has a symbol associated with it or it is
    assert(m_has_operand);
    return m_operand;
}

void NodeBase::set_function_operand(const Operand &operand)
{
    m_function_operand = operand;
    m_has_function_operand = true;
}

Operand NodeBase::get_function_operand() const
{
    assert(m_has_function_operand);
    return m_function_operand;
}

// TODO: implement member functions
