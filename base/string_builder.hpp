#pragma once

#include <arena.hpp>
#include <cstring>
#include <string_view>

struct StringBuilder {
  Arena *arena;
  char *start;
  U64 length;
  U64 cap;
};

StringBuilder sb_create(Arena *arena, U64 capacity);

void sb_append(StringBuilder *sb, std::string_view sv);
void sb_append_cstr(StringBuilder *sb, U8 *cstr);
const char *sb_cstr(StringBuilder *sb);
