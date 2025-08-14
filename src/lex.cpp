#include "arena.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>
#include <print>
#include <string_view>

enum TokenKind {
  TK_EOF,
  TK_IDENT,
  TK_CONSTANT,
  TK_PUNCT,
  TK_KEYWORD, // int, void, return, etc
  TK_STRING,
  TK_OPEN_BRACE,
};

auto perform_lex(Arena *arena, const char *file_name) -> int {
  auto s = scratch_begin(&arena, 1);
  std::ifstream f(file_name);
  assert(f.is_open());

  U64 size = std::filesystem::file_size(file_name);

  char *b = arena_push_array<char>(s.arena, size + 1);
  if (!f.read(b, size)) {
    return -1;
  }

  b[size] = '\0';

  std::println("FILE:\n{}", b);
  std::println("{} {}", size, s.arena->offset);
  scratch_end(s);
  return 0;
}
