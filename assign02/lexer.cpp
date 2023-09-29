#include <map>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include "cpputil.h"
#include "token.h"
#include "exceptions.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

Lexer::Lexer(FILE *in, const std::string &filename)
    : m_in(in), m_line(1), m_col(1), m_eof(false), m_filename(filename),
      single_char_map({{'+', TOK_PLUS},
                       {'-', TOK_MINUS},
                       {'*', TOK_TIMES},
                       {'/', TOK_DIVIDE},
                       {'(', TOK_LPAREN},
                       {')', TOK_RPAREN},
                       {'{', TOK_LBRACE},
                       {'}', TOK_RBRACE},
                       {',', TOK_COMMA},
                       {';', TOK_SEMICOLON}}),
      double_char_map({{'|', TOK_OR},
                       {'&', TOK_AND},
                       {'!', TOK_NOTEQUAL}}),
      single_or_double_char_map({{'<', {TOK_LESS, TOK_LESSOREQUAL}},
                                 {'>', {TOK_GREATER, TOK_GREATEROREQUAL}},
                                 {'=', {TOK_ASSIGN, TOK_EQUAL}}}),
      vars({{"var", TOK_VARIABLE},
            {"function", TOK_FUNC},
            {"if", TOK_IF},
            {"else", TOK_ELSE},
            {"while", TOK_WHILE}}),
      escape_char({'\"', '\n', '\r', '\t'})
{
}

Lexer::~Lexer()
{
  // delete any cached lookahead tokens
  for (auto i = m_lookahead.begin(); i != m_lookahead.end(); ++i)
  {
    delete *i;
  }
  fclose(m_in);
}

Node *Lexer::next()
{
  fill(1);
  if (m_lookahead.empty())
  {
    SyntaxError::raise(get_current_loc(), "Unexpected end of input");
  }
  Node *tok = m_lookahead.front();
  m_lookahead.pop_front();
  return tok;
}

Node *Lexer::peek(int how_many)
{
  // try to get as many lookahead tokens as required
  fill(how_many);

  // if there aren't enough lookahead tokens,
  // then the input ended before the token we want
  if (int(m_lookahead.size()) < how_many)
  {
    return nullptr;
  }

  // return the pointer to the Node representing the token
  return m_lookahead.at(how_many - 1);
}

Location Lexer::get_current_loc() const
{
  return Location(m_filename, m_line, m_col);
}

// Read the next character of input, returning -1 (and setting m_eof to true)
// if the end of input has been reached.
int Lexer::read()
{
  if (m_eof)
  {
    return -1;
  }
  int c = fgetc(m_in);
  if (c < 0)
  {
    m_eof = true;
  }
  else if (c == '\n')
  {
    m_col = 1;
    m_line++;
  }
  else
  {
    m_col++;
  }
  return c;
}

// "Unread" a character.  Useful for when reading a character indicates
// that the current token has ended and the next one has begun.
void Lexer::unread(int c)
{
  ungetc(c, m_in);
  if (c == '\n')
  {
    m_line--;
    m_col = 99;
  }
  else
  {
    m_col--;
  }
}

void Lexer::fill(int how_many)
{
  assert(how_many > 0);
  while (!m_eof && int(m_lookahead.size()) < how_many)
  {
    Node *tok = read_token();
    if (tok != nullptr)
    {
      m_lookahead.push_back(tok);
    }
  }
}

Node *Lexer::read_token()
{
  int c, line = -1, col = -1;

  // skip whitespace characters until a non-whitespace character is read
  for (;;)
  {
    line = m_line;
    col = m_col;
    c = read();
    if (c < 0 || !isspace(c))
    {
      break;
    }
  }

  if (c < 0)
  {
    // reached end of file
    return nullptr;
  }

  std::string lexeme;
  lexeme.push_back(char(c));

  if (isalpha(c))
  {
    Node *tok = read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalnum);
    // TODO: use set_tag to change the token kind if it's actually a keyword
    return tok;
  }
  else if (isdigit(c))
  {
    return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col, isdigit);
  }
  else
  {
    Node *target = nullptr;
    enum TokenKind kind, altkind;
    int desired_char = 0;
    std::pair<enum TokenKind, enum TokenKind> token_pair;
    switch (c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
    case ';':
    case ',':
    case '{':
    case '}':
      kind = Lexer::single_char_map[char(c)];
      return token_create(kind, lexeme, line, col);
    case '|':
    case '&':
    case '!':
      kind = Lexer::double_char_map[char(c)];
      desired_char = (c == '!') ? int('=') : c;
      target = read_dual_character_token(kind, lexeme, line, col, desired_char);
      if (target == nullptr)
      {
        break;
      }
      return target;
    case '<':
    case '>':
    case '=':
      token_pair = Lexer::single_or_double_char_map[char(c)];
      kind = token_pair.first;
      altkind = token_pair.second;
      return read_single_or_dual_character_token(kind, altkind, lexeme, line, col, char('='));
    // TODO: add cases for other kinds of tokens
    case '"':
      return read_string_literal("", line, col);
    default:
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
  }
  SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
}

// Helper function to create a Node object to represent a token.
Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme, int line, int col)
{
  Node *token = new Node(kind, lexeme);
  Location source_info(m_filename, line, col);
  token->set_loc(source_info);
  return token;
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
Node *Lexer::read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int))
{
  std::string lexeme(lexeme_start);
  for (;;)
  {
    int c = read();
    if (c >= 0 && pred(c))
    {
      // token has finished
      lexeme.push_back(char(c));
    }
    else
    {
      if (c >= 0)
      {
        unread(c);
      }
      if (vars.find(lexeme) != vars.end())
      {
        return token_create(vars[lexeme], lexeme, line, col);
      }
      return token_create(kind, lexeme, line, col);
    }
  }
}

// TODO: implement additional member functions if necessary
Node *Lexer::read_dual_character_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int desired_char)
{
  std::string lexeme(lexeme_start);
  int c = read();
  if (c >= 0 && c == desired_char)
  {
    lexeme.push_back(char(c));
    return token_create(kind, lexeme, line, col);
  }
  if (c >= 0)
  {
    unread(c);
  }
  return nullptr;
}

Node *Lexer::read_single_or_dual_character_token(enum TokenKind kind, enum TokenKind altkind, const std::string &lexeme_start, int line, int col, int desired_char)
{
  std::string lexeme(lexeme_start);
  int c = read();
  if (c >= 0 && c == desired_char)
  {

    lexeme.push_back(char(c));
    return token_create(altkind, lexeme, line, col);
  }
  if (c >= 0)
  {
    unread(c);
  }
  return token_create(kind, lexeme, line, col);
}

Node *Lexer::read_string_literal(const std::string &lexeme_start, int line, int col)
{
  std::string lexeme(lexeme_start);
  bool inner_quote = false;
  for (;;)
  {
    int c = read();
    if (c >= 0)
    {
      if (c == '"' && !inner_quote)
      {
        return token_create(TOK_STRING_LITERAL, lexeme, line, col);
      }
      if (inner_quote && (c != '"' && c != 'n' && c != 'r' && c != 't'))
      {
        SyntaxError::raise(get_current_loc(), "Unrecognized escape sequence '\\%c'", c);
      }
      // token has finished
      lexeme.push_back(char(c));
      inner_quote = ((int)c == 92) ? true : false;
    }
    else
    {
      SyntaxError::raise(get_current_loc(), "Unexpected end of input");
    }
  }
}