#include "arena.hpp"
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
  case TK_RIGHT_PAREN:
  case TK_LEFT_BRACE:
  case TK_RIGHT_BRACE:
  case TK_COMMA:
  case TK_DOT:
  case TK_SEMICOLON:
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
  case TK_STRING:
  case TK_NUMBER:
  case TK_KW_INT:
    return str8_lit("TK_KW_INT");
  case TK_EOF:
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
  Token *tokens;
  Size token_count;
  LexError maybe_error;
  String8 error_msg;
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

TokenResult next_token(Lexer &lexer) {
  // Skip whitespaces
  while (char_is_whitespace(current_char(lexer))) {
    if (current_char(lexer) == '\n')
      ++lexer.line;
    ++lexer.current;
  }

  if (current_char(lexer) == '\0')
    return {.token = {.kind = TK_EOF}};

  printf("HERE:\n%.*s", (int)(lexer.input.size - lexer.current),
         (char *)&lexer.input.str[lexer.current]);

  TokenResult tr = lex_identifier(lexer);

  String8 s = token_kind_to_str8(tr.token.kind);
  printf("kind: %.*s\n", (int)s.size, s.str);

  exit(1);

  // if (char_is_digit(current_char(lexer), 10)) {
  // }

  // for (Size i = 0; i < lexer.input.size; ++i) {
  //   if (char_is_whitespace(lexer.input.str[i])) {
  //   }
  //   printf("%c\n", lexer.input.str[i]);
  // }
  return {};
}

auto perform_lex(Arena *arena, const char *file_name) -> LexResult {
  auto s = scratch_begin(&arena, 1);
  std::ifstream f(file_name);

  assert(f.is_open());

  U64 size = std::filesystem::file_size(file_name);

  U8 *b = arena_push_array<U8>(s.arena, size + 1);
  if (!f.read(reinterpret_cast<char *>(b), size)) {
    return {.maybe_error = LEX_ERROR_IO,
            .error_msg = str8_lit("Couldn't read bytes from file")};
  }

  String8 str = str8(b, size);
  Lexer lexer = {.input = str, .current = 0, .line = 0};

  Token *tokens_start = arena_push<Token>(arena);
  Size token_count = 0;

  LexResult lex_result = {.tokens = tokens_start,
                          .token_count = token_count,
                          .maybe_error = LEX_OK,
                          .error_msg = {}};

  while (true) {
    TokenResult result = next_token(lexer);

    Token *t = arena_push<Token>(arena);
    *t = result.token;
    token_count++;

    if (result.maybe_error != LEX_OK) {
      lex_result.maybe_error = result.maybe_error;
      lex_result.error_msg = result.error_msg;
      break;
    }

    if (result.token.kind == TK_EOF)
      break;
  }

  scratch_end(s);
  f.close();

  lex_result.tokens = tokens_start;
  lex_result.token_count = token_count;
  return lex_result;
}
