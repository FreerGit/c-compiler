#include "arena.hpp"
#include "string_builder.hpp"
#include <cassert>
#include <cstdio>
#include <print>
#include <string_view>
#include <vector>

#include "lex.cpp"

enum Stage { LEX, PARSE, CODEGEN, ALL };

auto main(int argc, char *argv[]) -> int {
  scratch_init_and_equip();
  auto arena = arena_alloc(GiB(4));

  if (argc < 2) {
    std::println("Wrong arguments {}", argc);
    return 1;
  }
  std::vector<std::string> args(argv, argv + argc);

  Stage stage = ALL;
  const char *name = "";
  for (auto &arg : args) {
    if (arg == "--lex") {
      assert(stage == ALL);
      stage = LEX;
    } else if (arg == "--parse") {
      assert(stage == ALL);
      stage = PARSE;
    } else if (arg == "--codegen") {
      assert(stage == ALL);
      stage = CODEGEN;
    } else {
      name = arg.c_str();
    }
  }

  std::println("{} {}", name, (U8)stage);

  switch (stage) {
  case LEX: {
    bool had_errors = false;
    LexResult result = perform_lex(&arena, name);
    for (Size i = 0; i < result.token_count; ++i) {

      TokenResult t = result.tokens[i];
      if (t.maybe_error != LEX_OK) {
        had_errors = true;

        printf("ERR: %d - %.*s\n", t.maybe_error, (int)t.error_msg.size,
               t.error_msg.str);
      } else {
        String8 s = token_kind_to_str8(t.token.kind);
        printf("kind: %.*s, iden: '%.*s'\n", (int)s.size, s.str,
               (int)t.token.source.size, t.token.source.str);
      }
    }

    if (had_errors)
      exit(1);

    return 0;
  };
  default:
    assert(false && "TODO");
  }

  // // call gcc's preprocessor
  // {
  //   auto ta = get_scratch();
  //   StringBuilder sb = sb_create(ta.arena, ta.arena->capacity);
  //   sb_append(&sb, "gcc -E -P ");

  //   sb_append(&sb, name);
  //   sb_append(&sb, " -o ");
  //   sb_append(&sb, name.substr(0, name.size() - 2));
  //   sb_append(&sb, ".i");
  //   // std::string_view pps_command = ;
  //   std::println("{}", sb_cstr(&sb));
  //   system(sb_cstr(&sb));
  //   release_scratch(ta);
  // }

  // // Compile the preprocessed file, stubbed, delete after
  // // just delete it for now
  // {
  //   auto ta = get_scratch();
  //   StringBuilder sb = sb_create(ta.arena, ta.arena->capacity);
  //   sb_append(&sb, "gcc ");

  //   sb_append(&sb, name);
  //   sb_append(&sb, " -o ");
  //   sb_append(&sb, name.substr(0, name.size() - 2));
  //   // std::string_view pps_command = ;
  //   std::println("{}", sb_cstr(&sb));
  //   system(sb_cstr(&sb));
  //   release_scratch(ta);
  // }
  return 0;
}
