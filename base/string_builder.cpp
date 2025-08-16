#include "strings.hpp"
#include <arena.hpp>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <string_builder.hpp>

StringBuilder sb_create(Arena *arena, U64 length) {
  U8 *buf = arena_push_array<U8>(arena, length);
  return {arena, buf, 0, length};
}

void sb_append(StringBuilder *sb, String8 str) {
  assert((sb->length + str.size <= sb->cap) && "SB overshot");
  std::memcpy(sb->start + sb->length, str.str, str.size);
  sb->length += str.size;
}

void sb_appendf(StringBuilder *sb, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  va_list args_copy;
  va_copy(args_copy, args);
  int needed = vsnprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy);

  assert(needed >= 0 && "vsnprintf failed");

  U64 new_len = sb->length + (U64)needed;
  assert(new_len <= sb->cap && "SB overshot");

  int written = vsnprintf((char *)sb->start + sb->length,
                          sb->cap - sb->length + 1, // +1 for NUL
                          fmt, args);
  va_end(args);

  assert(written == needed && "vsnprintf wrote unexpected length");

  sb->length = new_len;
}

void sb_append_char(StringBuilder *sb, U8 c) { sb_append(sb, str8(&c, 1)); }

void sb_append_cstr(StringBuilder *sb, U8 *cstr) {
  sb_append(sb, str8_cstring(cstr));
}

void sb_append_signed(StringBuilder *sb, S64 num) {
  char tmp[32];
  int written = snprintf(tmp, sizeof(tmp), "%lld", (long long)num);
  assert(written >= 0 && "snprintf failed");
  assert((sb->length + (U64)written) <= sb->cap && "SB overshot");

  std::memcpy(sb->start + sb->length, tmp, (size_t)written);
  sb->length += (U64)written;
}

const char *sb_cstr(StringBuilder *sb) {
  sb->start[sb->length] = '\0'; // Null terminate
  return (char *)sb->start;
}

String8 sb_to_str8(StringBuilder *sb) { return str8_cstring(sb->start); }
