#include "function.h"
#include "valrep.h"
#include "arr.h"
#include "str.h"
#include "environment.h"

ValRep::ValRep(ValRepKind kind)
    : m_kind(kind), m_refcount(0)
{
}

ValRep::~ValRep()
{
}

Function *ValRep::as_function()
{
  assert(m_kind == VALREP_FUNCTION);
  return static_cast<Function *>(this);
}

Arr *ValRep::as_array()
{
  assert(m_kind == VALREP_VECTOR);
  return static_cast<Arr *>(this);
}

Str *ValRep::as_str()
{
  assert(m_kind == VALREP_STRING);
  return static_cast<Str *>(this);
}

Environment *ValRep::as_env()
{
  assert(m_kind == VALREP_ENVIRONMENT);
  return static_cast<Environment *>(this);
}
