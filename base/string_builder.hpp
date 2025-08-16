#pragma once

#include "strings.hpp"
#include <arena.hpp>
#include <cstring>

struct StringBuilder {
  Arena *arena;
  U8 *start;
  U64 length;
  U64 cap;
};

StringBuilder sb_create(Arena *arena, U64 capacity);

void sb_append(StringBuilder *sb, String8 s);
void sb_appendf(StringBuilder *sb, const char *fmt, ...);
void sb_append_char(StringBuilder *sb, U8 c);
void sb_append_cstr(StringBuilder *sb, U8 *cstr);
void sb_append_signed(StringBuilder *sb, S64 num);
const char *sb_to_cstr(StringBuilder *sb);
String8 sb_to_str8(StringBuilder *sb);
