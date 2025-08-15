#pragma oncep
#include "arena.hpp"

struct String8 {
  U8 *str;
  U64 size;
};

struct String8List {
  String8 str;
  String8List *next;
  U64 count;
};

// Character classification & conversions
bool char_is_whitespace(U8 c);
bool char_is_upper(U8 c);
bool char_is_lower(U8 c);
bool char_is_alpha(U8 c);
bool char_is_slash(U8 c);
bool char_is_digit(U8 c, U32 base);
U8 char_to_lower(U8 c);
U8 char_to_upper(U8 c);

// String constructors
#define str8_lit(s)                                                            \
  String8 { (U8 *)(s), sizeof(s) - 1 }

String8 str8(U8 *str, U64 size);
String8 str8_cstring(U8 *cstr);

// String stylization
String8 upper_from_str8(Arena *arena, String8 string);
String8 lower_from_str8(Arena *arena, String8 string);

// String slicing
String8 str8_substr(String8 str, U64 start, U64 end);

String8 str8_trim_whitespace(String8 s);

// String Formatting & Copying
String8 str8_cat(Arena *arena, String8 s1, String8 s2);

// String matching
bool str8_match(String8 s1, String8 s2);
bool str8_match_insensitive(String8 s1, String8 s2);

// String splitting & joining
String8List str8_split(Arena *arena, String8 s, U8 sep);
String8 str8_join(Arena *arena, String8List list, String8 sep);
