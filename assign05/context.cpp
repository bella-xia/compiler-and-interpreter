// Copyright (c) 2021-2022, David H. Hovemeyer <david.hovemeyer@gmail.com>
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

#include <set>
#include <memory>
#include <algorithm>
#include <iterator>
#include <cassert>
#include "exceptions.h"
#include "node.h"
#include "ast.h"
#include "parse.tab.h"
#include "lex.yy.h"
#include "parser_state.h"
#include "semantic_analysis.h"
#include "symtab.h"
#include "peephole_ll.h"
#include "highlevel_codegen.h"
#include "local_storage_allocation.h"
#include "lowlevel_codegen.h"
#include "context.h"

Context::Context()
    : m_ast(nullptr), m_symtabs(std::vector<SymbolTable *>()),
      m_sema(SemanticAnalysis(m_symtabs))
{
}

Context::~Context()
{
  delete m_ast;
  for (SymbolTable *ele : m_symtabs)
  {
    delete ele;
  }
}

struct CloseFile
{
  void operator()(FILE *in)
  {
    if (in != nullptr)
    {
      fclose(in);
    }
  }
};

namespace
{

  template <typename Fn>
  void process_source_file(const std::string &filename, Fn fn)
  {
    // open the input source file
    std::unique_ptr<FILE, CloseFile> in(fopen(filename.c_str(), "r"));
    if (!in)
    {
      RuntimeError::raise("Couldn't open '%s'", filename.c_str());
    }

    // create an initialize ParserState; note that its destructor
    // will take responsibility for cleaning up the lexer state
    std::unique_ptr<ParserState> pp(new ParserState);
    pp->cur_loc = Location(filename, 1, 1);

    // prepare the lexer
    yylex_init(&pp->scan_info);
    yyset_in(in.get(), pp->scan_info);

    // make the ParserState available from the lexer state
    yyset_extra(pp.get(), pp->scan_info);

    // use the ParserState to either scan tokens or parse the input
    // to build an AST
    fn(pp.get());
  }

}

void Context::scan_tokens(const std::string &filename, std::vector<Node *> &tokens)
{
  auto callback = [&](ParserState *pp)
  {
    YYSTYPE yylval;

    // the lexer will store pointers to all of the allocated
    // token objects in the ParserState, so all we need to do
    // is call yylex() until we reach the end of the input
    while (yylex(&yylval, pp->scan_info) != 0)
      ;

    std::copy(pp->tokens.begin(), pp->tokens.end(), std::back_inserter(tokens));
  };

  process_source_file(filename, callback);
}

void Context::parse(const std::string &filename)
{
  auto callback = [&](ParserState *pp)
  {
    // parse the input source code
    yyparse(pp);

    // free memory allocated by flex
    yylex_destroy(pp->scan_info);

    m_ast = pp->parse_tree;

    // delete any Nodes that were created by the lexer,
    // but weren't incorporated into the parse tree
    std::set<Node *> tree_nodes;
    m_ast->preorder([&tree_nodes](Node *n)
                    { tree_nodes.insert(n); });
    for (auto i = pp->tokens.begin(); i != pp->tokens.end(); ++i)
    {
      if (tree_nodes.count(*i) == 0)
      {
        delete *i;
      }
    }
  };

  process_source_file(filename, callback);
}

void Context::analyze()
{
  assert(m_ast != nullptr);
  m_sema.visit(m_ast);
}

void Context::highlevel_codegen(ModuleCollector *module_collector, bool optimize)
{
  // Assign
  //   - vreg numbers to parameters
  //   - local storage offsets to local variables requiring storage in
  //     memory
  //
  // This will also determine the total local storage requirements
  // for each function.
  //
  // Any local variable not assigned storage in memory will be allocated
  // a vreg as its storage.
  LocalStorageAllocation local_storage_alloc;
  local_storage_alloc.visit(m_ast);

  // TODO: find all of the string constants in the AST
  //       and call the ModuleCollector's collect_string_constant
  //       member function for each one
  int idx = 0;
  std::map<std::string, Node *> *string_literal_map = m_sema.get_string_literal_map();
  for (auto i = string_literal_map->cbegin(); i != string_literal_map->cend();
       ++i)
  {
    std::string str_lit = i->first;
    std::string str_name = "_str" + std::to_string(idx);
    Node *str_lit_node = i->second;
    module_collector->collect_string_constant(str_name, str_lit);
    str_lit_node->set_operand(Operand(Operand::IMM_LABEL, str_name));
    idx++;
  }

  // collect all of the global variables
  SymbolTable *globals = m_sema.get_global_symtab();
  for (auto i = globals->cbegin(); i != globals->cend(); ++i)
  {
    Symbol *sym = *i;
    if (sym->get_kind() == SymbolKind::VARIABLE)
      module_collector->collect_global_var(sym->get_name(), sym->get_type());
  }

  // generating high-level code for each function, and then send the
  // generated high-level InstructionSequence to the ModuleCollector
  int next_label_num = 0;
  for (auto i = m_ast->cbegin(); i != m_ast->cend(); ++i)
  {
    Node *child = *i;
    if (child->get_tag() == AST_FUNCTION_DEFINITION)
    {
      HighLevelCodegen hl_codegen(next_label_num, optimize);
      hl_codegen.visit(child);
      std::string fn_name = child->get_kid(1)->get_str();
      std::shared_ptr<InstructionSequence> hl_iseq = hl_codegen.get_hl_iseq();
      std::shared_ptr<InstructionSequence> cur_hl_iseq(hl_iseq);
      if (optimize)
      {
        // High-level optimization

        // Create a control-flow graph representation of the high-level code

        // Do local optimizations
        bool modified = false;
        int prev_length = 0;
        int idx = 0;
        int stack_idx = child->get_symbol()->get_optimized_stack_size();
        do
        {
          HighLevelControlFlowGraphBuilder hl_cfg_builder(cur_hl_iseq);
          std::shared_ptr<ControlFlowGraph> cfg = hl_cfg_builder.build();
          LocalOptimizationHighLevel hl_opts(cfg);
          prev_length = cur_hl_iseq->get_length();
          cfg = hl_opts.transform_cfg();
          modified = hl_opts.is_modified();
          idx++;
          // convert the transformed high-level CFG back to an Instruction sequence
          // module_collector->collect_function(fn_name, cur_hl_iseq);
          cur_hl_iseq = cfg->create_instruction_sequence();

        } while ((modified || prev_length > (int)cur_hl_iseq->get_length()) && idx < 10);

        HighLevelControlFlowGraphBuilder hl_cfg_builder(cur_hl_iseq);
        std::shared_ptr<ControlFlowGraph> cfg = hl_cfg_builder.build();
        LocalMregAssignmentHighLevel hl_opts(cfg);
        hl_opts.set_stack_idx(stack_idx);
        cfg = hl_opts.transform_cfg();
        // convert the transformed high-level CFG back to an Instruction sequence
        // module_collector->collect_function(fn_name, cur_hl_iseq);
        stack_idx = hl_opts.get_max_stack_idx();
        child->get_symbol()->set_total_optimized_stack_size(stack_idx);
        cur_hl_iseq = cfg->create_instruction_sequence();
      }

      // store a pointer to the function definition AST in the
      // high-level InstructionSequence: this is useful in case information
      // about the function definition is needed by the low-level
      // code generator
      cur_hl_iseq->set_funcdef_ast(child);

      module_collector->collect_function(fn_name, cur_hl_iseq);

      // make sure local label numbers are not reused between functions
      next_label_num = hl_codegen.get_next_label_num();
    }
  }
}

namespace
{

  // ModuleCollector implementation which generates low-level code
  // from generated high-level code, and then forwards the generated
  // low-level code to a delegate.
  class LowLevelCodeGenModuleCollector : public ModuleCollector
  {
  private:
    ModuleCollector *m_delegate;
    bool m_optimize;

  public:
    LowLevelCodeGenModuleCollector(ModuleCollector *delegate, bool optimize);
    virtual ~LowLevelCodeGenModuleCollector();

    virtual void collect_string_constant(const std::string &name, const std::string &strval);
    virtual void collect_global_var(const std::string &name, const std::shared_ptr<Type> &type);
    virtual void collect_function(const std::string &name, const std::shared_ptr<InstructionSequence> &iseq);
  };

  LowLevelCodeGenModuleCollector::LowLevelCodeGenModuleCollector(ModuleCollector *delegate, bool optimize)
      : m_delegate(delegate), m_optimize(optimize)
  {
  }

  LowLevelCodeGenModuleCollector::~LowLevelCodeGenModuleCollector()
  {
  }

  void LowLevelCodeGenModuleCollector::collect_string_constant(const std::string &name, const std::string &strval)
  {
    m_delegate->collect_string_constant(name, strval);
  }

  void LowLevelCodeGenModuleCollector::collect_global_var(const std::string &name, const std::shared_ptr<Type> &type)
  {
    m_delegate->collect_global_var(name, type);
  }

  void LowLevelCodeGenModuleCollector::collect_function(const std::string &name, const std::shared_ptr<InstructionSequence> &iseq)
  {
    LowLevelCodeGen ll_codegen(m_optimize);

    // translate high-level code to low-level code
    std::shared_ptr<InstructionSequence> ll_iseq = ll_codegen.generate(iseq);

    LowLevelControlFlowGraphBuilder ll_cfg_builder(ll_iseq);
    std::shared_ptr<ControlFlowGraph> ll_cfg = ll_cfg_builder.build();

    if (m_optimize)
    {
      bool done = false;
      while (!done)
      {
        PeepholeLowLevel peephole_ll(ll_cfg);
        ll_cfg = peephole_ll.transform_cfg();
        // ll_iseq = ll_cfg->create_instruction_sequence();
        // m_delegate->collect_function(name, ll_iseq);
        if (peephole_ll.get_num_matched() == 0)
          done = true;
      }
      ll_iseq = ll_cfg->create_instruction_sequence();
    }
    // send the low-level code on to the delegate (i.e., print the code)
    m_delegate->collect_function(name, ll_iseq);
  }
}

void Context::lowlevel_codegen(ModuleCollector *module_collector, bool optimize)
{
  LowLevelCodeGenModuleCollector ll_codegen_module_collector(module_collector, optimize);
  highlevel_codegen(&ll_codegen_module_collector, optimize);
}
