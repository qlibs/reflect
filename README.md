<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/reflect/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Freflect.svg)</a>
<a href="https://godbolt.org/z/qehrne83x">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## C++20 minimal static reflection library

> https://en.wikipedia.org/wiki/Reflective_programming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/reflect/main/reflect)
- Minimal (see [#API](#api))
- Verfifies itself upon include (run all tests via static_asserts)
- Compiler changes agnostic (no ifdefs for compiler specific implementations)

### Requirements

- C++20* ([gcc-12+](https://godbolt.org/z/313c4rq14), [clang-15+](https://godbolt.org/z/qcrxGnTMK), [msvc-19.36+](https://godbolt.org/z/YPxdaobEv))

---

### Hello world

>

```cpp
```

---

### API

```cpp
template <class T, auto Size> struct fixed_string;
template <class T, auto Size> fixed_string(const T (&str)[Size]) -> fixed_string<T, Size-1>;
template<class... Cs> fixed_string(const Cs... cs) -> fixed_string<std::common_type_t<Cs...>, sizeof...(Cs)>;

{
  static_assert(0u == std::size(fixed_string{""}));
  static_assert(fixed_string{""} == fixed_string{""});
  static_assert(std::string_view{""} == std::string_view{fixed_string{""}});
  static_assert(3u == std::size(fixed_string{"foo"}));
  static_assert(std::string_view{"foo"} == std::string_view{fixed_string{"foo"}});
  static_assert(fixed_string{"foo"} == fixed_string{"foo"});
}
```

``cpp
consteval auto debug(auto&&...) -> void;

{
  debug(foo{}); // compile-time error: debug(foo) is not defined
}
```

```cpp
template <class Fn, class T>
[[nodiscard]] constexpr auto visit(Fn&& fn, T&& t) noexcept;

struct foo { int a; int b; };

{
  static_assert(2 == visit([](auto&&... args) { return sizeof...(args); }, foo{}));
}
```

```cpp
template<class T> inline constexpr auto size = /*unspecified */

struct foo { int a; int b; };

{
  static_assert(2 == size<foo>);
}
```

```cpp
template <class T>
[[nodiscard]] consteval auto type_name(const T& = {}) noexcept;

struct foo { int a; int b; };

{
  static_assert(std::string_view{"foo"} == type_name<foo>());
  static_assert(std::string_view{"foo"} == type_name(foo{}));
}
```

```cpp
template <unsigned Min = REFLECT_ENUM_MIN, unsigned Max = REFLECT_ENUM_MAX, class E>
  requires (std::is_enum_v<E> and Max > Min)
[[nodiscard]] constexpr auto enum_name(const E e = {}) noexcept;

enum class Enum { foo = 1, bar = 2 };

{
  static_assert(std::string_view{"foo"} == enum_name(Enum::foo));
  static_assert(std::string_view{"bar"} == enum_name(Enum::bar));
}
```

```cpp
template <auto N, class T>
  requires (N < size<T>)
[[nodiscard]] consteval auto member_name(const T& = {}) noexcept;

struct foo { int a; int b; };

{
  static_assert(std::string_view{"a"} == member_name<0, foo>());
  static_assert(std::string_view{"a"} == member_name<0>(foo{}));
  static_assert(std::string_view{"b"} == member_name<1, foo>());
  static_assert(std::string_view{"b"} == member_name<1>(foo{}));
}
```

```cpp
template<auto N, class T>
  requires (std::is_integral_v<decltype(N)> and N < size<std::remove_cvref_t<T>>)
[[nodiscard]] constexpr decltype(auto) get(T&& t) noexcept;

struct foo { int a; bool b; };

{
  constexpr auto f = foo{.i=42, .b=true};
  static_assert(42 == get<0>(f));
  static_assert(true == get<1>(f));
}
```

```cpp
template <class T, fixed_string Name>
concept has_member_name = /*unspecified*/;

struct foo { int a; int b; };

{
  static_assert(has_member_name<foo, "a">);
  static_assert(has_member_name<foo, "b">);
  static_assert(not has_member_name<foo, "c">);
}
```

```cpp
template<fixed_string Name, class T>
  requires has_member_name<T, Name>
constexpr decltype(auto) get(T&& t) noexcept;

struct foo { int a; int b; };

{
  constexpr auto f = foo{.i=42, .b=true};
  static_assert(42 == get<"a">(f));
  static_assert(true == get<"b">(f));
}
```

```cpp
template<class R, class T>
[[nodiscard]] constexpr auto to(T&& t) noexcept;

struct foo { int a; int b; };
struct bar { int a{}; int b{}; };

{
  constexpr auto b = to<bar>(foo{.a=4, .b=2});
  static_assert(4 == b.a);
  static_assert(2 == b.b);
}

{
  auto f = foo{.a=4, .b=2};
  auto b = to<bar>(f);
  f.a = 42;
  assert(42 == f.a);
  assert(4 == b.a);
  assert(2 == b.b);
}
```

```cpp
template<template<class...> class R, class T>
[[nodiscard]] constexpr auto to(T&& t) noexcept;

struct foo { int a; int b; };

{
  constexpr auto t = to<std::tuple>(foo{.a=4, .b=2});
  static_assert(4 == std::get<0>(t));
  static_assert(2 == std::get<1>(t));
}

{
  auto f = foo{.a=4, .b=2};
  auto t = to<std::tuple>(f);
  std::get<0>(t) *= 10;
  f.b=42;
  assert(40 == std::get<0>(t) and 40 == f.a);
  assert(42 == std::get<1>(t) and 42 == f.b);
}
```

```cpp
template<class R, class T>
[[nodiscard]] constexpr auto as(T&& t);

struct foo { int a; int b; };
struct baz {
  int a{};
  int c{};
};

{
  const auto b = as<baz>(foo{.a=4, .b=2});
  assert(4 == b.a and 0 == b.c);
}
```

> Configuration

```cpp
#define REFLECT 1'0'0       // Current library version (SemVer)
#define REFLECT_MAX_SIZE 64 // Max number of member fields to be reflected
#define REFLECT_ENUM_MIN 0  // Min size for enum name
#define REFLECT_ENUM_MAX 64 // Min size for enum name lookup
```

---

### FAQ

- How `reflect` compares to https://wg21.link/P2996?

    > `reflect` library only provides basic reflection primitvies, mostly via hacks and workarounds to deal with lack of the reflection.
    https://wg21.link/P2996 is a language proprosal with a lot of more features and capabilities.
    If accepted there is no reason to use `reflect` library from C++26+.

- How `reflect` works under the hood?

    > There are a few different workarounds which can be applied to achieve reflection in C++20. `reflect` uses structure bindings, concepts and source_location. See visit implementation for more details.

- How `reflect` can be compiler changes agnostic?

    > `reflect` precomputes required prefixes/postfixes to find intresting data (for the known up-front types) from the `source_location::function_name()` output for each compiler upon inclusion.
    Any compiler change will end up with new prefixes/postfixes and won't require additional maintanace.

- How `reflect` can be compiler changes agnostic?

- What does it mean that `reflect` tests itself upon inlude?

    > `reflect` runs all tests (via static_asserts) upon include. If the include compiled it means all tests are passing and the library works correctly on given compiler, enviornment.

- What is compile-time overhead of `reflect` library?

    > `reflect` include takes < 20ms (that includes running all tests).
    The most expensive calls are `visit` and `enum_to_name` which timing will depend on the number of reflected elements and/or min/max values provided.
    There are no recursive template instantiations in the library.

- How to extend number of members to be reflected (default: 64)?

    > Add new `reflect::detail::visit` overleads

    ```cpp
        template <class Fn, class T>
          requires requires { std::remove_cvref_t<T>{any{}, any{}}; }
        [[nodiscard]] constexpr decltype(auto) reflect::detail::visit(Fn&& fn, T&& t, tag<2>) noexcept {
          auto&& [_1, _2] = std::forward<T>(t);
          return std::forward<Fn>(fn)(std::forward<decltype(_1)>(_1), std::forward<decltype(_2)>(_2));
        }
    ```

    > #define REFLECT_MAX_SIZE <<Number of supported overloads>>

- What is row polymorphism?

- Similar projects?
    > [boost.pfr](https://github.com/boostorg/pfr), [glaze](https://github.com/stephenberry/glaze), [reflect-cpp](https://github.com/getml/reflect-cpp)

- How can I contribute?

    > Please follow [CONTRIBUTING.md](.github/CONTRIBUTING.md)

- How to report a bug?

    > Please follow [PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md)
---

**Disclaimer** `reflect` is not an official Boost library.
