<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3811R0</td>
</tr>
<tr>
<td>Date</td>
<td>2025-08-15</td>
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

# default comparison memory safety

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

- [default comparison memory safety](#default-comparison-memory-safety)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Wording](#Wording)
  - [Impact on the standard](#Impact-on-the-standard)
  - [References](#References)

## Abstract

The `C++` standard should require compilers to implement `defaulted comparison operator functions` free of memory safety related undefined behavior and this guarantee should be stated in the standard.

Is it unreasonable to require compilers to implement `defaulted comparison operator functions` free of memory safety related undefined behavior? Consider the sample generated examples from the proposals that created the `defaulted comparison operator functions` language feature.

### `Consistent comparison` [^p0515r3]

```cpp
class Point {
 int x;
 int y;
public:
 friend bool operator==(const Point& a, const Point& b)
 {
     return a.x == b.x && a.y == b.y;
 }
 friend bool operator< (const Point& a, const Point& b)
 {
     return a.x < b.x || (a.x == b.x && a.y < b.y);
 }
 friend bool operator!=(const Point& a, const Point& b)
 {
     return !(a==b);
 }
 friend bool operator<=(const Point& a, const Point& b)
 {
     return !(b<a);
 }
 friend bool operator> (const Point& a, const Point& b)
 {
     return b<a;
 }
 friend bool operator>=(const Point& a, const Point& b)
 {
     return !(a<b);
 }
 // ... non-comparison functions ...
};
```

```cpp
class TotallyOrdered : Base {
    string tax_id;
    string first_name;
    string last_name;
public:
    std::strong_ordering operator<=>(const TotallyOrdered& that) const {
        if (auto cmp = (Base&)(*this) <=> (Base&)that; cmp != 0) return cmp;
        if (auto cmp = last_name <=> that.last_name; cmp != 0) return cmp;
        if (auto cmp = first_name <=> that.first_name; cmp != 0) return cmp;
        return tax_id <=> that.tax_id;
    }
    // ... non-comparison functions ...
};

```

### `<=> != ==` [^p1185r2]

```cpp
struct X {
    A a;
    B b;
    C c;

    ??? operator<=>(X const& rhs) const {
        if (auto cmp = a <=> rhs.a; cmp != 0)
            return cmp;
        if (auto cmp = b <=> rhs.b; cmp != 0)
            return cmp;
        return c <=> rhs.c;
    }

    bool operator==(X const& rhs) const {
        return a == rhs.a &&
            b == rhs.b &&
            c == rhs.c;
    }

    bool operator!=(X const& rhs) const {
        return !(*this == rhs);
    }
};
```

### `When do you actually use <=>?` [^p1186r3]

```cpp
struct Aggr {
    int i;
    char c;
    Legacy q;

    strong_ordering operator<=>(Aggr const& rhs) const {
        // pairwise <=> works fine for these
        if (auto cmp = i <=> rhs.i; cmp != 0) return cmp;
        if (auto cmp = c <=> rhs.c; cmp != 0) return cmp;

        // synthesizing strong_ordering from == and <
        if (q == rhs.q) return strong_ordering::equal;
        if (q < rhs.q) return strong_ordering::less;

        // sanitizers might also check for
        [[ assert: rhs.q < q; ]]
        return strong_ordering::greater;
    }
};
```

How do these examples rank with respect to being free of memory safety related undefined behavior?

<table>
<tr>
<td colspan="2">

`memory safety`

</td>
</tr>
<tr>
<td>&#128504;</td>
<td>uninitialized memory</td>
</tr>
<tr>
<td>&#128504;</td>
<td>range access</td>
</tr>
<tr>
<td>&#128504;</td>
<td>null pointer dereference</td>
</tr>
<tr>
<td>&#128504;</td>
<td>dangling</td>
</tr>
<tr>
<td>&#128504;</td>
<td>invalidation</td>
</tr>
<tr>
<td>&#128504;</td>
<td>type safety</td>
</tr>
</table>

## Motivation

There has been sustained interest in creating islands of safety in `C++` that goes beyond the `constexpr` evaluation of `core constant expressions`.

`Safe C++` [^p3390r0]
`Strategy for removing safety-related UB by default` [^p3436r0]

Most every day programmers are more specifically concerned about the memory unsafety subset of undefined behavior. While we can't at this moment mandate that all of `C++` is free of memory unsafety, can we at least lock down the low hanging fruit that is already amenable to safety related verbiage.

## Wording

### 11.10 Comparisons [class.compare]

#### 11.10.1 Defaulted comparison operator functions [class.compare.default]

<sup>1</sup> A defaulted comparison operator function (12.4.3) shall be a non-template function that

...

++<sup>1.4</sup> free of memory safety related undefined or erroneous behavior.++

## Impact on the standard

Safety not only requires safety to be in the language but also a part of the libraries even when those functions are created by the compiler. This proposal provides safety guarantees in some portions of the standard by requiring compiler implementers to make `defaulted comparison operator functions` free of memory safety related undefined behavior.

The proposed changes are relative to the current working draft `N5008` [^n5008].

## References
<!-- Consistent comparison -->
[^p0515r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0515r3.pdf>
<!-- <=> != == -->
[^p1185r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1185r2.html>
<!-- When do you actually use <=>? -->
[^p1186r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1186r3.html>
<!-- Working Draft, Programming Languages -- C++ -->
[^n5008]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/n5008.pdf>
<!-- Safe C++ -->
[^p3390r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3390r0.html>
<!-- Strategy for removing safety-related UB by default -->
[^p3436r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3436r0.pdf>
