#include "arena.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

// Alignment helpers
constexpr U64 align_to_page_size(U64 size) noexcept {
  return (size + ARENA_PAGE_SIZE - 1) & ~(ARENA_PAGE_SIZE - 1);
}

// Arena creation/destruction
Arena arena_alloc(U64 capacity) {
  U64 aligned_capacity = align_to_page_size(capacity);

#ifndef NDEBUG
  // Debug: add extra guard page
  U64 total_size = aligned_capacity + ARENA_PAGE_SIZE;
  void *base = mmap(nullptr, total_size, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  assert(base != MAP_FAILED && "mmap failed");

  // Poison the guard page at the end
  U8 *guard_page = static_cast<U8 *>(base) + aligned_capacity;
  mprotect(guard_page, ARENA_PAGE_SIZE, PROT_NONE);

  return Arena{static_cast<U8 *>(base), 0, aligned_capacity};
#else
  // Release: normal allocation
  void *base = mmap(nullptr, aligned_capacity, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  assert(base != MAP_FAILED && "mmap failed");

  return Arena{static_cast<U8 *>(base), 0, aligned_capacity};
#endif
}

void arena_release(Arena *arena) {
#ifndef NDEBUG
  munmap(arena->base, arena->capacity + ARENA_PAGE_SIZE);
#else
  munmap(arena->base, arena->capacity);
#endif

  arena->base = nullptr;
  arena->capacity = 0;
  arena->offset = 0;
}

void *arena_push_size(Arena *arena, U64 size, U64 alignment) {
  assert(alignment && (alignment & (alignment - 1)) == 0 &&
         "Alignment must be a power of two");

  uintptr_t current_ptr =
      reinterpret_cast<uintptr_t>(arena->base + arena->offset);
  uintptr_t aligned_ptr = (current_ptr + alignment - 1) & ~(alignment - 1);

  U64 alignment_offset = aligned_ptr - current_ptr;
  U64 total_size = alignment_offset + size;

  assert(arena->offset + total_size <= arena->capacity && "Arena OOM!");

  void *result = reinterpret_cast<void *>(aligned_ptr);
  arena->offset += total_size;
  return result;
}

// Position and offset functions
U64 arena_pos(Arena *arena) { return arena->offset; }

void arena_pop_to(Arena *arena, U64 pos) {
  assert(pos <= arena->offset &&
         "Cannot pop to position beyond current offset");
  arena->offset = pos;
}

void arena_pop(Arena *arena, U64 amount) {
  U64 new_offset = (amount >= arena->offset) ? 0 : arena->offset - amount;
  arena_pop_to(arena, new_offset);
}

void arena_reset(Arena *arena) { arena_pop_to(arena, 0); }

// Temporary arena scopes
ArenaTemp temp_begin(Arena *arena) {
  U64 pos = arena_pos(arena);
  ArenaTemp temp = {arena, pos};
  return temp;
}

void temp_end(ArenaTemp temp) { arena_pop_to(temp.arena, temp.pos); }

// Scratch arenas
typedef struct {
  Arena arenas[2];
} Scratches;

thread_local Scratches *tl_scratches = nullptr;

void scratch_init_and_equip() {
  if (!tl_scratches) {
    tl_scratches = new Scratches{};
    for (std::size_t i = 0; i < 2; ++i) {
      tl_scratches->arenas[i] = arena_alloc(MiB(64));
    }
  }
}

Arena *tl_get_scratch(Arena *conflicts[], std::size_t count) {
  for (Arena &arena : tl_scratches->arenas) {
    bool has_conflict = false;
    for (std::size_t j = 0; j < count; ++j) {
      if (&arena == conflicts[j]) {
        has_conflict = true;
        break;
      }
    }
    if (!has_conflict)
      return &arena;
  }
  return nullptr; // no available scratch arena
}

ArenaTemp scratch_begin(Arena **conflicts, U64 count) {
  Arena *arena = tl_get_scratch(conflicts, count);
  return temp_begin(arena);
}

void scratch_end(ArenaTemp temp) { temp_end(temp); }
