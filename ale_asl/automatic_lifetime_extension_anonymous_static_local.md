<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>D****R0</td>
</tr>
<tr>
<td>Date</td>
<td>2022-04-28</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Evolution Working Group (EWG)</td>
</tr>
</table>

# automatic lifetime extension - anonymous static local

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

- [automatic lifetime extension - anonymous static local](#automatic-lifetime-extension-anonymous-static-local)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [The conditions](#the-conditions)
  - [Why not before](#why-not-before)
  - [References](#references)

## Abstract

This document proposes changes to the C++ language to allow writing code that results in fewer dangling references.

## Motivating Examples

```cpp
std::string_view sv = "hello world"s;// immediate dangling reference
```
<!--
Example

// x86-64 clang (trunk)
// -std=c++20 -O2 -Wdangling-gsl
#include <iostream>
#include <string>

using namespace std::string_literals;

int main() {
    std::string_view sv = "hello world"s;
    std::cout << sv << std::endl;
    return 42;
}
-->
It is clear from the `string_view` example that it dangles because `sv` is a reference and `""s` is a temporary.
***What is being proposed is that same example doesn't dangle!***
That given simple reasonable conditions, the lifetime of the temporary gets automatically extended.

## The conditions

* argument would normally be a temporary
* parameter is a constant reference or constant pointer
* type of the temporary argument has constexpr constructor
* type of the temporary argument has compile time comparison; constexpr <=>

What is being proposed is that the storage duration of the argument, under these specific conditions, would be changed from automatic to static!
In other words, the temporary argument would be constructed with the constexpr constructor, stored statically in read only memory and automatically deduplicated if this unamed constant expression was used more than once.

This is reasonable because a constant reference/pointer parameter doesn't care how the object was constructed and if the compiler has the choice to constexpr construct the object once than why wouldn't one want that.

This feature is also similar to existing features/concepts that programmers are familiar with. This feature is an implicit anonymous constant. It is more like an implicit anonymous static local.

There is interest in eliminating, if not reducing dangling references. Consider `Why lifetime of temporary doesn't extend till lifetime of enclosing object?` [^soauto]. Also `Bind Returned/Initialized Objects to the Lifetime of Parameters` [^bindp] and `Lifetime safety: Preventing common dangling` [^lifetimesafety] talks about identify such issues but this proposal would address a small portion of them especially ones that would be considered surprising to programmers.

## Why not before

Only recently have we had all the pieces to make this happen.

### C++11

* constexpr

### C++14

* relaxed constexpr restrictions

### C++20

* The spaceship operator
* `Making std::string constexpr` [^string]
* `Making std::vector constexpr` [^vector]


### C++23??

* `P2255R2 (A type trait to detect reference binding to temporary)` [^temporarytt]
* `Missing constexpr in std::optional and std::variant` [^optionalvariant]
* `Making std::unique_ptr constexpr` [^uniqueptr]

## References

<!--P2255R2 (A type trait to detect reference binding to temporary)-->
[^temporarytt]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2255r2.html>
<!--Bind Returned/Initialized Objects to the Lifetime of Parameters-->
[^bindp]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0936r0.pdf>
<!--Lifetime safety: Preventing common dangling-->
[^lifetimesafety]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1179r1.pdf>
<!--Why lifetime of temporary doesn't extend till lifetime of enclosing object?-->
[^soauto]: <https://stackoverflow.com/questions/6936720/why-lifetime-of-temporary-doesnt-extend-till-lifetime-of-enclosing-object>
<!--Making std::string constexpr-->
[^string]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0980r1.pdf>
<!--Making std::vector constexpr-->
[^vector]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1004r2.pdf>
<!--Missing constexpr in std::optional and std::variant-->
[^optionalvariant]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2231r1.html>
<!--Making std::unique_ptr constexpr-->
[^uniqueptr]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2231r1.html>
