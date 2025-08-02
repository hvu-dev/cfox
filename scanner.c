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

Token scan_token() {
  scanner.start = scanner.current;

  if (is_at_eof())
    make_token(TOKEN_EOF);

  return make_error_token("Unexpected character");
}
