<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3810R1</td>
</tr>
<tr>
<td>Date</td>
<td>2025-10-03</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>SG23 Safety and Security</td>
</tr>
</table>

# hardened memory safety guarantees

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

- [hardened memory safety guarantees](#hardened-memory-safety-guarantees)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Wording](#Wording)
  - [Impact on the standard](#Impact-on-the-standard)
  - [References](#References)

## Changelog

### R1

- fixed copy paste errors

## Abstract

The `C++` standard could say that all of its hardening is itself free of memory safety related undefined behavior if it required that STL implementations make the following functions free of memory safety related undefined behavior: `size()`, `empty()`, iterator difference, `static_extent()`, `extent()`, `has_value()`, `valueless_by_exception()`, `holds_alternative<I>()`.

Following is hopefully a unique list of the hardened contract assertions currently proposed for the `C++26` standard.

### `Standard Library Hardening` [^p3471r4]

```cpp
idx < size() is true
empty() is false
Count <= size() is true
count <= size() is true
Offset <= size() && (Count == dynamic_extent || Count <= size() - Offset) is true
offset <= size() && (count == dynamic_extent || count <= size() - offset) is true
count == extent is true
(last - first) == extent is true
ranges::size(r) == extent is true
il.size() == extent is true
s.size() == extent is true
pos < size() is true
empty() is false
n <= size() is true
a.empty() is false
n < a.size() is true
static_extent(r) == dynamic_extent || static_extent(r) == other.extent(r) is true
n < size() is true
has_value() is true
has_value() is false
```

### `Minor additions to C++26 standard library hardening` [^p3697r0]

```cpp
i >= 0
i < N
!empty() is true
length > 0 is true
n < length is true
i.length > 0 is true
x.length > 0 and y.length > 0 are true
n >= 0
n <= length is true
-n <= length is true
x.v_.valueless_by_exception() is false
holds_alternative<I>(v_) is true
x.v_.valueless_by_exception() and y.v_.valueless_by_exception() are each false
skip <= skip + max_depth is true
frame_no < size()
```

Is it unreasonable to require a trivial to write member function to be written free of memory safety related undefined behavior? Consider for example the two most common ways to write a size member function.

```cpp
class something
{
private:
    size_t s;
public:
    constexpr size_t size() const
    {
        return s; 
    }
};
```

```cpp
class something
{
private:
    iterator start;
    iterator end;
public:
    constexpr size_t size() const
    {
        return end - start; 
    }
};
```

Both of these common implementations are free of uninitialized, range access, null pointer dereference and use after free memory errors. Neither is their type safety errors. So requiring something that STL implementors are already doing does not seem unreasonable. It is similar for the other value semantic functions that simply return copies of members of the class in question.

## Motivation

There has been sustained interest in contracts being free of undefined behavior. This was a part of the `Contracts — Use Cases` [^crit.noundef] [^crit.noassume] back in 2020. There has been a lot of interest since.

2022 `Contracts for C++: Prioritizing Safety ` [^p2680r1]
2024 `P2900R6 May Be Minimal, but It Is Not
Viable` [^p3173r0]
2024 `Contracts: Protecting The Protector` [^p3285r0]
2024 `Static analysis and 'safety' of Contracts, P2900 vs. P2680/P3285` [^p3362r0]
2025 `Contract concerns` [^p3573r0]

Most every day programmers are more specifically concerned about the memory unsafety subset of undefined behavior. While we can't at this moment mandate that contracts are free of memory unsafety, can we at least offer the guarantee that the standard by default won't deliberately create hardened contracts with memory safety related undefined behavior without a very good reason for doing so.

## Impact on the standard

Safety not only requires safety to be in the language but also a part of the libraries. This proposal provides safety guarantees in some portions of the standard by requiring STL implementers to make some of their functions safe.

## References
<!-- Standard library hardening -->
[^p3471r4]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3471r4.html>
<!-- Minor additions to C++26 standard library hardening -->
[^p3697r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3697r0.html>
<!-- Working Draft, Programming Languages -- C++ -->
[^n5008]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/n5008.pdf>
<!-- Contracts — Use Cases -->
[^crit.noundef]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1995r1.html#crit.noundef>
<!-- Contracts — Use Cases -->
[^crit.noassume]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1995r1.html#crit.noassume>
<!-- Contracts for C++: Prioritizing Safety -->
[^p2680r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2680r1.pdf>
<!-- P2900R6 May Be Minimal, but It Is Not
Viable -->
[^p3173r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3173r0.pdf>
<!-- Contracts: Protecting The Protector -->
[^p3285r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3285r0.pdf>
<!-- Static analysis and 'safety' of Contracts, P2900 vs. P2680/P3285 -->
[^p3362r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3362r0.html>
<!-- Contract concerns -->
[^p3573r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3573r0.pdf>
