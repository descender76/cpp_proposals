<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3739R2</td>
</tr>
<tr>
<td>Date</td>
<td>2025-08-10</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Library Working Group (LWG)</td>
</tr>
</table>

# Standard Library Hardening - using std::optional<T&>

<style>
.inline-link
{
    font-size: small;
    margin-top: -2.8em;
    margin-right: 4px;
    text-align: right;
    font-weight: bold;
}

code
{
    font-family: "Fira Code", monospace !important;
    font-size: 0.87em;
}

.sourceCode
{
    font-size: 0.95em;
}

a code
{
    color: #0645ad;
}
</style>

## Table of contents

- [Standard Library Hardening - using std::optional<T&>](#Standard-Library-Hardening-using-std-optional-T)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Wording](#Wording)
  - [Impact on the standard](#Impact-on-the-standard)
  - [References](#References)

## Changelog

### R2

- added  `Inspecting exception_ptr` [^p2927r3] [^P3748R0]
- added `define_static_{string,object,array}` [^p3491r3]
- added more `const`
- added more formatting

## Abstract

Utilize `std::optional<T&>` [^p2988r12] and `T&` in the `C++26` standard instead of `T*`  order to harden it against null pointer dereference errors for similar memory safety reasons as `Standard Library Hardening` [^p3471r4] and `Minor additions to C++26 standard library hardening` [^p3697r0].

This was discussed during the standardization of `inplace_vector` [^p0843r14] but could not be adopted since `std::optional<T&>` [^p2988r12] was not sufficiently along in the `C++26` standardization process. Now that `std::optional<T&>` [^p2988r12] was adopted during the June 2025 Sophia meeting, it would be ideal that `inplace_vector` [^p0843r14] and other superfluous instances of `T*` underwent minor revisions to better align it with the rest of `C++26` and consequently provide a more robust interface.

The first request is that 3 of `inplace_vector's` [^p0843r14] modifiers be revised to return a `const std::optional<T&>` [^p2988r12] instead of a pointer.

<table>
<tr>
<td>

template< class... Args >
constexpr ~~pointer~~++const optional<T&>++ try_emplace_back( Args&&... args );

constexpr ~~pointer~~++const optional<T&>++ try_push_back( const T& value );

constexpr ~~pointer~~++const optional<T&>++ try_push_back( T&& value );

</td>
</tr>
</table>

The second request is that `Inspecting exception_ptr` [^p2927r3] [^P3748R0] be similarly revised.

<table>
<tr>
<td>

template <class E>
constexpr ~~const E*~~++const optional<const T&>++ exception_ptr_cast(const exception_ptr& p) noexcept

</td>
</tr>
</table>

The third request is that `define_static_{string,object,array}` [^p3491r3] be similarly revised.

<table>
<tr>
<td>

template <class T><br/>
consteval auto define_static_object(T&& v) -> ~~remove_reference_t<T> const*~~++const T&++;

</td>
</tr>
</table>

## Motivation

Returning a pointer from these three methods are less than an ideal interface. The primary problem is that it leads to null pointer dereference errors. It is essentially a 100% unsafe interface. Now contrast that with the different iterations of `optional`.

While `optional<T>` can't be used as an alternative to pointer, in `C++17` when it was standardized, `optional` had an equal number of unsafe and safe modifiers. The unsafe ones were `operator->` and `operator*` and the safe modifiers were `value` and `value_or`. In other words, `optional` was born with 50% null pointer dereference safety. 

In `C++23`, the `and_then`, `transform` and `or_else` modifiers were added. This brought the total of modifiers to 5 of 7. The result is that 71% of all modifiers are safe from null pointer dereference errors.

However, only in `C++26` with the advent of `std::optional<T&>` [^p2988r12] does `optional` becomes a viable substitute for a nullable pointer. Also in this release is `std::optional range support` [^p3168r2] which turns `optional` is a range of 0 or 1. This opens the door for 125 range algorithms and 34 range adapters to become null pointer deference safe modifiers for `optional`. Those range algorithms and adapters are growing at a much faster rate than new modifiers being added to the `optional` class itself. This brings the modifer safety rating up to 98%. The original 2 unsafe modifiers and hence the remaining 2% are being addressed by `Standard Library Hardening` [^p3471r4].

Besides the safety benefit, the monadic and range usage are more ergonometric than the legacy pointer based interface.

There are equivalent benefits for `exception_ptr_cast` [^p2927r3] [^P3748R0] and `define_static_object` [^p3491r3].

## Wording

### 23.3.16 Class template inplace_vector [inplace.vector]


#### 23.3.16.1 Overview [inplace.vector.overview]

...

(5.3.3) — If

...

```cpp
namespace std {
template<class T, size_t N>
class inplace_vector {
public:
```

...

template< class... Args ><br/>
constexpr ~~pointer~~++const optional<T&>++ try_emplace_back( Args&&... args );

constexpr ~~pointer~~++const optional<T&>++ try_push_back( const T& value );

constexpr ~~pointer~~++const optional<T&>++ try_push_back( T&& value );

...

```cpp
};
}
```

...

#### 23.3.16.5 Modifiers [inplace.vector.modifiers]

...

template<class... Args><br/>
constexpr ~~pointer~~++const optional<T&>++  try_emplace_back(Args&&... args);<br/>
constexpr ~~pointer~~++const optional<T&>++  try_push_back(const T& x);<br/>
constexpr ~~pointer~~++const optional<T&>++  try_push_back(T&& x);<br/>
<sup>8</sup> Let `vals` denote a pack:<br/>
<sup>(8.1)</sup> — `std::forward<Args>(args)...` for the first overload,<br/>
<sup>(8.2)</sup> — `x` for the second overload,<br/>
<sup>(8.3)</sup> — `std::move(x)` for the third overload.<br/>
<sup>9</sup> *Preconditions*: `value_type` is `Cpp17EmplaceConstructible` into `inplace_vector` from `vals....`<br/>
<sup>10</sup> *Effects*: If `size() < capacity()` is `true`, appends an object of type `T` direct-non-list-initialized with `vals....` Otherwise, there are no effects.<br/>
<sup>11</sup> *Returns*: `nullptr` if `size() == capacity()` is `true`, otherwise `addressof(back())`.<br/>
<sup>12</sup> *Throws*: Nothing unless an exception is thrown by the initialization of the inserted element.<br/>
<sup>13</sup> *Complexity*: Constant.<br/>
<sup>14</sup> *Remarks*: If an exception is thrown, there are no effects on `*this`.

## Impact on the standard

Other than `inplace_vector`[^p0843r14], `Inspecting exception_ptr` [^p2927r3] [^P3748R0] and `define_static_{string,object,array}` [^p3491r3] which are all three completely new to the standard, there are no other changes to the library standard and since `C++26` has not be released, this tweak is not expected to cause any problems.

The proposed changes are relative to the current working draft `N5008` [^n5008].

## References
<!-- C++ Core Guidelines -->
[^CppCoreGuidelines]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
<!-- inplace_vector -->
[^p0843r14]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html>
<!-- Minor additions to C++26 standard library hardening -->
[^p3697r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3697r0.html>
<!-- std::optional<T&> -->
[^p2988r12]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2988r12.pdf>
<!-- std::optional range support -->
[^p3168r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3168r2.html>
<!-- Standard Library Hardening -->
[^p3471r4]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3471r4.html>
<!-- Working Draft, Programming Languages -- C++ -->
[^n5008]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/n5008.pdf>
<!-- Inspecting exception_ptr -->
[^p2927r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2927r3.html>
<!-- Inspecting exception_ptr should be constexpr -->
[^P3748R0]: <https://wiki.edg.com/pub/Wg21sofia2025/StrawPolls/P3748R0.html>
<!-- define_static_{string,object,array} -->
[^p3491r3]: <https://wiki.edg.com/pub/Wg21sofia2025/StrawPolls/p3491r3.html>
