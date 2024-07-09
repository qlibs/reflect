<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/reflect/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Freflect.svg)</a>
<a href="https://godbolt.org/z/zvooxGPP9">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/EW1evKeW4">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## C++20 minimal static reflection library

> https://en.wikipedia.org/wiki/Reflective_programming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/reflect/main/reflect)
    - Easy integration (see [FAQ](#faq))
- Minimal [API](#api)
- Verifies itself upon include (aka run all tests via static_asserts but it can be disabled with `-DNTEST` - see [FAQ](#faq))
    - Basically guarantees no UB, no memory leaks*
- Compiles cleanly with ([`-fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`](https://godbolt.org/z/M747ocGfx))
- Agnostic to compiler changes (no ifdefs for the compiler specific implementations - see [FAQ](#faq))
- Optimized run-time execution and binary size (see [performance](#perf))
- Fast compilation times (see [compilation times](#comp))

### Requirements

- C++20 ([gcc-12+, clang-15+, msvc-19.36+](https://godbolt.org/z/xPc19Moef))
    - STL headers (array, string_view, source_location, type_traits, utility, [tuple])

---

### Hello world (https://godbolt.org/z/EW1evKeW4)

```cpp
#include <reflect>

enum E { A, B };
struct foo { int a; E b; };

constexpr auto f = foo{.a = 42, .b = B};

// reflect::size
static_assert(2 == reflect::size(f));

// reflect::type_id
static_assert(reflect::type_id(f.a) != reflect::type_id(f.b));

// reflect::type_name
static_assert("foo"sv == reflect::type_name(f));
static_assert("int"sv == reflect::type_name(f.a));
static_assert("E"sv   == reflect::type_name(f.b));

// reflect::enum_name
static_assert("B"sv == reflect::enum_name(f.b));

// reflect::member_name
static_assert("a"sv == reflect::member_name<0>(f));
static_assert("b"sv == reflect::member_name<1>(f));

// reflect::get
static_assert(42 == reflect::get<0>(f)); // by index
static_assert(B  == reflect::get<1>(f));

static_assert(42 == reflect::get<"a">(f)); // by name
static_assert(B  == reflect::get<"b">(f));

// reflect::to
constexpr auto t = reflect::to<std::tuple>(f);
static_assert(42 == std::get<0>(t));
static_assert(B  == std::get<1>(t));

int main() {
  reflect::for_each([](auto I) {
    std::print("{}.{}:{}={} ({}/{}/{})\n",
        reflect::type_name(f),                  // foo, foo
        reflect::member_name<I>(f),             // a  , b
        reflect::type_name(reflect::get<I>(f)), // int, E
        reflect::get<I>(f),                     // 42 , B
        reflect::size_of<I>(f),                 // 4  , 4
        reflect::align_of<I>(f),                // 4  , 4
        reflect::offset_of<I>(f));              // 0  , 4
  }, f);
}

// and more (see API)...
```

---

### Examples

- [feature] Opt-in mixins - https://godbolt.org/z/sj7fYKoc3
- [feature] Meta-programming (https://github.com/boost-ext/mp) - https://godbolt.org/z/s7ce6bh5d
- [future] Structured Bindings can introduce a Pack (https://wg21.link/P1061) - https://godbolt.org/z/Ga3bc3KKW
- [performance] Minimal-Perfect-Hashing based `enum_name` (https://github.com/boost-ext/mph) - https://godbolt.org/z/Kb9bWz4zP

---

<a name="perf"></a>
### Performance/Binary size (https://godbolt.org/z/7TbobjWfj)

```cpp
struct foo { int bar; };
auto type_name(const foo& f) { return reflect::type_name(f); }
```

```asm
type_name(foo const&): // $CXX -O3 -DNDEBUG
        lea     rdx, [rip + type_name<foo>]
        mov     eax, 3
        ret

type_name<foo>
        .ascii  "foo"
```

```cpp
struct foo { int bar; };
auto member_name(const foo& f) { return reflect::member_name<0>(f); }
```

```asm
member_name(foo const&): // $CXX -O3 -DNDEBUG
        lea     rdx, [rip + member_name<0ul, foo>]
        mov     eax, 3
        ret

member_name<0ul, foo>
        .ascii  "bar"
```

```cpp
enum class E { A, B, };
auto enum_name(const E e) { return reflect::enum_name(e); }
```

```asm
enum_name(E): // $CXX -O3 -DNDEBUG (generates switch)
        xor     eax, eax
        xor     ecx, ecx
        cmp     edi, 1
        sete    cl
        lea     rdx, [rip + enum_name<0>]
        cmove   rax, rdx
        test    edi, edi
        lea     rdx, [rip + enum_name<1>]
        cmovne  rdx, rax
        mov     eax, 1
        cmovne  rax, rcx
        ret

enum_name<0ul>:
        .ascii  "A"

enum_name<1ul>:
        .ascii  "B"
```

<a name="comp"></a>
### Compilation times

> [include] https://raw.githubusercontent.com/boost-ext/reflect/main/reflect

```cpp
time g++-13.2 -x c++ -std=c++20 reflect -c -DNTEST   # 0.113s
time g++-13.2 -x c++ -std=c++20 reflect -c           # 0.253s
```

```cpp
time clang++-17 -x c++ -std=c++20 reflect -c -DNTEST # 0.119s
time clang++-17 -x c++ -std=c++20 reflect -c         # 0.322s
```

---

### API

```cpp
template <class Fn, class T> requires std::is_aggregate_v<std::remove_cvref_t<T>>
[[nodiscard]] constexpr auto visit(Fn&& fn, T&& t) noexcept;
```

```cpp
struct foo { int a; int b; };
static_assert(2 == visit([](auto&&... args) { return sizeof...(args); }, foo{}));
```

```cpp
template<class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto size() -> std::size_t;

template<class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto size(const T&) -> std::size_t;
```

```cpp
struct foo { int a; int b; } f;
static_assert(2 == size<foo>());
static_assert(2 == size(f));
```

```cpp
template <class T> [[nodiscard]] constexpr auto type_name() noexcept;
template <class T> [[nodiscard]] constexpr auto type_name(const T&) noexcept;
```

```cpp
struct foo { int a; int b; };
static_assert(std::string_view{"foo"} == type_name<foo>());
static_assert(std::string_view{"foo"} == type_name(foo{}));
```

```cpp
template <class T> [[nodiscard]] constexpr auto type_id() noexcept;
template <class T> [[nodiscard]] constexpr auto type_id(T&&) noexcept;
```

```cpp
struct foo { };
struct bar { };
static_assert(type_id(foo{}) == type_id(foo{}));
static_assert(type_id(bar{}) != type_id<foo>());
```

```cpp
template<class E>
[[nodiscard]] constexpr auto to_underlying(const E e) noexcept;

template<class E> requires std::is_enum_v<E>
consteval auto enum_min(const E = {}) { return REFLECT_ENUM_MIN; }

template<class E> requires std::is_enum_v<E>
consteval auto enum_max(const E = {}) { return REFLECT_ENUM_MAX; }

template<class E,
         fixed_string unknown = "",
         auto Min = enum_min(E{}),
         auto Max = enum_max(E{})>
  requires (std::is_enum_v<E> and Max > Min)
[[nodiscard]] constexpr auto enum_name(const E e) noexcept -> std::string_view {
```

```cpp
enum class Enum { foo = 1, bar = 2 };
static_assert(std::string_view{"foo"} == enum_name(Enum::foo));
static_assert(std::string_view{"bar"} == enum_name(Enum::bar));
```

```cpp
enum class Enum { foo = 1, bar = 1024 };
consteval auto enum_min(Enum) { return Enum::foo; }
consteval auto enum_max(Enum) { return Enum::bar; }

static_assert(std::string_view{"foo"} == enum_name(Enum::foo));
static_assert(std::string_view{"bar"} == enum_name(Enum::bar));
```

```cpp
template <std::size_t N, class T>
  requires (std::is_aggregate_v<T> and N < size<T>())
[[nodiscard]] constexpr auto member_name(const T& = {}) noexcept;
```

```cpp
struct foo { int a; int b; };
static_assert(std::string_view{"a"} == member_name<0, foo>());
static_assert(std::string_view{"a"} == member_name<0>(foo{}));
static_assert(std::string_view{"b"} == member_name<1, foo>());
static_assert(std::string_view{"b"} == member_name<1>(foo{}));
```

```cpp
template<std::size_t N, class T>
  requires (std::is_aggregate_v<std::remove_cvref_t<T>> and
            N < size<std::remove_cvref_t<T>>())
[[nodiscard]] constexpr decltype(auto) get(T&& t) noexcept;
```

```cpp
struct foo { int a; bool b; };
constexpr auto f = foo{.i=42, .b=true};
static_assert(42 == get<0>(f));
static_assert(true == get<1>(f));
```

```cpp
template <class T, fixed_string Name> requires std::is_aggregate_v<T>
concept has_member_name = /*unspecified*/
```

```cpp
struct foo { int a; int b; };
static_assert(has_member_name<foo, "a">);
static_assert(has_member_name<foo, "b">);
static_assert(not has_member_name<foo, "c">);
```

```cpp
template<fixed_string Name, class T> requires has_member_name<T, Name>
constexpr decltype(auto) get(T&& t) noexcept;
```

```cpp
struct foo { int a; int b; };
constexpr auto f = foo{.i=42, .b=true};
static_assert(42 == get<"a">(f));
static_assert(true == get<"b">(f));
```

```cpp
template<fixed_string... Members, class TSrc, class TDst>
  requires (std::is_aggregate_v<TSrc> and std::is_aggregate_v<TDst>)
constexpr auto copy(const TSrc& src, TDst& dst) noexcept -> void;
```

```cpp
struct foo { int a; int b; };
struct bar { int a{}; int b{}; };

bar b{};
foo f{};

copy(f, b);
assert(b.a == f.a);
assert(b.b == f.b);

copy<"a">(f, b);
assert(b.a == f.a);
assert(0 == b.b);
```

```cpp
template<template<class...> class R, class T>
  requires std::is_aggregate_v<std::remove_cvref_t<T>>
[[nodiscard]] constexpr auto to(T&& t) noexcept;
```

```cpp
struct foo { int a; int b; };

constexpr auto t = to<std::tuple>(foo{.a=4, .b=2});
static_assert(4 == std::get<0>(t));
static_assert(2 == std::get<1>(t));

auto f = foo{.a=4, .b=2};
auto t = to<std::tuple>(f);
std::get<0>(t) *= 10;
f.b = 42;
assert(40 == std::get<0>(t) and 40 == f.a);
assert(42 == std::get<1>(t) and 42 == f.b);
```

```cpp
template<class R, class T>
[[nodiscard]] constexpr auto to(T&& t);
```

```cpp
struct foo { int a; int b; };
struct baz { int a{}; int c{}; };

const auto b = to<baz>(foo{.a=4, .b=2});
assert(4 == b.a and 0 == b.c);
```

```cpp
template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto size_of() -> std::size_t;

template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto size_of(T&&) -> std::size_t;

template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto align_of() -> std::size_t;

template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto align_of(T&&) -> std::size_t;

template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto offset_of() -> std::size_t;

template<std::size_t N, class T> requires std::is_aggregate_v<T>
[[nodiscard]] constexpr auto offset_of(T&&) -> std::size_t;
```

```cpp
struct foo { int a; bool b; };

static_assert(4 == size_of<0, foo>());
static_assert(1 == size_of<1, foo>());
static_assert(4 == align_of<0, foo>());
static_assert(1 == align_of<1, foo>());
static_assert(0 == offset_of<0, foo>());
static_assert(4 == offset_of<1, foo>());
```

```cpp
template<class Fn, class T>
  requires std::is_aggregate_v<std::remove_cvref_t<T>>
constexpr auto for_each(Fn&& fn) -> void;

template<class Fn, class T>
  requires std::is_aggregate_v<std::remove_cvref_t<T>>
constexpr auto for_each(Fn&& fn, T&& t) -> void;
```

```cpp
struct { int a; int b; } f;

reflect::for_each([&f](const auto I) {
  std::print("{}:{}={}", member_name<I>(f), get<I>(f)); // prints a:int=4, b:int=2
}, f);
```

> Configuration

```cpp
#define REFLECT 1'2'0           // Current library version (SemVer)
#define REFLECT_ENUM_MIN -1     // Min size for enum name (can be overridden)
                                // For example: `-DREFLECT_ENUM_MIN=0`
#define REFLECT_ENUM_MAX 1024   // Max size for enum name (can be overridden)
                                // For example: `-DREFLECT_ENUM_MAX=32`
#define NTEST                   // Disables running static_asserts tests
                                // Not enabled by default (use with caution)
```
---

### FAQ

- How does `reflect` compare to https://wg21.link/P2996?

    > `reflect` library only provides basic reflection primitives, mostly via hacks and workarounds to deal with lack of the reflection.
    https://wg21.link/P2996 is a language proposal with many more features and capabilities. It's like comparing a drop in the ocean to the entire sea!

- How does `reflect` work under the hood?

    > There are many different ways to implement reflection. `reflect` uses C++20's structure bindings, concepts and source_location to do it. See `visit` implementation for more details.

- How can `reflect` be agnostic to compiler changes?

    > `reflect` precomputes required prefixes/postfixes to find required names from the `source_location::function_name()` output for each compiler upon inclusion.
    Any compiler change will end up with new prefixes/postfixes and won't require additional maintenance.

- What does it mean that `reflect` tests itself upon include?

    > `reflect` runs all tests (via static_asserts) upon include. If the include compiles it means all tests are passing and the library works correctly on given compiler, environment.

- What is compile-time overhead of `reflect` library?

    > `reflect` include takes ~.2s (that includes running all tests).
    The most expensive calls are `visit` and `enum_to_name` whose timing will depend on the number of reflected elements and/or min/max values provided.
    There are no recursive template instantiations in the library.

- Can I disable running tests at compile-time for faster compilation times?

    > When `NTEST` is defined static_asserts tests won't be executed upon inclusion.
    Note: Use with caution as disabling tests means that there are no guarantees upon inclusion that the given compiler/env combination works as expected.

- How to extend the number of members to be reflected (default: 64)?

    > Override `visit`, for example - https://godbolt.org/z/Ga3bc3KKW

    ```cpp
    template <class Fn, class T> // requires https://wg21.link/P1061
    [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t) noexcept {
      auto&& [... ts] = std::forward<T>(t);
      return std::forward<Fn>(fn)(std::forward_like<T>(ts)...);
    }
    ```

- How to integrate with CMake/CPM?

    ```
    CPMAddPackage(
      Name reflect
      GITHUB_REPOSITORY boost-ext/reflect
      GIT_TAG v1.2.0
    )
    add_library(reflect INTERFACE)
    target_include_directories(reflect SYSTEM INTERFACE ${reflect_SOURCE_DIR})
    add_library(reflect::reflect ALIAS reflect)
    ```

    ```
    target_link_libraries(${PROJECT_NAME} reflect::reflect);
    ```

- Similar projects?
    > [boost.pfr](https://github.com/boostorg/pfr), [glaze](https://github.com/stephenberry/glaze), [reflect-cpp](https://github.com/getml/reflect-cpp), [magic_enum](https://github.com/Neargye/magic_enum)

---

**Disclaimer** `reflect` is not an official Boost library.
