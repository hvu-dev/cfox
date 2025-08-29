#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
  Token current;
  Token previous;
  bool had_error;
  bool panic_mode;
} Parser;
Parser parser;

// Order by precedence levels from lowest to highest
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();
typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// Prototypes for forward declarations
static void parse_grouping();
static void parse_unary();
static void parse_binary();
static void parse_number();
static void parse_expression();
static void parse_literal();
static ParseRule *get_rule(TokenType operator);
static void parse_precedence(Precedence precedence);
static void parse_string();

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] =
        {parse_grouping, NULL,
         PREC_NONE}, // [index] = {initial values} => assign element's index on
                     // initialisation undefined index will occupied with zero
                     // initialisation element
                     // since we are using "TokenType", an enum type, which will
                     // give us numbers from 0 to n (n is the number of
                     // defined TokenType)
                     // https://gcc.gnu.org/onlinedocs/gcc-3.0.4/gcc/Designated-Inits.html
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {parse_unary, parse_binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, parse_binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, parse_binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, parse_binary, PREC_FACTOR},
    [TOKEN_BANG] = {parse_unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_GREATER_EQUAL] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_LESS] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_LESS_EQUAL] = {NULL, parse_binary, PREC_EQUALITY},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = {parse_string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {parse_number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {parse_literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NULL] = {parse_literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {parse_literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static Chunk *compiling_chunk;

static Chunk *current_chunk() { return compiling_chunk; }

static void error_at(Token *token, const char *message) {
  if (parser.panic_mode)
    return;
  parser.panic_mode = true;
  fprintf(stderr, "[Line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);

  parser.had_error = true;
}

static void error(const char *message) { error_at(&parser.previous, message); }

static void error_at_current(const char *message) {
  error_at(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  while (true) {
    parser.current = scan_token();
    if (parser.current.type != TOKEN_ERROR)
      break;

    error_at_current(parser.current.start);
  }
}

static void consume(TokenType type, const char *message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  error_at_current(message);
}

static void emit_byte(uint8_t b) {
  write_byte_to_chunk(current_chunk(), b, parser.previous.line);
}
static void emit_bytes(uint8_t b1, uint8_t b2) {
  emit_byte(b1);
  emit_byte(b2);
}

static void emit_return() { emit_byte(OP_RETURN); }

static void stop_compile() {
  emit_return();
#ifdef DEBUG_PRINT_CODE
  if (!parser.had_error) {
    disassemble_chunk(current_chunk(), "code");
  }
#endif
}

static uint8_t make_constant(Value value) {
  int constant = add_constant(current_chunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk");
    return 0;
  }

  return (uint8_t)constant;
}

static void emit_constant(Value value) {
  emit_bytes(OP_CONSTANT, make_constant(value));
}

static void parse_precedence(Precedence precedence) {
  advance();
  ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule == NULL) {
    error("Expect expression");
    return;
  }
  prefix_rule();

  while (precedence <= get_rule(parser.current.type)->precedence) {
    advance();
    ParseFn infix_rule = get_rule(parser.previous.type)->infix;

    infix_rule();
  }
}

static void parse_number() {
  double value = strtod(parser.previous.start, NULL);
  emit_constant(NUMBER_VAL(value));
}

static void parse_expression() { parse_precedence(PREC_ASSIGNMENT); }
static ParseRule *get_rule(TokenType operator) {
        return &rules[operator];
}
static void parse_grouping() {
  parse_expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void parse_binary() {
  TokenType operator_type = parser.previous.type;
  ParseRule *rule = get_rule(operator_type);
  parse_precedence((Precedence)(rule->precedence + 1));

  switch (operator_type) {
  case TOKEN_PLUS:
    emit_byte(OP_ADD);
    break;
  case TOKEN_MINUS:
    emit_byte(OP_SUBSTRACT);
    break;
  case TOKEN_STAR:
    emit_byte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emit_byte(OP_DIVIDE);
    break;
  case TOKEN_BANG_EQUAL:
    emit_bytes(OP_EQUAL, OP_NOT);
    break;
  case TOKEN_EQUAL_EQUAL:
    emit_byte(OP_EQUAL);
    break;
  case TOKEN_GREATER:
    emit_byte(OP_GREATER);
    break;
  case TOKEN_GREATER_EQUAL:
    emit_bytes(OP_LESS, OP_NOT);
    break;
  case TOKEN_LESS:
    emit_byte(OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emit_bytes(OP_GREATER, OP_NOT);
    break;
  default:
    return;
  }
}

static void parse_unary() {
  TokenType operator_type = parser.previous.type;

  parse_precedence(PREC_UNARY);

  switch (operator_type) {
  case TOKEN_MINUS:
    emit_byte(OP_NEGATE);
    break;
  case TOKEN_BANG:
    emit_byte(OP_NOT);
    break;
  default:
    return;
  }
}

static void parse_literal() {
  switch (parser.previous.type) {
  case TOKEN_FALSE:
    emit_byte(OP_FALSE);
    break;
  case TOKEN_TRUE:
    emit_byte(OP_TRUE);
    break;
  case TOKEN_NULL:
    emit_byte(OP_NULL);
    break;
  default:
    return;
  }
}

static void parse_string() {
  emit_constant(OBJECT_VAL(
      copy_string(parser.previous.start + 1, parser.previous.length - 2)))
}

bool compile(const char *source, Chunk *chunk) {
  init_scanner(source);
  compiling_chunk = chunk;

  parser.had_error = false;
  parser.panic_mode = false;

  advance();
  parse_expression();
  consume(TOKEN_EOF, "Expected end of file");
  stop_compile();
  return !parser.had_error;
}
