#include "compiler.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
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
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_MAX_COUNT];
    int local_count;
    int scope_depth;
} Compiler;
Compiler *current_compiler = NULL;

// Prototypes for forward declarations
static void parse_grouping(bool can_assign);
static void parse_unary(bool can_assign);
static void parse_binary(bool can_assign);
static void parse_number(bool can_assign);
static void parse_literal(bool can_assign);
static void parse_expression();
static ParseRule *get_rule(TokenType operator);
static void parse_precedence(Precedence precedence);
static void parse_string(bool can_assign);
static void parse_statement();
static void parse_declaration();
static void parse_print_statement();
static void parse_expression_statement();
static void get_variable(bool can_assign);

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] =
        {parse_grouping, NULL, PREC_NONE
        },  // [index] = {initial values} => assign element's index on
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
    [TOKEN_IDENTIFIER] = {get_variable, NULL, PREC_NONE},
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

static void init_compiler(Compiler *compiler) {
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current_compiler = compiler;
}

static Chunk *compiling_chunk;

static Chunk *current_chunk() { return compiling_chunk; }

static void error_at(Token *token, const char *message) {
    if (parser.panic_mode) return;
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
        if (parser.current.type != TOKEN_ERROR) break;

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

static bool check(TokenType type) { return parser.current.type == type; }

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
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

static void parse_expression() { parse_precedence(PREC_ASSIGNMENT); }

static void add_local(Token name) {
    if (current_compiler->local_count == UINT8_MAX_COUNT) {
        error("Too many local variables in local scope");
        return;
    }

    Local *local = &current_compiler->locals[current_compiler->local_count++];
    local->name = name;
    local->depth = -1;
}

static void mark_initialized_status() {
    current_compiler->locals[current_compiler->local_count - 1].depth =
        current_compiler->scope_depth;
}

static bool has_identifier_existed(Token *lhs, Token *rhs) {
    if (lhs->length != rhs->length) return false;

    return memcmp(lhs->start, rhs->start, lhs->length) == 0;
}

static void declare_variable() {
    Token *name = &parser.previous;

    for (int i = current_compiler->local_count - 1; i >= 0; i--) {
        Local *local = &current_compiler->locals[i];
        if (local->depth != -1 && local->depth < current_compiler->scope_depth)
            break;

        if (has_identifier_existed(name, &local->name)) {
            error("Variable with this name has already existed in this scope");
        }
    }

    add_local(*name);
}

static uint8_t make_constant(Value value) {
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static uint8_t get_identifier_constant(Token *token) {
    return make_constant(OBJECT_VAL(copy_string(token->start, token->length)));
}

static uint8_t parse_variable(const char *error_message) {
    consume(TOKEN_IDENTIFIER, error_message);

    if (current_compiler->scope_depth > 0) {
        declare_variable();
        return 0;
    }

    return get_identifier_constant(&parser.previous);
}

static int resolve_local(Compiler *compiler, Token *name) {
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];

        if (has_identifier_existed(&local->name, name)) {
            if (local->depth == -1)
                error("Can not assign a variable to its own initializer");
            return i;
        }
    }

    return -1;
}

static void define_variable(uint8_t global) {
    if (current_compiler->scope_depth > 0) {
        mark_initialized_status();
        return;
    }

    emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void get_named_variable(Token name, bool can_assign) {
    uint8_t get_op, set_op;
    int arg = resolve_local(current_compiler, &name);
    if (arg == -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else {
        arg = get_identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        parse_expression();
        emit_bytes(get_op, (uint8_t)arg);
    } else {
        emit_bytes(set_op, (uint8_t)arg);
    }
}

static void get_variable(bool can_assign) {
    get_named_variable(parser.previous, can_assign);
}

static void parse_var_declaration() {
    uint8_t global = parse_variable("Expect variable name");

    if (match(TOKEN_EQUAL)) {
        parse_expression();
    } else {
        emit_byte(OP_NULL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    define_variable(global);
}

static void synchronize() {
    parser.panic_mode = false;
    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:;  // Do nothing
        }

        advance();
    }
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

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;

        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        error("Invalid assignment");
    }
}

static void parse_number(bool can_assign) {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void parse_expression_statement() {
    parse_expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emit_byte(OP_POP);
}
static ParseRule *get_rule(TokenType operator) {
        return &rules[operator];
}
static void parse_grouping(bool can_assign) {
    parse_expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void parse_binary(bool can_assign) {
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

static void parse_unary(bool can_assign) {
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

static void parse_literal(bool can_assign) {
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

static void parse_string(bool can_assign) {
    emit_constant(OBJECT_VAL(
        copy_string(parser.previous.start + 1, parser.previous.length - 2)
    ));
}

static void parse_block() {
    while (!(check(TOKEN_RIGHT_BRACE) || check(TOKEN_EOF))) {
        parse_declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect closing '}' for a block");
}

static void parse_print_statement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after print keyword");
    parse_expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after print keyword");
    consume(TOKEN_SEMICOLON, "Expect ';' after an expression");
    emit_byte(OP_PRINT);
}

static void make_new_scope() { current_compiler->scope_depth++; }

static void end_scope() {
    current_compiler->scope_depth--;
    while (current_compiler->local_count > 0 &&
           current_compiler->locals[current_compiler->local_count - 1].depth >
               current_compiler->scope_depth) {
        emit_byte(OP_POP);
        --current_compiler->local_count;
    }
}

static void parse_statement() {
    if (match(TOKEN_PRINT)) {
        parse_print_statement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        make_new_scope();
        parse_block();
        end_scope();
    } else {
        parse_expression_statement();
    }
}

static void parse_declaration() {
    if (match(TOKEN_VAR)) {
        parse_var_declaration();
    } else {
        parse_statement();
    }

    if (parser.panic_mode) synchronize();
}

bool compile(const char *source, Chunk *chunk) {
    init_scanner(source);
    Compiler compiler;
    init_compiler(&compiler);

    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    while (!match(TOKEN_EOF)) {
        parse_declaration();
    }
    stop_compile();
    return !parser.had_error;
}
