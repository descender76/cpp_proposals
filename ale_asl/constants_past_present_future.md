---
type: slide
---

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

## embarrassing (Part 1)

Most if not all programming languages in use don't immediately dangle their constants/literals.

---

## embarrassing (Part 2)

### Assembly is easier and safer

Guaranteed static and const

```asm
     .globl a
    .section    .rodata
    .align 4
    .type   a, @object
    .size   a, 4
a:
    .long   5
```

Decomposed constant in assembly instruction

```asm
mov <register>,<constant>
mov <memory>,<constant>
```

---

## embarrassing (Part 3)

### C is easier and safer
<!--
`2021/10/18 Meneide, C Working Draft` (n2731)
-->
*"6.5.2.5 Compound literals"* **&#182; 5**

*"The value of the compound literal is that of an unnamed object initialized by the initializer list. If the compound literal occurs outside the body of a function, the object has static storage duration; otherwise, it has **automatic storage duration associated with the enclosing block**."*

---

## embarrassing (Part 4)

### C++ was easier and safer

CFront i.e. C with classes because C++ was preprocessed to C code which didn't immediately dangle their constants/literals.

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

**CON(s)** (explicit)
- How many times must we say const?
- Compilers must undue boilerplate

---

## Static storage for braced initializers (C++26) (p2752r3)

```cpp
void f(std::initializer_list<double> il);
void h() {
    f({1, 2, 3});
    //static constexpr double __b[3] = {double{1}, double{2}, double{3}}; // backing array
    //f(std::initializer_list<double>(__b, __b+3));
}
```

When size is 5000, 500, 50, 5, 1?
Superfluous bracelets when size is 1?

**CON(s)** (explicit)
- No size restrictions
- No static guarantee

---

## std::span over an initializer list (C++26) (p2447r6)

```cpp
const double& dangler(const double& d) { return d; }
const double& dangler(std::span<const double, 1> s)
    { return dangler(s.front()); }
void h() {
    dangler({1});// practically safe
    //static constexpr double __b[3] = {double{1}}; // backing array
    //f(std::initializer_list<double>(__b, __b+1));
}
```

**CON(s)** (explicit)
- library user must wrap in initializer list
- library writer must duplicate functions
- No static guarantee

---

## `std::constant_wrapper` (p2781r4)

```cpp
std::cw<1>
std::cw<2uz>
std::cw<3.0>
std::cw<4.f>
std::cw<foo>
std::cw<my_complex(1.f, 1.f)>
```

**CON(s)** (explicit)
- Only works on structural types

---

## `C` `constexpr static` (C23)

```c
&(static constexpr struct foo) {1, 'a', 'b'}
```

PRO(s)
- Best explicit syntax; better if one keyword

**CON(s)** (explicit)
- Only works on C23
- `constexpr static` vs. constant or read_only
- "For the storage duration of the created objects we go with C++ for compatibility"

---

## constexpr structured bindings and references to constexpr variables (p2686r3)

- **Allowing static and non-tuple constexpr structured binding**
- `Making constexpr implicitly static`
- ~~Always re-evaluate a call to get?~~
- **Symbolic addressing**

**CON(s)**
- **Wording:** ~~or temporary object~~

---

## Better than Rust?

```rust
// rvalue_static_promotion
const X: &'static T = &<constexpr foo>;
```

"... the only keyword that most Rust programmers should need to know is const -- I imagine static variables will be used quite rarely."

**CON(s)** (explicit)
- not really a automatic promotion as it must be requested
- like C++, a tool that must be used, not default
<!--
---

## References [^n2731] [^clambdamacro] [^p2752r3] [^p2447r6] [^p2781r4] [^n3018] [^n3038] [^p2686r3] [^rvalue_static_promotion] [^const-vs-static]-->

<!--2021/10/18 Meneide, C Working Draft-->
[^n2731]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2731.pdf>
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
