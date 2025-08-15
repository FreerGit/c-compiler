#include "strings.hpp"
#include "arena.hpp"
#include <cstring>

// Character classification & conversions
bool char_is_whitespace(U8 c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool char_is_upper(U8 c) { return c >= 'A' && c <= 'Z'; }

bool char_is_lower(U8 c) { return c >= 'a' && c <= 'z'; }

bool char_is_alpha(U8 c) { return char_is_upper(c) || char_is_lower(c); }

bool char_is_slash(U8 c) { return c == '/' || c == '\\'; }

bool char_is_digit(U8 c, U32 base) {
  if (base <= 10)
    return c >= '0' && c < '0' + base;
  return (c >= '0' && c <= '9') || (c >= 'a' && c < 'a' + base - 10) ||
         (c >= 'A' && c < 'A' + base - 10);
}

U8 char_to_lower(U8 c) { return char_is_upper(c) ? c + 32 : c; }

U8 char_to_upper(U8 c) { return char_is_upper(c) ? c - 32 : c; }

// String constructors
String8 str8(U8 *str, U64 size) { return {str, size}; }
String8 str8_cstring(U8 *cstr) { return {cstr, (U64)strlen((char *)cstr)}; }

// String stylization
String8 upper_from_str8(Arena *arena, String8 string) {
  U8 *buf = arena_push_array<U8>(arena, string.size);
  for (U64 i = 0; i < string.size; ++i)
    buf[i] = char_to_upper(string.str[i]);
  return {buf, string.size};
}

String8 lower_from_str8(Arena *arena, String8 string) {
  U8 *buf = arena_push_array<U8>(arena, string.size);
  for (U64 i = 0; i < string.size; ++i)
    buf[i] = char_to_lower(string.str[i]);
  return {buf, string.size};
}

// String slicing
String8 str8_substr(String8 str, U64 start, U64 end) {
  if (start > str.size)
    start = str.size;
  if (end > str.size)
    end = str.size;
  if (end < start)
    end = start;
  return {str.str + start, end - start};
}

String8 str8_trim_whitespace(String8 s) {
  U64 start = 0;
  while (start < s.size && char_is_whitespace(s.str[start]))
    start++;
  U64 end = s.size;
  while (end > start && char_is_whitespace(s.str[end - 1]))
    end--;
  return {s.str + start, end - start};
}

// String Formatting & Copying
String8 str8_cat(Arena *arena, String8 s1, String8 s2) {
  U8 *buf = arena_push_array<U8>(arena, s1.size + s2.size);
  memcpy(buf, s1.str, s1.size);
  memcpy(buf + s1.size, s2.str, s2.size);
  return {buf, s1.size + s2.size};
}

// String matching
bool str8_match(String8 s1, String8 s2) {
  return (s1.size == s2.size) && (memcmp(s1.str, s2.str, s1.size) == 0);
}

bool str8_match_insensitive(String8 s1, String8 s2) {
  if (s1.size != s2.size)
    return false;
  for (U64 i = 0; i < s1.size; ++i) {
    if (char_to_lower(s1.str[i]) != char_to_lower(s2.str[i]))
      return false;
  }
  return true;
}

// String splitting & joining
// String8List str8_split(Arena *arena, String8 s, U8 sep);
// String8 str8_join(Arena *arena, String8List list, String8 sep);
