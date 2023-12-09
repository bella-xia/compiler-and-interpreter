#include <cassert>
#include <iostream>
#include "highlevel_formatter.h"
#include "print_instruction_seq.h"
#include "print_highlevel_code.h"

PrintCode::PrintCode()
    : m_mode(NONE)
{
}

PrintCode::~PrintCode()
{
}

void PrintCode::collect_string_constant(const std::string &name, const std::string &strval)
{
  // FIXME: really, we should encode special characters in the string if there are any
  set_mode(RODATA);
  // std::string string_lit;
  // bool inner_quote = false;
  // for (int i = 0; i < strval.size(); ++i)
  // {
  //   char c = strval.at(i);
  //   if (c == '"' && !inner_quote)
  //   {
  //     continue;
  //   }
  //   else if (inner_quote)
  //   {
  //     switch (c)
  //     {
  //     case '"':
  //       string_lit.append(1, '\"');
  //       break;
  //     case 'n':
  //       string_lit.append(1, '\n');
  //       break;
  //     case 'r':
  //       string_lit.append(1, '\r');
  //       break;
  //     case 't':
  //       string_lit.append(1, '\t');
  //       break;
  //     default:
  //       break;
  //     }
  //     inner_quote = false;
  //     // don't do anything yet
  //   }
  //   else if ((int)c == 92)
  //   {
  //     inner_quote = true;
  //   }
  //   else
  //   {
  //     string_lit.append(1, c);
  //   }
  //   // token has finished

  //   inner_quote = ((int)c == 92) ? true : false;
  // }
  printf("\n%s: .string \"%s\"\n", name.c_str(), strval.c_str());
}

void PrintCode::collect_global_var(const std::string &name, const std::shared_ptr<Type> &type)
{
  set_mode(DATA);
  printf("\n\t.globl %s\n", name.c_str());
  printf("\t.align %u\n", type->get_alignment());
  printf("%s: .space %u\n", name.c_str(), type->get_storage_size());
}

void PrintCode::collect_function(const std::string &name, const std::shared_ptr<InstructionSequence> &iseq)
{
  set_mode(CODE);
  printf("\n\t.globl %s\n", name.c_str());
  printf("%s:\n", name.c_str());

  // print_instructions will be overridden by the concrete implementation
  // class to use the appropriate kind of formatter
  print_instructions(iseq);
}

void PrintCode::set_mode(Mode mode)
{
  if (mode == m_mode)
    return;

  switch (mode)
  {
  case RODATA:
    printf("\t.section .rodata\n");
    break;

  case DATA:
    printf("\t.section .data\n");
    break;

  case CODE:
    printf("\t.section .text\n");
    break;

  default:
    assert(false); // should not happen
  }

  m_mode = mode;
}
