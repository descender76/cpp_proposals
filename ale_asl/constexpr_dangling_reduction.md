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

# dangling reduction - constexpr

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

- [dangling reduction - constexpr](#dangling-reduction-constexpr)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [The conditions](#the-conditions)
  - [Why not before](#why-not-before)
  - [Other languages](#other-languages)
  - [Impact on current proposals](#impact-on-current-proposals)
    - [p2255r2](#p2255r2)
    - [p2576r0](#p2576r0)
  - [Rationale](#rationale)
    - [Expectations](#expectations)
    - [Past](#past)
    - [Present](#present)
    - [Future](#future)
      - [Proposal #1: `C++` with `static storage duration`](#proposal-1-c-with-static-storage-duration)
      - [Proposal #2: `C` `compound literals` with `static storage duration`](#proposal-2-c-compound-literals-with-static-storage-duration)
      - [Proposal #3: `constexpr` is `static storage duration`](#proposal-3-constexpr-is-static-storage-duration)
  - [Frequently Asked Questions](#frequently-asked-questions)
  - [References](#references)

<!--
  - [Ancillary examples](#ancillary-examples)
-->

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

1. the argument would, prior to this proposal, be a temporary
1. the type of the parameter is a constant reference or constant pointer
1. type of the temporary argument has a constexpr constructor
1. **[Optional]** type of the temporary argument has compile time comparison; constexpr <=>

What is being proposed is that the storage duration of the argument, under these specific conditions, be changed from automatic to static!
In other words, the temporary argument would be constructed with the constexpr constructor, stored statically in read only memory and automatically deduplicated if this unnamed constant expression was used more than once provided the type has an option `constexpr` spaceship operator.

This is reasonable because a constant reference/pointer parameter doesn't care how the object was constructed and if the compiler has the choice to constexpr construct the object once than why wouldn't one want that.

This feature is also similar to existing features/concepts that programmers are familiar with. This feature is an implicit anonymous constant. It is more like an implicit anonymous static local.

There is interest in eliminating, if not reducing dangling references. Consider `Why lifetime of temporary doesn't extend till lifetime of enclosing object?` [^soauto]. Also `Bind Returned/Initialized Objects to the Lifetime of Parameters` [^bindp] and `Lifetime safety: Preventing common dangling` [^lifetimesafety] talks about identify such issues but this proposal would address a small portion of them especially ones that would be considered surprising to programmers.

## Why not before

Only recently have we had all the pieces to make this happen. Further there is greater need now that more types are getting constexpr constructors. Also types that would normally be dynamically allocated such as string and vector has opened up the door wide for many more types being constructed at compile time. 

### C++11

* constexpr

### C++14

* relaxed constexpr restrictions

### C++20

* The spaceship operator
* `Making std::string constexpr` [^string]
* `Making std::vector constexpr` [^vector]

### C++23??

* `P2255R2 (A type trait to detect reference binding to temporary)` [^p2255r2]
* `Missing constexpr in std::optional and std::variant` [^optionalvariant]
* `Making std::unique_ptr constexpr` [^uniqueptr]

Since `C++11`, constexpr has continued to gain ground but the final pieces of the feature has only recently landed with stronger comparison via the spaceship operator in C++20 and, if it makes it, the ability to detect temporaries in `C++23`.

## Other languages

### many native languages

Many native languages, including C and C++, already provide this capability by storing said instances in the `COFF String Table` of the `portable executable format` [^pe].

### Java

Java automatically perform interning on its `String class` [^java].

### C# and other .NET languages

The .NET languages also performs interning on its `String class` [^csharp]

### many other languages

According to Wikipedia's article, `String interning` [^stringinterning], Python, PHP, Lua, Ruby, Julia, Lisp, Scheme, Smalltalk and Objective-C's each has this capability in one fashion or another.

What is proposed here is increasing the interning that C++ already does, but is not limited to just strings, in order to reduce dangling references. That fact is literals in `C++` is needless complicated with clearly unnecessary memory safety issues that impedes programmers coming to `C++` from other languages, even `C`.

<!--

## Ancillary Examples

```cpp
void some_function(const char*);

// native strings can also be created at compile time
some_fuction("hello world");
```

---

```cpp
void some_function(const std::string&);

// As of C++20, std::string can be constructed constexpr
some_fuction("hello world"s);
```

---

```cpp
void some_function(const std::vector<std::string>&);

// As of C++20, std::vector and std::string can be constructed constexpr
some_fuction({"hello", "world"});
```

---

```cpp
void some_function(const std::optional<std::string>&);

// As of C++23, std::optional and std::string can be constructed constexpr
some_fuction("hello world"s);
```

---

```cpp
void some_function(const std::variant<std::string>&);

// As of C++23, std::variant and std::string can be constructed constexpr
some_fuction("hello world"s);
```

---

```cpp
void some_function(const std::unique_ptr<std::string>&);

// As of C++23, std::unique_ptr can be constructed constexpr
some_fuction(std::make_unique<std::string>("hello world"));
```

-->

## Impact on current proposals

### p2255r2
***`A type trait to detect reference binding to temporary`*** [^p2255r2]

Following is a slightly modified example taken from the `p2255r2` [^p2255r2] proposal. Only the suffix `s` has been added. Currently, such an example is immediately dangling.

#### Before

```cpp
std::tuple<const std::string&> x("hello"s); // dangling
```

After `p2255r2` [^p2255r2] this becomes ill-formed, which is a vast improvement since the compiler is informing us of an error.

#### After p2255r2

```cpp
std::tuple<const std::string&> x("hello"s); // ill-formed
```

After this proposal, such an example becomes correct because `"hello"s` will be globally allocated and as such can't dangle since `std::string` has a `constexpr` constructor.

#### After this proposal

```cpp
std::tuple<const std::string&> x("hello"s); // correct
```

With this proposal `reference_constructs_from_temporary` and `reference_converts_from_temporary` would always return false for constant expressions. Does this mean `p2255r2` [^p2255r2] is useless? NO, while instances created at compile time would no longer dangle, mutable objects still could dangle. This is illustrated in the following examples.

#### Before

```cpp
std::tuple<const std::string&> x(factory_of_string_at_runtime()); // dangling
```

After `p2255r2` [^p2255r2] this becomes ill-formed, which again is a vast improvement since the compiler is informing us of an error.

#### After p2255r2

```cpp
std::tuple<const std::string&> x(factory_of_string_at_runtime()); // ill-formed
```

### p2576r0
***`The constexpr specifier for object definitions`*** [^p2576r0]

The `p2576r0` [^p2576r0] proposal is about contributing `constexpr` back to the `C` programming language. Interestingly, `C++` has `constexpr` in the first place, in part, to allow `C99` compound literals in `C++`. In the `p2576r0` [^p2576r0] proposal there are numerous references to "constant expression" and "static storage duration" highlighting that this and my proposal are playing in the same playground. Consider the following:

*"C requires that objects with static storage duration are only initialized with constant expressions.*

*"Because C limits initialization of objects with static storage duration to constant expressions, it can be difficult to create clean abstractions for complicated value generation."*

Further, the `p2576r0` [^p2576r0] proposal has a whole section devoted to just storage duration.

*"3.4. Storage duration"*

*"For the storage duration of the created objects we go with C++ for compatibility, that is per default we have automatic in block scope and static in file scope. The default for block scope can be overwritten by static or refined by register. It would perhaps be more natural for named constants"*

-  *"to be addressless (similar to a register declaration or an enumeration),"*
-  *"to have static storage duration (imply static even in block scope), or"*
-  *"to have no linkage (similar to typedef or block local static)"*

*"but we decided to go with C++’s choices for compatibility."*

My proposal would constitute a delay in the `p2576r0` [^p2576r0] proposal as I am advocating for refining the `C++` choices before contributing `constexpr` back to `C`. I also believe that the `p2576r0` [^p2576r0] proposal fails to consider a fourth alternative with respect to storage duration and that is to go with how `C` handles compound literals and have `C++` to conform with it. This will be considered momentarily.

## Rationale

### Expectations

There is a general expectation that constant expressions are already of static storage duration. While that is wrong, that expectation is still there. Consider the following examples. Everywhere you see ROMable, you might as well say global or static.

**December 19, 2012**

`Using constexpr to Improve Security, Performance and Encapsulation in C++` [^smartbear]

*"One of the advantages of user-defined literals with a small memory footprint is that an implementation can store them in the system’s ROM. Without a constexpr constructor, the object would require dynamic initialization and therefore wouldn’t be ROM-able."*

...

*"Compile-time evaluation of expressions often leads to more efficient code and enables the compiler to store the result in the system’s ROM."*

**May 21, 2015**

`Bitesize Modern C++ : constexpr` [^bitesize]

*"ROM-able types"*

...

*"Since everything required to construct the Rommable object is known at compile-time it can be constructed in read-only memory."*

**4 February 2019**

`C++ Core Guidelines: Programming at Compile Time with constexpr` [^guidelines]

*"A constant expression"*

-  *"can be evaluated at compile time."*
-  *"give the compiler deep insight into the code."*
-  *"are implicitly thread-safe."*
-  *"can be constructed in the read-only memory (ROM-able)."*

It isn't just that resolved constant expressions can be placed in ROM which makes programmers believe these should be stored globally but also the fact that fundamentally these expressions are executed at compile time. Constant expressions are the closest thing `C++` has to pure functions. That means the results are the same given the parameters, since these expressions run at compile time than the resultant values are the same no matter where or when in the `C++` program. This is essentially global to the program; technically across programs too.

This proposal just requests that at least in specific scenarios that instead of resolved constant expressions CAN be ROMable but rather that they HAVE to be or at least the next closest thing; constant and `static storage duration`.

Let's also consider the view of compiler writers briefly.

**Asked 9 years, 10 months ago**

`How can I get GCC to place a C++ constexpr in ROM?` [^gccconstexprrom]

...

*"As far as I understand the ffx object in the code below should end up in ROM (code), but instead it is placed in DATA."*

...

*"This is indeed fixed in gcc 4.7.0"*

So there is at least some understanding among compiler writers that instances produced from constexpr expressions
were ROMable. What is more interesting is that it doesn't matter if it is in the ROM/code/text segment or the DATA segment, it is still global regardless of which. One is just read only. If we can guarantee `static storage duration` for constant temporaries we can reduce danglings.

### Past

TODO

### Present

TODO

### Future

TODO

#### Proposal #1: `C++` with `static storage duration`

TODO

#### Proposal #2: `C` `compound literals` with `static storage duration`

TODO

#### Proposal #3: `constexpr` is `static storage duration`

TODO

## Frequently Asked Questions

TODO

## References

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
<!--PE Format-->
[^pe]: <https://docs.microsoft.com/en-us/windows/win32/debug/pe-format>
<!--Class String-->
[^java]: <https://docs.oracle.com/javase/7/docs/api/java/lang/String.html#intern%28%29>
<!--String.Intern(String) Method-->
[^csharp]: <https://docs.microsoft.com/en-us/dotnet/api/system.string.intern?view=net-6.0>
<!--String interning-->
[^stringinterning]: <https://en.wikipedia.org/wiki/String_interning>
<!--A type trait to detect reference binding to temporary-->
[^p2255r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2255r2.html>
<!--The constexpr specifier for object definitions-->
[^p2576r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2576r0.html>
<!--Using constexpr to Improve Security, Performance and Encapsulation in C++-->
[^smartbear]: <https://smartbear.com/blog/using-constexpr-to-improve-security-performance-an/>
<!--Bitesize Modern C++ : constexpr-->
[^bitesize]: <https://blog.feabhas.com/2015/05/bitesize-modern-c-constexpr/>
<!--C++ Core Guidelines: Programming at Compile Time with constexpr-->
[^guidelines]: <https://www.modernescpp.com/index.php/c-core-guidelines-programming-at-compile-time-with-constexpr>
<!--How can I get GCC to place a C++ constexpr in ROM?-->
[^gccconstexprrom]: <https://stackoverflow.com/questions/10982249/how-can-i-get-gcc-to-place-a-c-constexpr-in-rom>

<!---->
<!--
[^]: <>
-->

<!--
recipients
To: C++ Library Evolution Working Group <lib-ext@lists.isocpp.org>
Cc: Tomasz Kamiński <tomaszkam@gmail.com>
Gašper Ažman
gasper.azman@gmail.com
Tim Song
<t.canens.cpp@gmail.com>
Alex Gilding (Perforce UK)
Jens Gustedt (INRIA France)
Martin Uecker and Joseph Myers
Gabriel Dos Reis Bjarne Stroustrup Jens Maurer
Gabriel Dos Reis
gdr@cs.tamu.edu
-->
