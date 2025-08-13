#include "scanner.h"

#include <stdbool.h>
#include <string.h>

typedef struct {
  const char *start;   // current token start position
  const char *current; // current char that being read of the current token
  int current_line;    // current line of code
} Scanner;
Scanner scanner;

void init_scanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.current_line = 1;
}

static bool is_at_eof() { return *scanner.current == '\0'; }

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static char match(char expected) {
  if (is_at_eof())
    return false;
  if (*scanner.current != expected)
    return false;

  scanner.current++;
  return true;
}

static Token make_token(TokenType type) {
  Token token;

  token.type = type;
  token.start = scanner.start;
  token.line = scanner.current_line;
  token.length = (int)(scanner.current - scanner.start);

  return token;
}

static Token make_error_token(const char *reason) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = reason;
  token.length = (int)strlen(reason);
  token.line = scanner.current_line;

  return token;
}

char peek() { return *scanner.current; }

char peek_next() { return is_at_eof() ? '\0' : *(scanner.current + 1); }

void skip_white_space() {
  while (true) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
      advance();
      break;
    case '\n':
      scanner.current_line++;
      advance();
      break;
    case '/':
      if (peek_next() == '/') {
        while (peek() != '\n' && !is_at_eof()) {
          advance();
        }
      } else
        return;
      break;
    default:
      return;
    }
  }
}

static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static TokenType check_keyword(int start, int length, const char *remaining,
                               TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, remaining, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

static TokenType get_identifier_type() {
  switch (scanner.start[0]) {
  case 'a':
    return check_keyword(1, 2, "nd", TOKEN_AND);
  case 'c':
    return check_keyword(1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return check_keyword(1, 3, "lse", TOKEN_ELSE);
  case 'f':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a':
        return check_keyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return check_keyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return check_keyword(2, 6, "nction", TOKEN_FUN);
      }
    }
    break;
  case 'i':
    return check_keyword(1, 1, "f", TOKEN_IF);
  case 'n':
    return check_keyword(1, 2, "il", TOKEN_NIL);
  case 'o':
    return check_keyword(1, 1, "r", TOKEN_OR);
  case 'p':
    return check_keyword(1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return check_keyword(1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return check_keyword(1, 4, "uper", TOKEN_SUPER);
  case 't':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h':
        return check_keyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return check_keyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  case 'v':
    return check_keyword(1, 2, "ar", TOKEN_VAR);
  case 'w':
    return check_keyword(1, 4, "hile", TOKEN_WHILE);
  default:
    return TOKEN_IDENTIFIER;
  }
}

static Token get_string() {
  while (peek() != '"' && !is_at_eof()) {
    if (peek() == '\n')
      scanner.current_line++;
    advance();
  }
  if (is_at_eof())
    return make_error_token("Unterminated string");

  // Consume the closing "
  advance();

  return make_token(TOKEN_STRING);
}

static Token get_number() {
  while (is_digit(peek()))
    advance();

  if (peek() == '.' && is_digit(peek_next())) {
    // Consume '.' character
    advance();
    // Look for further numbers after '.'
    while (is_digit(peek()))
      advance();
  }

  return make_token(TOKEN_NUMBER);
}

static Token get_identifier() {
  while (is_alpha(peek()) || is_digit(peek()))
    advance();
  return make_token(get_identifier_type());
}

Token scan_token() {
  skip_white_space();
  scanner.start = scanner.current;

  if (is_at_eof())
    return make_token(TOKEN_EOF);

  char c = advance();

  if (is_alpha(c))
    return get_identifier();
  if (is_digit(c))
    return get_number();

  switch (c) {
  case '(':
    return make_token(TOKEN_LEFT_PAREN);
  case ')':
    return make_token(TOKEN_RIGHT_PAREN);
  case '{':
    return make_token(TOKEN_LEFT_BRACE);
  case '}':
    return make_token(TOKEN_RIGHT_BRACE);
  case ';':
    return make_token(TOKEN_SEMICOLON);
  case ',':
    return make_token(TOKEN_COMMA);
  case '.':
    return make_token(TOKEN_DOT);
  case '+':
    return make_token(TOKEN_PLUS);
  case '-':
    return make_token(TOKEN_MINUS);
  case '*':
    return make_token(TOKEN_STAR);
  case '/':
    return make_token(TOKEN_SLASH);
  case '!':
    return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"':
    return get_string();
  }

  return make_error_token("Unexpected character");
}
