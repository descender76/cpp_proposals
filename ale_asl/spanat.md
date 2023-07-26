<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2821R4</td>
</tr>
<tr>
<td>Date</td>
<td>2023-7-26</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Library Evolution Working Group (LEWG)<br/>SG23 Safety and Security</td>
</tr>
</table>

# span.at()

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

- [span.at()](#spanat)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Request](#Request)
  - [Motivation](#Motivation)
    - [Safety](#Safety)
    - [Consistency](#Consistency)
    - [Public Relations](#Public-Relations)
  - [Wording](#Wording)
  - [Feature test macro](#Feature-test-macro)
  - [Implementation Experience](#Implementation-Experience)
  - [Summary](#Summary)
  - [References](#References)
<!--
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
-->

## Changelog

### R4

- Added a wording section

### R3

- Fixed some typos

### R2

- Corrected `Feature test macro`
- Added references to `Usability Enhancements for std::span` [^p1024r0] [^p1024r1]

### R1

- Added verbiage stating that this is not freestanding due to its throwing an exception
- Added feature test macro section

## Abstract

This paper proposes the standard adds the `at` method to `std::span` class in order to address safety, consistency and PR (public relations) concerns.

## Request

```cpp
// no freestanding
#if __STDC_HOSTED__ == 1
constexpr reference at(size_type idx) const;
#endif
```

Returns a reference to the element at specified location idx, with bounds checking.

If idx is not within the range of the container, an exception of type std::out_of_range is thrown.

**Parameters**
idx	-	index of the element to return

**Return value**
Reference to the requested element.

**Exceptions**
Throws std::out_of_range if idx >= size().

**Complexity**
Constant.

## Motivation

### Safety

This new method is safe in the sense that it has defined behavior instead of undefined behavior. Further, the defined behavior is one that can be caught in the code by catching the exception.

### Consistency

The `std::string` [^stringat], `std::string_view` [^stringviewat], `std::deque` [^dequeat], `std::vector` [^vectorat] and `std::array` [^arrayat], all have both the unsafe `operator[]` and the safe `at` function.

<table>
<tr>
<td>
</td>
<td>

`operator[]`

</td>
<td>

`at()`

</td>
</tr>
<tr>
<td>

`std::string`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
</tr>
<tr>
<td>

`std::string_view`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
</tr>
<tr>
<td>

`std::deque`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
</tr>
<tr>
<td>

`std::vector`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
</tr>
<tr>
<td>

`std::array`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
</tr>
<tr>
<td>

`std::span`

</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:red;text-align:center">&#x2717;</td>
</tr>
</table>

### Public Relations

`C++` keeps giving easy wins to our rivals even though they like to hit below the belt. It is nice when a supporter of a rival language makes an honest comparison.

*"In both Rust and C++, there is a method for checked array indexing, and a method for unchecked array indexing. The languages actually agree on this issue. They only disagree about which version gets to be spelled with brackets."* - **Being Fair about Memory Safety and Performance** [^fair]

Unfortunately this isn't totally true.

*"`std::span<T>` **indexing**"* [^modern]

...

*"`std::vector` and `std::array` can at least theoretically be used safely because they offer an `at()` method which is bounds checked (in practice I’ve never seen this done, but you could imagine a project adopting a static analysis tool which simply banned calls to `std::vector<T>::operator[]`). `span` does not offer an `at()` method, or any other method which performs a bounds checked lookup."* [^modern]

*"Interestingly, both Firefox and Chromium’s backports of std::span do perform bounds checks in operator[], and thus they’ll never be able to safely migrate to std::span."* - **Modern C++ Won't Save Us** [^modern]

Consequently, the programming community in general are encouraged to ask some tough questions.

1. Why was `span::at()` not provided in `C++20`? [^p1024r0]
1. Was this an accidental omission or was it deliberate? [^p1024r1]
1. If deliberate, what was the rationale?
1. If deliberate, are the other `at` functions going to be deprecated?
1. Why was `span::at()` not added in `C++23`?
1. Is it going to be in `C++26`?

Ultimately, this becomes a stereotypical example of how `C++` traditionally handles safety. This example gets to be pointed at for years/decades to come. All of this could have been avoided, along with more effort of adding this function individually had more consideration been given valid safety concerns.

## Wording

The wording is relative to N4950 [^n4950].

<!--
17.3.2 Header <version> synopsis [version.syn]
-->

> Insert the following into [[span.overview]](https://eel.is/c++draft/span.overview), header `<span>` synopsis within the span class, immediately after the subscript operator in the element access section:
<!--
24.7.2.2.1 Overview [span.overview]

class span {

// 24.7.2.2.6, element access
constexpr reference operator[](size_type idx) const;
-->
```cpp
// no freestanding
#if __STDC_HOSTED__ == 1
constexpr reference at(size_type idx) const;
#endif
```

> Insert the following into [[span.elem]](https://eel.is/c++draft/span.elem), span's `Element access` section, immediately after the subscript operator:
<!--
24.7.2.2.6 Element access [span.elem]
constexpr reference operator[](size_type idx) const;
1 Preconditions: idx < size() is true.
2 Effects: Equivalent to: return *(data() + idx);
-->
> constexpr reference at(size_type idx) const;
>
> *Returns*: *(data() + idx).
>
> *Throws*: out_of_range if idx >= size().


## Feature test macro

> Insert the following to [[version.syn]](https://eel.is/c++draft/version.syn), header `<version>` synopsis:
<!--
<pre>
#define __cpp_lib_span_at 20XXXXL <i>// also in &lt;functional&gt</i>
</pre>
-->
```cpp
#define __cpp_lib_span_at 20XXXXL // also in <span>
```

## Implementation Experience

Both of the `span lite` and `Guidelines Support Library` libraries have this new method implemented for years.

- `span lite: A single-file header-only version of a C++20-like span for C++98, C++11 and later` [^spanlite]
- `Guidelines Support Library` [^gslspan]

Likely, there are others too such as *"Firefox and Chromium’s backports of std::span"*. [^modern]

## Summary

Please add the `at` method to `std::span` class in order to address safety, consistency and PR (public relations) concerns. Also keep ones mind open to an `std::expected` version in the future as well as other reasonable safety requests.

<!--
## Frequently Asked Questions
-->
## References

<!--Re: function at() for span<> that throws exception on out-of-range, like vector and array ::at()-->
[^lsp2019110757]: <https://lists.isocpp.org/std-proposals/2019/11/0757.php>
<!--std::span and the missing constructor ... immediate dangling-->
[^p2447r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2447r1.html>
<!--span lite: A single-file header-only version of a C++20-like span for C++98, C++11 and later-->
[^spanlite]: <https://github.com/martinmoene/span-lite#at>
<!--Re: std::span-->
[^lsp2020041303]: <https://lists.isocpp.org/std-proposals/2020/04/1303.php>
<!--std::span implementation for C++11 and later-->
[^span11]: <https://github.com/tcbrindle/span>
<!--Guidelines Support Library-->
[^gslspan]: <https://github.com/microsoft/GSL/blob/main/include/gsl/span_ext#L141>
<!--std::out_of_range-->
[^out_of_range]: <https://en.cppreference.com/w/cpp/error/out_of_range>
<!--std::string::at-->
[^stringat]: <https://en.cppreference.com/w/cpp/string/basic_string/at>
<!--std::string::operator[]-->
[^stringindexer]: <https://en.cppreference.com/w/cpp/string/basic_string/operator_at>
<!--std::string_view::at-->
[^stringviewat]: <https://en.cppreference.com/w/cpp/string/basic_string_view/at>
<!--std::string_view::operator[]-->
[^stringviewindexer]: <https://en.cppreference.com/w/cpp/string/basic_string_view/operator_at>
<!--std::deque::at-->
[^dequeat]: <https://en.cppreference.com/w/cpp/container/deque/at>
<!--std::deque::operator[]-->
[^dequeindexer]: <https://en.cppreference.com/w/cpp/container/deque/operator_at>
<!--std::vector::at-->
[^vectorat]: <https://en.cppreference.com/w/cpp/container/vector/at>
<!--std::vector::operator[]-->
[^vectorindexer]: <https://en.cppreference.com/w/cpp/container/vector/operator_at>
<!--std::array::at-->
[^arrayat]: <https://en.cppreference.com/w/cpp/container/array/at>
<!--std::array::operator[]-->
[^arrayindexer]: <https://en.cppreference.com/w/cpp/container/array/operator_at>
<!--std::span::operator[]-->
[^spanindexer]: <https://en.cppreference.com/w/cpp/container/span/operator_at>
<!--Being Fair about Memory Safety and Performance-->
[^fair]: <https://www.thecodedmessage.com/posts/unsafe/>
<!--Modern C++ Won't Save Us-->
[^modern]: <https://alexgaynor.net/2019/apr/21/modern-c++-wont-save-us/>
<!--Usability Enhancements for std::span-->
[^p1024r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1024r0.pdf>
<!--Usability Enhancements for std::span-->
[^p1024r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1024r1.pdf>
<!--Working Draft, Standard for Programming Language C++-->
[^n4950]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf>
<!--inplace_vector-->
[^p0843r8]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0843r8.html>
<!--function_ref: a type-erased callable reference-->
[^p0792r14]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0792r14.html>
<!--MDSPAN-->
[^p0009r18]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0009r18.html>
<!--TODO-->
[^TODO]: <TODO>
