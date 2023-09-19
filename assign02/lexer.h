#ifndef LEXER_H
#define LEXER_H

#include <deque>
#include <cstdio>
#include "token.h"
#include "node.h"
#include <map>
#include <set>
#include <string>

class Lexer
{
private:
  FILE *m_in;
  int m_line,
      m_col;
  bool m_eof;
  std::deque<Node *> m_lookahead;
  std::string m_filename;

  std::map<char, enum TokenKind> single_char_map;
  std::map<char, enum TokenKind> double_char_map;
  std::map<char, std::pair<enum TokenKind, enum TokenKind>> single_or_double_char_map;
  std::map<std::string, enum TokenKind> vars;

public:
  Lexer(FILE *in, const std::string &filename);
  ~Lexer();

  // Consume the next token.
  // Throws SyntaxError if the input ends before
  // one token can be read.
  Node *next();

  // Look ahead and return a pointer to a future token
  // without consuming it. The how_far parameter indicates
  // how many tokens to look ahead (1 means return the
  // next token, 2 means the token after the next token,
  // etc.)
  Node *peek(int how_far = 1);

  Location get_current_loc() const;

private:
  int read();
  void unread(int c);
  void fill(int how_many);
  Node *read_token();
  Node *token_create(enum TokenKind kind, const std::string &lexeme, int line, int col);
  Node *read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int));
  Node *read_dual_character_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int desired_char);
  Node *read_single_or_dual_character_token(enum TokenKind kind, enum TokenKind altkind, const std::string &lexeme_start, int line, int col, int desired_char);
  // TODO: add additional member functions if necessary
};

#endif // LEXER_H
