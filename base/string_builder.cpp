#include "strings.hpp"
#include <arena.hpp>
#include <cassert>
#include <string_builder.hpp>

StringBuilder sb_create(Arena *arena, U64 capacity) {
  char *buf =
      static_cast<char *>(arena_push_size(arena, capacity, alignof(char)));
  return {arena, buf, 0, capacity};
}

void sb_append(StringBuilder *sb, String8 str) {
  assert((sb->length + str.size <= sb->cap) && "SB overshot");
  std::memcpy(sb->start + sb->length, str.str, str.size);
  sb->length += str.size;
}

void sb_append_cstr(StringBuilder *sb, U8 *cstr) {
  sb_append(sb, str8_cstring(cstr));
}

const char *sb_cstr(StringBuilder *sb) {
  sb->start[sb->length] = '\0'; // Null terminate
  return sb->start;
}
