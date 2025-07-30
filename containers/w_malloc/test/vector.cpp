#include "vec.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdio>

TEST_CASE("default constructor", "[vec]") {
  auto v = Vector<int>();
  REQUIRE(v.empty());
  REQUIRE(v.size() == 0);
}

TEST_CASE("size constructor") {
  auto v = Vector<int>(1);
  v.push_back(1);
  v.push_back(2);
  REQUIRE(!v.empty());
  REQUIRE(v.size() == 2);
  // for(std::size_t i = 0; i < v.size(); i++) {
  //   std::printf("%i\n", v[i]);
  // }
}

// element access
TEST_CASE("at") {
  auto v = Vector<size_t>(8);
  REQUIRE_THROWS(v.at(0));
  v.push_back(5);
  REQUIRE_NOTHROW(v.at(0));
  REQUIRE(v.at(0) == 5);
  v.at(0) = 4;
  REQUIRE(v.at(0) == 4);
}

TEST_CASE("operator[]") {
  auto v = Vector<char>();
  v.push_back('c');
  REQUIRE(v[0] == 'c');
  v[0] = 'a';
  REQUIRE(v[0] == 'a');
}
