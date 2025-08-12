#include "arena.hpp"
#include <cstdio>
#include <ostream>

auto a(auto &arena) -> void {
  ArenaTemp outer = get_scratch();

  U32 *outer_num = arena_push<U32>(outer.arena);
  *outer_num = 999;

  printf("Outer before inner: %u\n", *outer_num);

  // Nested scratch arena
  b(arena);

  printf("Outer after inner: %u\n", *outer_num);

  release_scratch(outer);
}

auto b(Arena &arena) -> void {
  auto temp = get_scratch();
  U32 *num = arena_push<U32>(&arena);
  *num = 123;

  char *msg = arena_push_array<char>(&arena, 6);
  snprintf(msg, 6, "hello");

  printf("Number: %u\n", *num);
  printf("Message: %s\n", msg);

  release_scratch(temp);
}

auto main() -> int {

  auto x = arena_alloc(1024);
  auto s = printf("%lu\n", arena_pos(&x));
  auto n = arena_push<U32>(&x);
  auto y = arena_pos(&x);
  printf("%lu, %u\n", y, *n);
  *n = 55;
  printf("%u\n", *n);

  a(x);
  return 0;
}
