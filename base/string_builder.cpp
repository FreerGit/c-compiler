#include <arena.hpp>
#include <cassert>
#include <string_builder.hpp>

StringBuilder sb_create(Arena *arena, U64 capacity) {
  char *buf =
      static_cast<char *>(arena_push_size(arena, capacity, alignof(char)));
  return {arena, buf, 0, capacity};
}

void sb_append(StringBuilder *sb, std::string_view sv) {
  assert((sb->length + sv.size() <= sb->cap) && "SB overshot");
  std::memcpy(sb->start + sb->length, sv.data(), sv.size());
  sb->length += sv.size();
}

void sb_append_cstr(StringBuilder *sb, const char *cstr) {
  sb_append(sb, std::string_view(cstr, std::strlen(cstr)));
}

const char *sb_cstr(StringBuilder *sb) {
  sb->start[sb->length] = '\0'; // Null terminate
  return sb->start;
}
