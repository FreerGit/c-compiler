#pragma once
#include <cstdint>
#include <unistd.h>

using U64 = std::uint64_t;
using U32 = std::uint32_t;
using U16 = std::uint16_t;
using U8 = std::uint8_t;

using S64 = std::int64_t;
using S32 = std::int32_t;
using S16 = std::int16_t;
using S8 = std::int8_t;

using Ptr = std::uintptr_t;
using Size = std::size_t;

using F32 = float;
using F64 = double;

struct Arena {
  U8 *base;
  U64 offset;
  U64 capacity;
};

struct ArenaTemp {
  Arena *arena;
  U64 pos;
};

// Helpers
static constexpr auto GiB(U64 bytes) -> Size { return bytes << 30; }
static constexpr auto MiB(U64 bytes) -> Size { return bytes << 20; }
static constexpr auto KiB(U64 bytes) -> Size { return bytes << 10; }

// Alignment helpers
// Assume page size is 4096
static constexpr U64 ARENA_PAGE_SIZE = 4096;

constexpr U64 align_to_page_size(U64 size) noexcept;

template <typename T> constexpr U64 align_to(U64 size) noexcept;

// Arena creation/destruction
Arena arena_alloc(U64 capacity);
void arena_release(Arena *arena);

// Core arena functions
void *arena_push_size(Arena *arena, U64 size, U64 alignment = sizeof(void *));

template <typename T> T *arena_push(Arena *arena);

template <typename T> T *arena_push_array(Arena *arena, U64 count);

template <typename T> T *arena_push_zero(Arena *arena);

template <typename T> T *arena_push_array_zero(Arena *arena, U64 count);

// Position and offset functions
U64 arena_pos(Arena *arena);
void arena_pop_to(Arena *arena, U64 pos);
void arena_pop(Arena *arena, U64 amount);
void arena_reset(Arena *arena);

// Template implementations
template <typename T> constexpr U64 align_to(U64 size) noexcept {
  return (size + alignof(T) - 1) & ~(alignof(T) - 1);
}

template <typename T> T *arena_push(Arena *arena) {
  return static_cast<T *>(arena_push_size(arena, sizeof(T), alignof(T)));
}

template <typename T> T *arena_pusharena_push_array(Arena *arena, U64 count) {
  return static_cast<T *>(
      arena_push_size(arena, sizeof(T) * count, alignof(T)));
}

template <typename T> T *arena_push_zero(Arena *arena) {
  T *result = arena_push<T>(arena);
  memset(result, 0, sizeof(T));
  return result;
}

template <typename T> T *arena_push_array_zero(Arena *arena, U64 count) {
  T *result = arena_push_array<T>(arena, count);
  memset(result, 0, sizeof(T) * count);
  return result;
}

// Temporary arena scopes
ArenaTemp temp_begin(Arena *arena);
void temp_end(ArenaTemp temp);

// Scratch arenas
void scratch_init_and_equip();
ArenaTemp scratch_begin(Arena **conflicts, U64 count);
void scratch_end(ArenaTemp temp);
