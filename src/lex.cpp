#include "arena.hpp"
#include "string_builder.hpp"
#include "strings.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>
#include <print>

// TODO: Should probably be in order of precedence
enum TokenKind {
  // Seperators
  TK_LEFT_PAREN,
  TK_RIGHT_PAREN,
  TK_LEFT_BRACE,
  TK_RIGHT_BRACE,
  TK_COMMA,
  TK_DOT,
  TK_SEMICOLON,

  // Operators
  TK_MINUS,
  TK_PLUS,
  TK_SLASH,
  TK_STAR,
  TK_BANG,
  TK_BANG_EQUAL,
  TK_EQUAL,
  TK_EQUAL_EQUAL,
  TK_GREATER,
  TK_GREATER_EQUAL,
  TK_LESS,
  TK_LESS_EQUAL,

  // Identifiers
  TK_IDENTIFIER,
  TK_STRING,
  TK_NUMBER,

  // Keywords
  TK_KW_INT,
  TK_KW_VOID,
  TK_KW_RETURN,
  // TK_AND,
  // TK_CLASS,
  // TK_ELSE,
  // TK_FALSE,
  // TK_FUN,
  // TK_FOR,
  // TK_IF,
  // TK_NIL,
  // TK_OR,
  // TK_RETURN,
  // TK_WHILE,

  TK_EOF,
  TK_ERROR,
};

constexpr String8 token_kind_to_str8(TokenKind tk) {
  switch (tk) {
  case TK_LEFT_PAREN:
    return str8_lit("TK_LEFT_PAREN");
  case TK_RIGHT_PAREN:
    return str8_lit("TK_RIGHT_PAREN");
  case TK_LEFT_BRACE:
    return str8_lit("TK_LEFT_BRACE");
  case TK_RIGHT_BRACE:
    return str8_lit("TK_RIGHT_BRACE");
  case TK_COMMA:
    return str8_lit("TK_COMMA");
  case TK_DOT:
    return str8_lit("TK_DOT");
  case TK_SEMICOLON:
    return str8_lit("TK_SEMICOLON");
  case TK_MINUS:
  case TK_PLUS:
  case TK_SLASH:
  case TK_STAR:
  case TK_BANG:
  case TK_BANG_EQUAL:
  case TK_EQUAL:
  case TK_EQUAL_EQUAL:
  case TK_GREATER:
  case TK_GREATER_EQUAL:
  case TK_LESS:
  case TK_LESS_EQUAL:
  case TK_IDENTIFIER:
    return str8_lit("TK_IDENTIFIER");
  case TK_STRING:
  case TK_NUMBER:
  case TK_KW_INT:
    return str8_lit("TK_KW_INT");
  case TK_KW_VOID:
    return str8_lit("TK_KW_VOID");
  case TK_KW_RETURN:
    return str8_lit("TK_KW_RETURN");
  case TK_EOF:
    return str8_lit("TK_EOF");
  case TK_ERROR:
  default:
    assert(false && "unreachable");
  }
}

enum LexError {
  LEX_OK,
  LEX_ERROR_IO,
  LEX_ERROR_UNTERMINATED_STRING,
  LEX_ERROR_INVALID_NUMBER,
  LEX_ERROR_INVALID_CHARACTER,
  LEX_ERROR_UNEXPECTED_EOF,
};

struct Token {
  TokenKind kind;
  String8 source;
  S64 num_value;
  // Maybe helpful?
  //  U64 line
};

struct TokenResult {
  Token token;
  LexError maybe_error;
  String8 error_msg;
};

struct LexResult {
  TokenResult *tokens;
  Size token_count;
};

struct Lexer {
  const String8 input;
  U64 current;
  U64 line;
};

struct KeywordPair {
  String8 str;
  TokenKind kind;
};

static KeywordPair keywords[] = {
    {str8_lit("int"), TK_KW_INT},
    {str8_lit("void"), TK_KW_VOID},
    {str8_lit("return"), TK_KW_RETURN},
    // {str8_lit("main"), TK_CLASS},
    // {str8_lit("else"), TK_ELSE},   {str8_lit("false"), TK_FALSE},
    // {str8_lit("fun"), TK_FUN},     {str8_lit("for"), TK_FOR},
    // {str8_lit("if"), TK_IF},       {str8_lit("nil"), TK_NIL},
    // {str8_lit("or"), TK_OR},       {str8_lit("return"), TK_RETURN},
    // {str8_lit("while"), TK_WHILE},
};

constexpr Size keywords_table_size = sizeof(keywords) / sizeof(keywords[0]);

U8 current_char(Lexer &lexer) { return lexer.input.str[lexer.current]; }

void advance(Lexer &lexer) {
  if (lexer.current < lexer.input.size) {
    if (lexer.input.str[lexer.current] == '\n') {
      lexer.line++;
    }
    lexer.current++;
  }
}

// Check whether it's a keyword - otherwise it's an identifier
TokenKind identifier_type(String8 text) {
  for (Size i = 0; i < keywords_table_size; ++i) {
    if (str8_match(text, keywords[i].str)) {
      return keywords[i].kind;
    }
  }
  return TK_IDENTIFIER;
}

TokenResult lex_identifier(Lexer &lexer) {
  U64 start = lexer.current;
  U64 end = start;
  while (char_is_alpha(current_char(lexer)) ||
         char_is_digit(current_char(lexer), 10) || current_char(lexer) == '_') {
    advance(lexer);
    ++end;
  }

  Token token = {.kind = TK_IDENTIFIER,
                 .source = str8(&lexer.input.str[start], end - start)};
  token.kind = identifier_type(token.source); // Check if it's a keyword

  return {.token = token, .maybe_error = LEX_OK};
}

TokenResult lex_number(Lexer &lexer) {
  U64 start = lexer.current;
  U64 end = start;

  while (char_is_digit(current_char(lexer), 10)) {
    advance(lexer);
    ++end;
  }

  // TODO decimal part

  Token t = {
      .kind = TK_KW_INT,
      .source = str8(&lexer.input.str[start], end - start),
      .num_value = 0, // TODO actually parse the num
  };
  TokenResult result = {.token = t};
  return result;
}

TokenResult next_token(Arena *arena, Lexer &lexer) {
  // Skip whitespaces
  while (char_is_whitespace(current_char(lexer))) {
    if (current_char(lexer) == '\n')
      ++lexer.line;
    ++lexer.current;
  }

  if (current_char(lexer) == '\0')
    return {.token = {.kind = TK_EOF}};

  U8 c = current_char(lexer);
  advance(lexer);

  switch (c) {
  case '(':
    return {.token = {.kind = TK_LEFT_PAREN}};
  case ')':
    return {.token = {.kind = TK_RIGHT_PAREN}};
  case '{':
    return {.token = {.kind = TK_LEFT_BRACE}};
  case '}':
    return {.token = {.kind = TK_RIGHT_BRACE}};
  case ';':
    return {.token = {.kind = TK_SEMICOLON}};
  case ',':
    return {.token = {.kind = TK_COMMA}};
  case '.':
    return {.token = {.kind = TK_DOT}};
  case '-':
    return {.token = {.kind = TK_MINUS}};
  case '+':
    return {.token = {.kind = TK_PLUS}};
  case '/':
    // Is a comment, skip the line.
    if (current_char(lexer) == '/') {
      advance(lexer);
      printf("HERE: %c\n", current_char(lexer));

      while (current_char(lexer) != '\n' && current_char(lexer) != '\0') {
        advance(lexer);
      }

      return next_token(arena, lexer);
    }

    return {.token = {.kind = TK_SLASH}};
  case '*':
    return {.token = {.kind = TK_STAR}};

  default:
    if (char_is_digit(c, 10)) {
      lexer.current--;
      return lex_number(lexer);
    }

    if (char_is_alpha(c) || c == '_') {
      lexer.current--; // Back up once
      return lex_identifier(lexer);
    }

    StringBuilder sb = sb_create(arena, 1024);
    sb_append(&sb, str8_lit("Unexpected Character! -> "));
    sb_appendf(&sb, "\'%c\', found at %lu:%lu", c, (S64)lexer.line,
               (S64)lexer.current);

    String8 err_str = sb_to_str8(&sb);

    return {.maybe_error = LEX_ERROR_INVALID_CHARACTER, .error_msg = err_str};
  };
}

auto perform_lex(Arena *arena, const char *file_name) -> LexResult {
  auto s = scratch_begin(&arena, 1);
  std::ifstream f(file_name);

  assert(f.is_open());

  U64 size = std::filesystem::file_size(file_name);

  U8 *b = arena_push_array<U8>(s.arena, size + 1);
  if (!f.read(reinterpret_cast<char *>(b), size)) {
    assert(false && "Couldn't read from file");
  }

  String8 str = str8(b, size);
  Lexer lexer = {.input = str, .current = 0, .line = 0};

  // Token *tokens_start = arena_push<Token>(arena);
  // Size token_count = 0;

  LexResult lex_result = {
      .tokens = nullptr,
      .token_count = 0,
  };

  while (true) {

    TokenResult result = next_token(s.arena, lexer);

    TokenResult *t = arena_push<TokenResult>(arena);
    if (lex_result.tokens == nullptr)
      lex_result.tokens = t;

    *t = result;
    lex_result.token_count++;

    if (result.token.kind == TK_EOF)
      break;
  }

  scratch_end(s);
  f.close();

  return lex_result;
}
