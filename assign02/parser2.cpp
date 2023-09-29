#include <cassert>
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
////////////////////////////////////////////////////////////////////////

// This is the grammar (Unit is the start symbol):
//
// Unit -> Stmt
// Unit -> Stmt Unit
// Stmt -> A ;
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F -> ident
// F -> ( A )
// Stmt -> var ident ;
// A    -> ident = A
// A    -> L
// L    -> R || R
// L    -> R && R
// L    -> R
// R    -> E < E
// R    -> E <= E
// R    -> E > E
// R    -> E >= E
// R    -> E == E
// R    -> E != E
// R    -> E

Parser2::Parser2(Lexer *lexer_to_adopt)
    : m_lexer(lexer_to_adopt), m_next(nullptr)
{
}

Parser2::~Parser2()
{
  delete m_lexer;
}

Node *Parser2::parse()
{
  return parse_Unit();
}
Node *Parser2::parse_Unit()
{
  // note that this function produces a "flattened" representation
  // of the unit

  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;)
  {
    unit->append_kid(parse_TStmt());
    if (m_lexer->peek() == nullptr)
      break;
  }

  return unit.release();
}

Node *Parser2::parse_TStmt()
{
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr && next_tok->get_tag() == TOK_FUNC)
  {
    return parse_Func();
  }
  return parse_Stmt();
}

Node *Parser2::parse_Stmt()
{

  std::unique_ptr<Node> s(new Node(AST_STATEMENT));

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr)
  {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  // Stmt -> if (A) {SList} or Stmt -> if (A) {SList} else {SList}
  else if (next_tok->get_tag() == TOK_IF)
  {
    std::unique_ptr<Node> if_tok(expect(static_cast<enum TokenKind>(TOK_IF)));
    std::unique_ptr<Node> if_ast(new Node(AST_IF));
    if_ast->set_loc(if_tok->get_loc());

    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> a_ast(parse_A());
    expect_and_discard(TOK_RPAREN);

    expect_and_discard(TOK_LBRACE);
    std::unique_ptr<Node> s_list_ast(parse_SList());
    expect_and_discard(TOK_RBRACE);
    if_ast->append_kid(a_ast.release());
    if_ast->append_kid(s_list_ast.release());

    Node *next_next_tok = m_lexer->peek();
    if (next_next_tok != nullptr && next_next_tok->get_tag() == TOK_ELSE)
    {
      expect_and_discard(TOK_ELSE);

      expect_and_discard(TOK_LBRACE);
      std::unique_ptr<Node> s_list_ast_2(parse_SList());
      expect_and_discard(TOK_RBRACE);
      if_ast->append_kid(s_list_ast_2.release());
    }
    s->append_kid(if_ast.release());
    return s.release();
  }
  // Stmt -> while (A) {SList}
  else if (next_tok->get_tag() == TOK_WHILE)
  {
    std::unique_ptr<Node> while_tok(expect(static_cast<enum TokenKind>(TOK_WHILE)));
    std::unique_ptr<Node> while_ast(new Node(AST_WHILE));
    while_ast->set_loc(while_tok->get_loc());

    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> a_ast(parse_A());
    expect_and_discard(TOK_RPAREN);

    expect_and_discard(TOK_LBRACE);
    std::unique_ptr<Node> s_list_ast(parse_SList());
    expect_and_discard(TOK_RBRACE);
    while_ast->append_kid(a_ast.release());
    while_ast->append_kid(s_list_ast.release());
    s->append_kid(while_ast.release());

    return s.release();
  }
  // Stmt -> var ident ;
  else if (next_tok->get_tag() == TOK_VARIABLE)
  {
    // get var
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_VARIABLE)));
    std::unique_ptr<Node> ast(new Node(AST_VARDEF));
    ast->set_loc(tok->get_loc());

    // get ident
    std::unique_ptr<Node> next_tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    std::unique_ptr<Node> next_ast(new Node(AST_VARREF));
    next_ast->set_str(next_tok->get_str());
    next_ast->set_loc(next_tok->get_loc());

    // connect statement -> var -> ident
    ast->append_kid(next_ast.release());
    s->append_kid(ast.release());
  }
  else
  {
    // Stmt -> A ;
    std::unique_ptr<Node> ast(parse_A());
    s->append_kid(ast.release());
  }
  expect_and_discard(TOK_SEMICOLON);
  return s.release();
}

Node *Parser2::parse_Func()
{
  // Func →  function ident ( OptPList ) { SList }
  std::unique_ptr<Node> func_tok(expect(static_cast<enum TokenKind>(TOK_FUNC)));
  std::unique_ptr<Node> func_ast(new Node(AST_FUNCTION));
  func_ast->set_loc(func_tok->get_loc());
  std::unique_ptr<Node> ident_tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
  std::unique_ptr<Node> ident_ast(new Node(AST_VARREF));
  ident_ast->set_str(ident_tok->get_str());
  ident_ast->set_loc(ident_tok->get_loc());
  func_ast->append_kid(ident_ast.release());

  expect_and_discard(TOK_LPAREN);
  func_ast.reset(parse_OptPList(func_ast.release()));
  expect_and_discard(TOK_RPAREN);

  expect_and_discard(TOK_LBRACE);
  std::unique_ptr<Node> slist_ast(parse_SList());
  expect_and_discard(TOK_RBRACE);
  func_ast->append_kid(slist_ast.release());

  return func_ast.release();
}

Node *Parser2::parse_OptPList(Node *ast_)
{
  std::unique_ptr<Node> ast(ast_);
  Node *next_tok = m_lexer->peek(1);
  if (next_tok != nullptr && next_tok->get_tag() != TOK_RPAREN)
  {
    ast->append_kid(parse_PList());
  }
  return ast.release();
}

Node *Parser2::parse_PList()
{
  std::unique_ptr<Node> p_list(new Node(AST_PARAMETER_LIST));
  for (;;)
  {
    std::unique_ptr<Node> ident_tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    std::unique_ptr<Node> ident_ast(new Node(AST_VARREF));
    ident_ast->set_str(ident_tok->get_str());
    ident_ast->set_loc(ident_tok->get_loc());
    p_list->append_kid(ident_ast.release());
    if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() != TOK_COMMA)
      break;
    expect_and_discard(TOK_COMMA);
  }
  return p_list.release();
}

Node *Parser2::parse_SList()
{
  std::unique_ptr<Node> s_list(new Node(AST_STATEMENT_LIST));
  for (;;)
  {
    s_list->append_kid(parse_Stmt());
    if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() == TOK_RBRACE)
      break;
  }
  return s_list.release();
}

Node *Parser2::parse_A()
{
  Node *next_tok = m_lexer->peek(1);
  Node *next_next_tok = m_lexer->peek(2);
  if (next_tok != nullptr && next_next_tok != nullptr)
  {
    int tag = next_tok->get_tag();
    int tag_2 = next_next_tok->get_tag();
    if (tag == TOK_IDENTIFIER && tag_2 == TOK_ASSIGN)
    {
      // A  → ident = A
      // get identifier
      std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
      std::unique_ptr<Node> ast(new Node(AST_VARREF));
      ast->set_str(tok->get_str());
      ast->set_loc(tok->get_loc());

      // get = info
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(TOK_ASSIGN)));

      std::unique_ptr<Node> term_ast(parse_A());

      ast.reset(new Node(AST_ASSIGN, {ast.release(), term_ast.release()}));
      ast->set_loc(op->get_loc());

      return ast.release();
    }
  }
  // A  → L
  return parse_L();
}

Node *Parser2::parse_L()
{
  // get first term R
  std::unique_ptr<Node> first_term_ast(parse_R());

  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    // L    → R || R
    // L    → R && R
    if (next_tok_tag == TOK_OR || next_tok_tag == TOK_AND)
    {
      // get || or && terminal symbol
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      std::unique_ptr<Node> sec_term_ast(parse_R());
      first_term_ast.reset(new Node(next_tok_tag == TOK_OR ? AST_LOGICAL_OR : AST_LOGICAL_AND, {first_term_ast.release(), sec_term_ast.release()}));

      // copy source information from operator node
      first_term_ast->set_loc(op->get_loc());
    }
  }
  // L    → R
  return first_term_ast.release();
}

Node *Parser2::parse_R()
{
  // get first term E
  std::unique_ptr<Node> first_term_ast(parse_E());
  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    std::map<int, enum ASTKind> token_ast_map = {{int(TOK_LESS), AST_LT},
                                                 {int(TOK_LESSOREQUAL), AST_LTE},
                                                 {int(TOK_GREATER), AST_GT},
                                                 {int(TOK_GREATEROREQUAL), AST_GTE},
                                                 {int(TOK_EQUAL), AST_EQ},
                                                 {int(TOK_NOTEQUAL), AST_NEQ}};
    if (token_ast_map.find(next_tok_tag) != token_ast_map.end())
    {
      //       R    → E < E
      // R    → E <= E
      // R    → E > E
      // R    → E >= E
      // R    → E == E
      // R    → E != E
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      std::unique_ptr<Node> sec_term_ast(parse_E());
      first_term_ast.reset(new Node(token_ast_map[next_tok_tag], {first_term_ast.release(), sec_term_ast.release()}));

      // copy source information from operator node
      first_term_ast->set_loc(op->get_loc());
    }
  }
  // R    → E
  return first_term_ast.release();
}

Node *Parser2::parse_E()
{
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast_)
{
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)
    {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      Node *term_ast = parse_T();
      ast.reset(new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast.release(), term_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_EPrime(ast.release());
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_T()
{
  // T -> F T'

  // Parse primary expression
  Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_)
{
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)
    {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next primary expression, incorporate into current AST
      Node *primary_ast = parse_F();
      ast.reset(new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast.release(), primary_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_TPrime(ast.release());
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_F()
{
  // F -> ^ number
  // F -> ^ ident
  // F -> ^ string
  // F -> ^ ( A )
  // F →  ident ( OptArgList )

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr)
  {
    error_at_current_loc("Unexpected end of input looking for primary expression");
  }
  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_STRING_LITERAL)
  {
    // F -> ^ number
    // F -> ^ string
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_STRING_LITERAL;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  }
  else if (tag == TOK_IDENTIFIER)
  {
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    int ast_tag = AST_VARREF;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    Node *next_next_tok = m_lexer->peek();
    if (next_next_tok != nullptr && next_next_tok->get_tag() == TOK_LPAREN)
    {
      // F →  ident ( OptArgList )
      std::unique_ptr<Node> fncall(new Node(AST_FNCALL));
      fncall->append_kid(ast.release());
      expect_and_discard(TOK_LPAREN);
      fncall.reset(parse_OptArgList(fncall.release()));
      expect_and_discard(TOK_RPAREN);
      return fncall.release();
    }
    // F -> ^ ident
    return ast.release();
  }
  else if (tag == TOK_LPAREN)
  {
    // F -> ^ ( A )
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  }
  else
  {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
  }
}

Node *Parser2::parse_OptArgList(Node *ast_)
{
  std::unique_ptr<Node> ast(ast_);
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr && next_tok->get_tag() != TOK_RPAREN)
  {
    ast->append_kid(parse_ArgList());
  }
  return ast.release();
}

Node *Parser2::parse_ArgList()
{
  std::unique_ptr<Node> arg_list(new Node(AST_ARGUMENT_LIST));
  for (;;)
  {
    arg_list->append_kid(parse_L());
    if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() != TOK_COMMA)
      break;
    expect_and_discard(TOK_COMMA);
  }
  return arg_list.release();
}

Node *Parser2::expect(enum TokenKind tok_kind)
{
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (next_terminal->get_tag() != tok_kind)
  {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal.release();
}

void Parser2::expect_and_discard(enum TokenKind tok_kind)
{
  Node *tok = expect(tok_kind);
  delete tok;
}

void Parser2::error_at_current_loc(const std::string &msg)
{
  SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}
