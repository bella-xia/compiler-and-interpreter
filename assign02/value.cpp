#include <iostream>
#include "cpputil.h"
#include "exceptions.h"
#include "valrep.h"
#include "function.h"
#include "arr.h"
#include "str.h"
#include "value.h"
#include "environment.h"

Value::Value(enum ValueKind kind) : m_kind(kind){};

Value::Value(int ival)
    : m_kind(VALUE_INT)
{
  m_atomic.ival = ival;
}

Value::Value(Function *fn)
    : m_kind(VALUE_FUNCTION), m_rep(fn)
{
  // by creating a function, we are addinf one reference
  m_rep->add_ref();
}

Value::Value(Arr *arr) : m_kind(VALUE_ARRAY), m_rep(arr)
{
  m_rep->add_ref();
}

Value::Value(Str *str) : m_kind(VALUE_STRING), m_rep(str)
{
  m_rep->add_ref();
}

Value::Value(Environment *env) : m_kind(VALUE_ENVIRONMENT), m_rep(env)
{
  m_rep->add_ref();
}

Value::Value(IntrinsicFn intrinsic_fn)
    : m_kind(VALUE_INTRINSIC_FN)
{
  m_atomic.intrinsic_fn = intrinsic_fn;
}

Value::Value(const Value &other)
{
  if (other.is_atomic())
  {
    m_kind = other.m_kind;
    m_atomic = other.m_atomic;
  }
  else
  {
    m_kind = other.m_kind;
    m_rep = other.m_rep;
    m_rep->add_ref();
  }
  // Just use the assignment operator to copy the other Value's data
}

Value::~Value()
{
  if (is_dynamic())
  {
    m_rep->remove_ref();
    if (m_rep->get_num_refs() == 0)
    {
      delete m_rep;
    }
  }
  // TODO: handle reference counting (detach from ValRep, if any)
}

Value &Value::operator=(const Value &rhs)
{
  if (is_dynamic())
  {
    // if it is dynamic, then remove the current reference
    m_rep->remove_ref();
    if (m_rep->get_num_refs() == 0)
    {
      delete m_rep;
    }
    if (rhs.is_dynamic())
    {
      m_rep = rhs.m_rep;
      m_rep->add_ref();
    }
    else
    {
      m_rep = nullptr;
      m_atomic = rhs.m_atomic;
    }
  }
  else
  {
    if (rhs.is_dynamic())
    {
      m_rep = rhs.m_rep;
      m_rep->add_ref();
    }
    else
    {
      m_atomic = rhs.m_atomic;
    }
  }
  m_kind = rhs.m_kind;
  return *this;
}

Function *Value::get_function() const
{
  assert(m_kind == VALUE_FUNCTION);
  return m_rep->as_function();
}

Arr *Value::get_array()
{
  assert(m_kind == VALUE_ARRAY);
  return m_rep->as_array();
}

Str *Value::get_str()
{
  assert(m_kind == VALUE_STRING);
  return m_rep->as_str();
}

Environment *Value::get_env()
{
  assert(m_kind == VALUE_ENVIRONMENT);
  return m_rep->as_env();
}

std::string Value::as_str() const
{
  switch (m_kind)
  {
  case VALUE_INT:
    return cpputil::format("%d", m_atomic.ival);
  case VALUE_FUNCTION:
    return cpputil::format("<function %s>", m_rep->as_function()->get_name().c_str());
  case VALUE_INTRINSIC_FN:
    return "<intrinsic function>";
  case VALUE_ENVIRONMENT:
    return "<environment>";
  case VALUE_STRING:
    return print_str();
  case VALUE_ARRAY:
    return print_arr();
  default:
    // this should not happen
    RuntimeError::raise("Unknown value type %d", int(m_kind));
  }
}
std::string Value::print_arr() const
{
  assert(m_kind == VALUE_ARRAY);
  std::string arr = "[";
  Arr *vec = m_rep->as_array();
  int len = vec->get_len().get_ival();
  for (int i = 0; i < len; ++i)
  {
    arr.append(vec->get_idx(i).as_str());
    if (i != len - 1)
    {
      arr.append(", ");
    }
    else
    {
      arr.append("]");
    }
  }
  return arr;
}
std::string Value::print_str() const
{
  assert(m_kind == VALUE_STRING);
  std::string str = "";
  std::string str_obj = m_rep->as_str()->get_str();
  int length = str_obj.size();
  bool escape = false;
  for (int i = 0; i < length; ++i)
  {
    if (escape)
    {
      switch (str_obj[i])
      {
      case 't':
        str.append(cpputil::format("%c", '\t'));
        break;
      case '"':
        str.append(cpputil::format("%c", '\"'));
        break;
      case 'r':
        str.append(cpputil::format("%c", '\r'));
        break;
      case 'n':
        str.append(cpputil::format("%c", '\n'));
        break;
      default:
        break;
      }
    }
    else if ((int)(str_obj[i]) != 92)
    {
      str.append(cpputil::format("%c", (int)(str_obj[i])));
    }
    escape = (int)(str_obj[i]) == 92;
  }
  return str;
}

// TODO: implementations of additional member functions
