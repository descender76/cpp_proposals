<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3739R4</td>
</tr>
<tr>
<td>Date</td>
<td>2025-11-06</td>
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
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Changelog

### R4

- realign references from N5008 to N5014 [^n5014]
- added more wording

### R3

- added more context to historical discussion
- added `Frequently Asked Questions` section

### R2

- added  `Inspecting exception_ptr` [^p2927r3] [^P3748R0]
- added `define_static_{string,object,array}` [^p3491r3]
- added more `const`
- added more formatting

## Abstract

Utilize `std::optional<T&>` [^p2988r12] and `T&` in the `C++26` standard instead of `T*` in order to harden it against null pointer dereference errors for similar memory safety reasons as `Standard Library Hardening` [^p3471r4] and `Minor additions to C++26 standard library hardening` [^p3697r0].

This was discussed during the March 2024 WG21 meeting in Tokyo, Japan at the Library Working Group during the standardization of `inplace_vector` [^p0843r11] but could not be adopted since `std::optional<T&>` [^p2988r12] was not sufficiently along in the `C++26` standardization process. Now that `std::optional<T&>` [^p2988r12] was adopted during the June 2025 Sophia meeting, it would be ideal that `inplace_vector` [^p0843r14] and other superfluous instances of `T*` underwent minor revisions to better align it with the rest of `C++26` and consequently provide a more robust interface.

<!--
https://wiki.edg.com/bin/view/Wg21tokyo2024/WebHome
LWG Tokyo Notes
https://docs.google.com/document/d/1IRA71rAastDNtyQLpa-wr3eUBRH8q4be90fyvZ5nGmY/edit?tab=t.0
-->

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

#### 17.9.2 Header <exception> synopsis [exception.syn]

...

template<class E> constexpr ~~const E*~~++const optional<const T&>++ exception_ptr_cast(const exception_ptr& p) noexcept;

...

#### 17.9.7 Exception propagation [propagation]

template<class E> constexpr ~~const E*~~++const optional<const T&>++ exception_ptr_cast(const exception_ptr& p) noexcept;

...

#### 21.4.1 Header <meta> synopsis [meta.syn]

...

template<class T>
consteval const ~~remove_cvref_t<T>*~~++T&++ define_static_object(T&& t);

...

#### 21.4.3 Promoting to static storage strings [meta.define.static]

...

```cpp
template<class T>
```
`consteval const`
~~remove_cvref_t<T>*~~++T&++ `define_static_object(T&& t);`<br/>
<sup>4</sup> <i>Effects</i>: Equivalent to:<br/>
```cpp
using U = remove_cvref_t<T>;
if constexpr (is_class_type(^^U)) {
```
&nbsp;&nbsp;&nbsp;&nbsp;`return` <span style="background-color:lightgreen">\*(</span>`addressof(extract<const U&>(meta::reflect_constant(std::forward<T>(t))))`<span style="background-color:lightgreen">)</span>`;`<br/>
```cpp
} else {
```
&nbsp;&nbsp;&nbsp;&nbsp;`return` <span style="background-color:lightgreen">\*(</span>`define_static_array(span(addressof(t), 1)).data()`<span style="background-color:lightgreen">)</span>`;`<br/>
```cpp
}
```
<br/>
    
<sup>5</sup> \[*Note 1*: For class types, **define_static_object** provides the ~~address~~++reference++ of the template parameter object (13.2)<br/>that is template-argument equivalent to t. — *end note*\]

...

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

The proposed changes are relative to the current working draft `N5014` [^n5014].

## Frequently Asked Questions

### Wasn't the return type of `try_push_back` and `try_emplace_back` settled in R6 and R7?

According to `NB-Commenting is Not a Vehicle for Redesigning inplace_vector`, [^p3830r0] `R6` [^p0843r6] discussed `optional<T>`, not `optional<T&>`. According to `NB-Commenting is Not a Vehicle for Redesigning inplace_vector`, [^p3830r0] `R7` [^p0843r7] discussed bool and pointer but not `optional<T&>`. Using `optional<T&>` was discussed during the March 2024 WG21 meeting in Tokyo, Japan at the Library Working Group during the consideration of `R11` [^p0843r11]. It was known that optional<&> was coming at some point and when it does that it would need to be considered revising the return parameters of `try_push_back` and `try_emplace_back`. `optional<T&>` did come and it does need to be considered on both safety and on internal `C++26` consistency grounds.

### Why `const <optional&>`?

Either `const <optional&>` or `<optional&>` will mitigate the spatial memory errors such as null pointer dereference, pointer arithmetic and pointer indexing associated with superfluous pointer usage. So removing `const` doesn't negate the safety advantages of providing `<optional&>`. So why `const`? We are talking about references which essentially are `const` pointers. Consequently, `const <optional&>` should be the default usage when one is expecting reference semantics. Why do many programmers prefer reference semantics over pointer semantics! Besides the spatial safety benefits already mentioned, reference semantics also helps temporal safety because once a reference aliases an object with global, stack or heap lifetime it always does so, which makes it easier for programmers to reason about lifetimes at compile time. Due to the mutability of non `const` pointers, reasoning about lifetimes are runtime in nature. Further, no functionality is lost by having reference semantics because `const` optional<&> is implicitly convertible to optional<&> just as `const` pointers are implicitly convertible to non `const` pointers.

```cpp
int value = 0;
```

<table>
<tr>
<td>

```cpp=
int * p00 = &value;
int const * p10 = &value;
int * const p01 = &value;
int const * const p11 = &value;
p00 = p00;
//p00 = p10;
p00 = p01;
//p00 = p11;
p10 = p00;
p10 = p10;
p10 = p01;
p10 = p11;
//p01 = p00;
//p01 = p10;
//p01 = p01;
//p01 = p11;
//p11 = p00;
//p11 = p10;
//p11 = p01;
//p11 = p11;
```

</td>
<td>

```cpp=
optional<int&> op00{value};
optional<const int&> op10{value};
const optional<int> op01{value};
const optional<const int> op11{value};
op00 = op00;
//op00 = op10;
op00 = op01;
//op00 = op11;
op10 = op00;
op10 = op10;
op10 = op01;
op10 = op11;
//op01 = op00;
//op01 = op10;
//op01 = op01;
//op01 = op11;
//op11 = op00;
//op11 = op10;
//op11 = op01;
//op11 = op11;
```

</td>
</tr>
</table>

### Doesn't `optional<T&>` have issues?

Sure and programmers are raising safety issues with inplace_vector<T>. National Body Comments is about fixing issues.

`LWG4299` [^LWG4299] and `LWG4300` [^LWG4300] are tiny clarifications.
`LWG4299` [^LWG4299], `LWG4300` [^LWG4300] and `LWG4300` [^LWG4300] does not impact the public interface of `optional<&>` nor does it have safety impacts. Further these issues could be fixed in `C++29` without it being a breaking change. Waiting till `C++29` to correct the return type of `try_emplace_back` and `try_push_back` would be a breaking change, which is why it needs to be addressed now. 

### Isn't optional<T&> NOT trivially copyable?

That is true. While that should be fixed now, it is not a breaking change if it was fixed early in `C++29`. There is even a paper that requests this, `Make optional<T&> trivially copyable` [^p3836r0]. What would be even better if the second design decision was chosen; "Additionally constrain its layout so it actually becomes a wrapper for a T*". [^p3836r0] This could be done simply by removing "// exposition only" and by adding a constructor from a pointer to the `optional<&>` [^p2988r12] proposal. This would allow programmers to opt into optional safety when working with legacy API(s) that are still superfluously using pointers, thus improving the safety benefits of `C++26` further via minimal corrections.

### Shouldn't `try_*_back` mostly only be used in boolean context?

According to `NB-Commenting is Not a Vehicle for Redesigning inplace_vector` [^p3830r0], yes! `optional<&>` [^p2988r12] was specifically designed to be used in a boolean context as seen by the multiple examples in the comparison table section at the start of that proposal.

### Shouldn’t we not be adding any more overhead to an object that is likely to be used briefly as a temporary and thrown away?

YES, that is why `optional<&>` [^p2988r12] should eventually be made `trivially copyable` [^p3836r0] and more important "additionally constrain its layout so it actually becomes a wrapper for a T*". [^p3836r0]

### Why `const T&` instead of `const T*`?

If the referent is non null, as it should be when dealing with objects of both static storage duration and automatic storage duration, then a reference should be used instead of a pointer. Beside that, `const T&` is ready to be consumed as input arguments.

<table>
<tr>
<td>

**C++ Core Guidelines**
***F.16: For "in" parameters, pass** cheaply-copied types by value and others **by reference to const*** [^cppcgrfin]

</td>
</tr>
</table>

We can have functions like `define_static_object` dereference the pointer a constant number of times, once per return, or we can have all programmers dereference the return every time the function is used as an input argument. In other words, a very large number that grows with time. The default choice seems clear.

## References
<!-- C++ Core Guidelines -->
[^CppCoreGuidelines]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
<!-- inplace_vector -->
[^p0843r6]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0843r6.html>
<!-- inplace_vector -->
[^p0843r7]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0843r7.html>
<!-- inplace_vector -->
[^p0843r11]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r11.html>
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
[^n5014]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/n5014.pdf>
<!-- Inspecting exception_ptr -->
[^p2927r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2927r3.html>
<!-- Inspecting exception_ptr should be constexpr -->
[^P3748R0]: <https://wiki.edg.com/pub/Wg21sofia2025/StrawPolls/P3748R0.html>
<!-- define_static_{string,object,array} -->
[^p3491r3]: <https://wiki.edg.com/pub/Wg21sofia2025/StrawPolls/p3491r3.html>
<!-- NB-Commenting is Not a Vehicle for Redesigning inplace_vector -->
[^p3830r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3830r0.pdf>
<!-- Make optional<T&> trivially copyable -->
[^p3836r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3836r0.html>
<!-- Missing Mandates: part in optional<T&>::transform -->
[^LWG4299]: <https://cplusplus.github.io/LWG/issue4299>
<!-- Missing Returns: element in optional<T&>::emplace -->
[^LWG4300]: <https://cplusplus.github.io/LWG/issue4300>
<!-- std::optional<T&>::iterator can't be a contiguous iterator for some T -->
[^LWG4308]: <https://cplusplus.github.io/LWG/issue4308>
<!-- F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const -->
[^cppcgrfin]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-in>
