anonymous constants: past, present and future

---

## Why anonymous constants?

1. Naming everything is not solution otherwise we wouldn't have lambdas i.e. anonymous functions
1. Requires name
1. Excessively wordy
1. Moved far from point of use
1. Requires maintaining file, manual sorting sparse list

---

## Why should anonymous constants be static?

- programmer expectation that constants are static like string "Hello World" and assembly inline/local constants
- temporaries can immediately dangle; dangling constants are embarrassing
- want ROM guarantee or at least next best thing const and static
- memory safe
- thread safe

---

## Requirements

1. const
1. constexpr; can be constructed at compile time
1. consteval

Result
- constinit; sensible lifetime extension of a temporary to a global

---

## Lambda and macros (present)

**Current best solution**

```cpp
const std::string& dangler(const std::string& s) { return s; }
#define CONSTANT ...// macro can conceal lambda usage

void h() {
    dangler([] /*consteval*/ -> auto const & {
            static constinit const std::string anonymous{"Hello World"};
            return anonymous;
        }());
    dangler(CONSTANT("Hello World"s));
}
```

**CON(s)**
- How many times must we say const?
- Compilers must undue boilerplate

---

## Static storage for braced initializers (C++26)

```cpp
void f(std::initializer_list<double> il);

void h() {
    f({1, 2, 3});
    //static constexpr double __b[3] = {double{1}, double{2}, double{3}}; // backing array
    //f(std::initializer_list<double>(__b, __b+3));
}
```

What about when size is 5000, 500, 50, 5, 1?
Can we get rid of superfluous bracelets when size is 1?

**CON(s)**
- No size restrictions
- No static guarantee

---

## std::span over an initializer list (C++26)

```cpp
const double& dangler(const double& d) { return d; }
const double& dangler(std::span<const double, 1> s) { return dangler(s.front()); }

void h() {
    dangler({1});// practically safe
    //static constexpr double __b[3] = {double{1}}; // backing array
    //f(std::initializer_list<double>(__b, __b+1));
}
```

implicit vs. explicit
**CON(s)**
- library user must unwrap first member
- library writer must duplicate functions
- No static guarantee

---

## `std::constant_wrapper`

```cpp
std::cw<1>
std::cw<2uz>
std::cw<3.0>
std::cw<4.f>
std::cw<foo>
std::cw<my_complex(1.f, 1.f)>
```

explicit vs. implicit

**CON(s)**
- Only works on structural types

---

## `C` `constexpr static` (C23)

```c
&(static constexpr struct foo) {1, 'a', 'b'}
```

explicit vs. implicit

**CON(s)**
- Only works on C23
- `constexpr static` vs. constant or read_only
- "For the storage duration of the created objects we go with C++ for compatibility"

---

## constexpr structured bindings and references to constexpr variables

- **Allowing static and non-tuple constexpr structured binding**
- `Making constexpr implicitly static`
- Always re-evaluate a call to get?
- **Symbolic addressing**

**CON(s)**
- **Wording:** ~~or temporary object~~

---

## Better than Rust

```rust
const X: &'static T = &<constexpr foo>;
```

"... the only keyword that most Rust programmers should need to know is const -- I imagine static variables will be used quite rarely."

explicit vs. implicit

**CON(s)**
- not really a automatic promotion as it must be requested
- tool that must be used, not default

---

## References [^clambdamacro] [^p2752r3] [^p2447r6] [^p2781r4] [^n3018] [^n3038] [^p2686r3] [^rvalue_static_promotion] [^const-vs-static]

<!--C++ lambda macro to force argument type-->
[^clambdamacro]: <https://stackoverflow.com/questions/72988593/c-lambda-macro-to-force-argument-type>
<!--Static storage for braced initializers-->
[^p2752r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2752r3.html>
<!--std::span over an initializer list-->
[^p2447r6]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2447r6.html>
<!--std::constant_wrapper-->
[^p2781r4]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2781r4.html>
<!--The constexpr specifier for object definitions-->
[^n3018]: <https://open-std.org/JTC1/SC22/WG14/www/docs/n3018.htm>
<!--Introduce storage-class specifiers for compound literals-->
[^n3038]: <https://open-std.org/JTC1/SC22/WG14/www/docs/n3038.htm>
<!--constexpr structured bindings and references to constexpr variables-->
[^p2686r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2686r3.pdf>
<!--rvalue_static_promotion-->
[^rvalue_static_promotion]: <https://rust-lang.github.io/rfcs/1414-rvalue_static_promotion.html>
<!--const-vs-static-->
[^const-vs-static]: <https://github.com/rust-lang/rfcs/blob/master/text/0246-const-vs-static.md>
