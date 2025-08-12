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
  printf("IS DEBUG");
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

// Per-thread scratch arenas
thread_local struct {
  Arena arenas[SCRATCH_ARENA_COUNT];
  bool initialized = false;
} tl_scratch = {};

static void scratch_init() {
  if (!tl_scratch.initialized) {
    for (U32 i = 0; i < SCRATCH_ARENA_COUNT; ++i) {
      tl_scratch.arenas[i] = arena_alloc(SCRATCH_ARENA_SIZE);
    }
    tl_scratch.initialized = true;
  }
}

ArenaTemp get_scratch(Arena **conflicts, U64 conflict_count) {
  scratch_init();

  for (U32 i = 0; i < SCRATCH_ARENA_COUNT; ++i) {
    Arena *candidate = &tl_scratch.arenas[i];

    bool is_conflict = false;
    for (U64 j = 0; j < conflict_count; ++j) {
      if (candidate == conflicts[j]) {
        is_conflict = true;
        break;
      }
    }

    if (!is_conflict) {
      return ArenaTemp{candidate, arena_pos(candidate)};
    }
  }

  assert(false && "No available scratch arena found!");
  return ArenaTemp{nullptr, 0};
}

ArenaTemp get_scratch() { return get_scratch(nullptr, 0); }

ArenaTemp get_scratch(Arena *conflict1) {
  Arena *conflicts[] = {conflict1};
  return get_scratch(conflicts, 1);
}

void release_scratch(ArenaTemp temp) {
  if (temp.arena) {
    arena_pop_to(temp.arena, temp.pos);
  }
}
