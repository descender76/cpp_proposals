<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P****R0</td>
</tr>
<tr>
<td>Date</td>
<td>2022-09-25</td>
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

# C++ is the next C++

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

- [C++ is the next C++](#c-is-the-next-c)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [Summary](#summary)
  - [Frequently Asked Questions](#frequently-asked-questions)
  - [References](#references)

## Abstract

Programmer's, Businesses and Government(s) want C++ to be safer and simpler. This has led some `C++` programmers to create new programming languages or preprocessors which again is a new language. This paper discusses using static analysis to make the `C++` language itself safer and simpler.

## Motivating Examples

Following is a wishlist. Most are optional. While, they all would be of benefit. It all starts with a new repeatable module level attribute that would preferably be applied once in the `primary module interface unit` and would automatically apply to it and all `module implementation unit`(s). It could also be applied to a `module implementation unit` but that would generally be less useful.

```cpp
export module some_module_name [[static_analysis("")]];// primary module interface unit
// or
module some_module_name [[static_analysis("")]];// module implementation unit
```

- It would be ideal if the name member of the `static_analysis` attribute could be passed as either an environment variable and/or command line argument to compilers so it could be used by pipelines to assert the degree of conformance to the defined static analyzer without actually changing the source.
- It would be ideal if compilers could standardize the environment variable name or command line argument name in order to ease tooling.
- It would be ideal if compilers could produce a machine readable report in JSON, YAML or something else so that pipelines could more easily consume the results.

The name of the static analyzer are dotted. Unscoped or names that start with `std.`, `c++.`, `cpp.`, `cxx.` or `c.` are reserved for standardization.

This proposal wants to stardardize two overarching static analyzer names; `safer` and `modern`.

<table>
<tr>
<td>

```cpp
[[static_analysis("safer")]]
```

</td>
<td>

The `safer` analyzer is for safety, primarily memory related. It is for those businesses and programmers who must conform to safety standards.

</td>
</tr>
<tr>
<td>

```cpp
[[static_analysis("modern")]]
```

</td>
<td>

The `safer` analyzer is a subset of `modern` analyzer. The `modern` analyzer goes beyond just memory and safety concerns. It can be thought of as bleeding edge. It is for those businesses and programmers who commit to modern code.

</td>
</tr>
</table>

Neither is concerned about formatting or nitpicking. Both static analyzers only produce errors. They both represent +infinity. These are meant for programmers, businesses and governments in which safety takes precedence. When a new version of the standard is released and adds new sub static analyzers than everyone's code is broken until their code is fixed. These sub static analyzers usually consist of features that have been mostly replaced with some other feature. It would be ideal if the errors produced not only say that the code is wrong but also provide a link to html page(s) maintained by both the `C++` teaching group and compiler specific errors. These pages should provide example(s) of what is being replaced and by what was it replaced. Mentioning the version of the `C++` standard would also be helpful.

### What are the `safer` and `modern` analyzers composed of?

These overarching static analyzers are composed of multiple static analyzers which can be used individually to allow a limited gradual adoption.

#### No pointers except function pointers

<table>
<tr>
<td>

```cpp
[[static_analysis("no_pointers_except_function_pointers")]]
```

</td>
<td>

`no_pointers_except_function_pointers` is a subset of `safer`.

</td>
</tr>
</table>

- Any declaration of a pointer is an error.
- Calling any function that has parameters that take a pointer is an error.
- Only function pointers can be used.

Can use modules that doesn't use these static analyzers.
replaced by
&
standard containers
smart pointers
(*). vs. -> (2 character difference)
past main proposals
shim
`A Modern C++ Signature for main` [^modernmain]
`Desert Sessions: Improving hostile environment interactions` [^hostileenv]

#### No unsafe casts

<table>
<tr>
<td>

```cpp
[[static_analysis("no_unsafe_casts")]]
```

</td>
<td>

`no_unsafe_casts` is a subset of `safer`.

</td>
</tr>
</table>

- Using `reinterpret_cast` produces an error.
- Using `C`/core cast produces an error.
- Using `const_cast` produces an error.

Why? These were replaced by `static_cast` and `dynamic_cast`.

#### No unions

<table>
<tr>
<td>

```cpp
[[static_analysis("no_union")]]
```

</td>
<td>

`no_union` is a subset of `safer`.

</td>
</tr>
</table>

Replaced by `std::variant`.

#### No mutable

<table>
<tr>
<td>

```cpp
[[static_analysis("no_mutable")]]
```

</td>
<td>

`no_mutable` is a subset of `safer`.

</td>
</tr>
</table>

Using the `mutable` keyword produces an error. The programmer shall not lie to oneself. The `mutable` keyword violates the safety of const and is rarely used.

#### No new or delete

<table>
<tr>
<td>

```cpp
[[static_analysis("no_new_delete")]]
```

</td>
<td>

`no_new_delete` is a subset of `safer`.

</td>
</tr>
</table>

Replaced with containers and smart pointers.

#### No deprecated

<table>
<tr>
<td>

```cpp
[[static_analysis("no_deprecated")]]
```

</td>
<td>

`no_deprecated` is a subset of `modern`.

</td>
</tr>
</table>

Using anything that has the deprecated attribute on it produces an error.

### What may `safer` and `modern` analyzers be composed of in the future?

#### No include

<table>
<tr>
<td>

```cpp
[[static_analysis("no_include")]]
```

</td>
<td>

`no_include` is a subset of `modern`.

</td>
</tr>
</table>

Replaced by import.
Don't add until #embed is added.

#### No goto

<table>
<tr>
<td>

```cpp
[[static_analysis("no_goto")]]
```

</td>
<td>

`no_goto` is a subset of `modern`.

</td>
</tr>
</table>

Using goto produces an error.
Don't add until break and continue to a label is added. Also a finite state machine.

## Summary

By adding static analysis to the `C++` language we can make the language safer and easier to teach.

## Frequently Asked Questions

### Shouldn't these be warnings instead of errors?

NO, otherwise we'll be stuck with what we just have. `C++` compilers produces plenty of warnings. `C++` static analyzers produces plenty of warnings. However, when some one talks about creating a new language, then old language syntax becomes invalid i.e. errors. This is for programmers. Programmers never upgrade their code unless they are forced to. Businesses and Government(s) want errors as well in order to ensure code quality and the assurance that bad code doesn't exist anywhere in the module. This is also important from a language standpoint because we are essentially pruning; somewhat. Keep in mind though that all of these pruned features still have use now and in the future as more constructs will be built upon them which is why they need to be part of the language just not a part of everyday usage of the language.

### Why at the module level? Why not safe and unsafe blocks?

Programmers never upgrade their code unless they are forced to. New programmers need training wheels and some of us older programmers like them too. Due to the proliferation of government regulations and oversight, businesses have acquired `software composition analysis` services and tools. These services map security errors to specific versions of modules; specifically programming artifacts such as executables and libraries. As such, businesses want to know is a module reasonably safe.

## References

<!-- Constant initialization -->
[^constinit]: <https://en.cppreference.com/w/cpp/language/constant_initialization>
<!-- Constants (C# Programming Guide) -->
[^csharpconstants]: <https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/classes-and-structs/constants>
<!-- Value categories -->
[^valuecategory]: <https://en.cppreference.com/w/cpp/language/value_category>
<!--Bind Returned/Initialized Objects to the Lifetime of Parameters-->
[^bindp]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0936r0.pdf>
<!-- String literal -->
[^stringliteral]: <https://en.cppreference.com/w/cpp/language/string_literal>
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
<!--Introduce storage-class specifiers for compound literals-->
[^n3038]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3038.htm>
<!--Using constexpr to Improve Security, Performance and Encapsulation in C++-->
[^smartbear]: <https://smartbear.com/blog/using-constexpr-to-improve-security-performance-an/>
<!--Bitesize Modern C++ : constexpr-->
[^bitesize]: <https://blog.feabhas.com/2015/05/bitesize-modern-c-constexpr/>
<!--C++ Core Guidelines: Programming at Compile Time with constexpr-->
[^guidelines]: <https://www.modernescpp.com/index.php/c-core-guidelines-programming-at-compile-time-with-constexpr>
<!--How can I get GCC to place a C++ constexpr in ROM?-->
[^gccconstexprrom]: <https://stackoverflow.com/questions/10982249/how-can-i-get-gcc-to-place-a-c-constexpr-in-rom>
<!--Literals for user-defined types-->
[^n1511]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1511.pdf>
<!--Generalized Constant Expressions—Revision 5-->
[^n2235]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf>
<!--2021/10/18 Meneide, C Working Draft-->
[^n2731]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2731.pdf>
<!--Working Draft, Standard for Programming Language C++-->
[^n4910]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4910.pdf>
<!--6.28 Compound Literals-->
[^gcccompoundliterals]: <https://gcc.gnu.org/onlinedocs/gcc/Compound-Literals.html>
<!--Compund literals storage duration in C-->
[^sogcccompoundliterals]: <https://stackoverflow.com/questions/62776214/compund-literals-storage-duration-in-c>
<!--C++ Language Evolution status pandemic edition-->
[^p1018r16]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1018r16.html>
<!--Fix the range‐based for loop, Rev1-->
[^p2012r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2012r1.pdf>
<!--C++ Core Guidelines-->
[^cppcgrfin]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-in>
<!--C++ Core Guidelines - F.42: Return a T* to indicate a position (only) -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f42-return-a-t-to-indicate-a-position-only>
<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgrf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>
<!--A Modern C++ Signature for main-->
[^modernmain]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0781r0.html>
<!--Desert Sessions: Improving hostile environment interactions-->
[^hostileenv]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1275r0.html>
