<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3824R2</td>
</tr>
<tr>
<td>Date</td>
<td>2025-11-1</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Core Working Group<br/>Evolution Working Group<br/>SG23: Safety and Security</td>
</tr>
</table>

# Static storage for braced initializers NBC examples

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

- [Static storage for braced initializers NBC examples](#Static-storage-for-braced-initializers-NBC-examples)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Wording](#Wording)
  - [Impact on the standard](#Impact-on-the-standard)
  - [References](#References)

## Changelog

### R2

- wording clarifications

### R1

- made inline references more pronounced 
- added `Wording` section 

## Abstract

**Require** `static storage duration` in `Static storage for braced initializers` [^p2752r3] so that more memory unsafety will not be added to the `C++` language and thus ensure that this feature will be utilized instead of being bypassed for legacy code that does offer such assurances.

## Motivation

### The problem

Without `Static storage for braced initializers` [^p2752r3] the programmer has to perform a work around as stated in `2.1. Workarounds` section. [^workarounds]

<table>
<tr>
<td>

*"This code creates a 2MB backing array on the stack frame of the function that initializes `v`:"*

```cpp
std::vector<char> v = {
    #embed "2mb-image.png"
};
```

*"This code does not:"*

```cpp
static const char backing[] = {
    #embed "2mb-image.png"
};
std::vector<char> v = std::vector<char>(backing, std::end(backing));
```

*"So the latter is a workaround. But it shouldn’t be necessary to work around this issue; we should fix it instead."*

</td>
</tr>
</table>

Unfortuntely, with this feature, as currently worded, the programmer still has to do the work around.

`9.5.5 List-initialization [dcl.init.list] Paragraph 6` [^n5014] makes no mention of `static storage duration` in the specification. It only has an example of what a compiler might do.

This is further confirmed in a footnote at the end of the standard.

<table>
<tr>
<td>

`C.1.4 Clause 9: declarations [diff.cpp23.dcl.dcl] Paragraph 2` [^n5014] 

*"Permit the implementation to store backing arrays in static read-only memory."*

</td>
</tr>
</table>

***Permit*** is not a guarantee and consequently is less than the guarantee made in the workaround where the `static` keyword is used.

Without a requirement, this feature not only fails to provide the desired performance goals but also fails by increasing inconsistency, ambiguity and unsafety in the language. Consider the following example.

```cpp
#include <initializer_list>

const char& first()
{
    std::initializer_list<char> il = { 'h', 'w' };
    return *il.begin();
}

const char& first(std::initializer_list<char> il)
{
    return *il.begin();
}

int main() {
    const char& rilm = first();
    const char& pilm = first({ 'h', 'w' });
    return 0;
}
```

This is actually two examples of potential dangling references, `rilm` and `pilm`, being created; both by return and by passthrough. If the compiler choose to make the lifetimes of the backing arrays of the initializer lists to have `static storage duration` than `rilm` and `pilm` are memory safe, race free and thread safe. If the compiler chooses `automatic storage duration` than `rilm` and `pilm` are dangling references.

If the programmer thinks that their code has `static storage duration` because it was based on the provided example in the standard and the compiler left it as `automatic storage duration` then the programmer likely did not make the necessary code changes to ensure it doesn’t dangle. Not being able to trust this feature will cause programmers to not trust their compilers and to program with the esoteric legacy workaround code, thus defeating the purpose of having this feature in the first place.

A programmer can’t be expected to be responsible for dangling, if the programmer can’t definitely reason about the lifetimes of objects. A code reviewer/code auditor can’t be expected to find dangling, if the reviewer/auditor can’t definitely reason about the lifetimes of objects. None of these three people should be forced to look at assembly, a different programming language, to know what the compiler decided to do in this instant and worse to repeat the process for each initializer list/braced initializer in a program.

Without definite verbiage in the standard, this is a non portable language feature between compilers. Worse yet, it may not be portable/stable within a single compiler as the behavior could vary with respect to different compiler flags or other internal logic.

Besides failing to require `static storage duration` the current verbiage fails to even mention the possibility of `static storage duration`.

<table>
<tr>
<td>

`9.5.5 List-initialization [dcl.init.list] Paragraph 6` [^n5014]

*"The backing array has the same lifetime as any other temporary object (6.8.7), except that initializing an initializer_list object from the array extends the lifetime of the array exactly like binding a reference to a temporary."*

</td>
</tr>
</table>

Except that it is not exactly the same because when the lifetime of a temporary is extended it still has `automatic storage duration` changed from the statement to the block, like a declared variable.

**C.1.4 Clause 9: declarations [diff.cpp23.dcl.dcl]	Paragraph 2** [^n5014]
```cpp
const int& x = (const int&)1; // temporary for value 1 has same lifetime as x
```

What is proposed is different because it is being extended by being turned into something that has `static storage direction`.

### The solution

The lifetime of the backing array associated with initializer lists needs to definitely state `static storage duration` in a similar fashion as stated else where in the standard. Consider the very first occurence of `static storage duration` in the standard.

**5.13.5 String literals [lex.string] Paragraph 9** [^n5014]
*"Evaluating a string-literal results in a string literal object with static storage duration (6.8.6)."*

Should a initializer list of ints or even chars be specified radically nebulous from the clear straight forward verbiage of string literals. Fixing the initializer list verbiage with the string literal verbiage would make the two more consistent and in a reasonable fashion.

<table>
<tr>
<td>

```cpp
const char& first()
{
    const char* sl = "hw";
    return *sl;
}

const char& first(const char* sl)
{
    return *sl;
}

int main() {
    // no dangling
    const char& rslm = first();
    const char& pslm = first("hw");
    return 0;
}
```

</td>
<td>

```cpp
#include <initializer_list>

const char& first()
{
    std::initializer_list<char> il = { 'h', 'w' };
    return *il.begin();
}

const char& first(std::initializer_list<char> il)
{
    return *il.begin();
}

int main() {
    // no dangling
    const char& rilm = first();
    const char& pilm = first({ 'h', 'w' });
    return 0;
}
```

</td>
</tr>
</table>

Even the example provided by the standard could benefit from some revisions.

`9.5.5 List-initialization [dcl.init.list] Paragraph 6` [^n5014]
    
```cpp
void f(std::initializer_list<double> il);
void g(float x) {
    f({1, x, 3});
}
void h() {
    f({1, 2, 3});
}
```

The current example could be expanded to better illustrate when this feature does kick in versus may kick in.

```cpp
void f(std::initializer_list<double> il);
void g(float x) {
    f({1, x, 3});// automatic storage duration
}
void h() {
    f({1, 2, 3});// static storage duration
}
void i() {
    const float x = 2;
    // ...
    // ...
    // ...
    f({1, x, x, x, 3});// for now, may or may not be static storage duration
    // depending on how much local analysis a compiler is willing to do
    // hopefully mandated in future release, at least programmers
    // would still have the static storage duration guarantee with
    // f({1, 2, 2, 2, 3});
}
```

While it would be ideal if both function `h` and function `i` examples had `static storage duration`, since `f({1, x, x, x, 3});` requires analysis of additional lines of code it could make sense, on the short term, that it MAY have `static storage duration`. Hopefully, even this scenario would be strengthened in future `C++` releases since it is needed for the deduplication of constants in order to fulfill DRY, don't repeat yourself<!--, and ODR, one definition rule-->. At least with requiring `static storage duration` on function `h`, their would be a fallback plan that would still allow the programmer to use this feature. Without even this minimal guarantee, programmers have to fallback on to esoteric workaround code.

Initializer lists are a pure reference type. Shouldn't other standard pure reference types also have similar functionality, such as `std::span<const T>` and `std::string_view`.

```cpp
#include <span>

const char& first()
{
    //std::span<const char> il = {{ 'h', 'w' }};// pre C++26
    std::span<const char> il = { 'h', 'w' };// C++26
    return il.front();
}

const char& first(std::span<const char> il)
{
    return il.front();
}

int main() {
    const char& rilm = first();// may or may not dangle
    //const char& pilm = first({{ 'h', 'w' }});// pre C++26, dangle
    const char& pilm = first({ 'h', 'w' });// C++26, may or may not dangle
    return 0;
}
```

```cpp
#include <initializer_list>
#include <string_view>

std::string_view to_string_view(std::initializer_list<char> il)
{
    return {il.begin(), il.size()};
}

const char& first()
{
    std::initializer_list<char> il = { 'h', 'w' };
    std::string_view sv = to_string_view(il);
    return sv.front();
}

const char& first(std::string_view il)
{
    return il.front();
}

int main() {
    const char& rilm = first();// may or may not dangle
    const char& pilm = first(to_string_view({ 'h', 'w' }));// may or may not dangle
    return 0;
}
```

Even these `std::span<const T>` and `std::string_view` examples would become safe if initializer lists were given a `static storage duration` guarantee.

### The bonus problem

Another important issue that should be fixed is that the original proposal was for *“Static storage for **braced initializers**”* yet the current verbiage is only for initializer lists. If the currently worded functionality is reasonable even for an initializer list of size one, why not for single instance objects that are braced initialized entirely of constant literals. 

<table>
<tr>
<td>

```cpp
std::initializer_list<int> x1 = { 1 };
```

</td>
<td>

```cpp
const int& x1 = {1};
const int& x2{1};
const int& x3 = 1;
```

</td>
</tr>
</table>

Might this be viewed as an inconsistency and even worse, a missed opportunity to fix a swath of dangling references by lifetime extending objects to have `static storage duration`.

<table>
<tr>
<td>

```cpp
const int& first()
{
  const int& constant = 1;
  return cl;
  // safe if static
}
```

</td>
<td>

```cpp
const int& first()
{
  const int constant = 1;
  return cl;
}
```

</td>
<td>

```cpp
const int& first()
{
  return 1;
  // safe if static
}
```

</td>
</tr>
<tr>
<td colspan="3">

**Example #2 Comments**
gcc, clang
warning: returning reference to local temporary object [-Wreturn-stack-address]
if not const
error: non-const lvalue reference to type 'int' cannot bind to a temporary of type 'int'
arguable should be error regardless of `const`
`C++23` P2266R3 `Simpler implicit move` returning reference to xvalue is  ill formed
</td>
</tr>
</table>

**NOTE:** The first and the third would be logically safe had the object had `static storage duration`. The second would be caught as an error if compilers such as clang and gcc would implement/deploy `Simpler implicit move` [^p2266r3] such that returning a reference to an xvalue is ill-formed. If the bonus solution is accepted, than hopefully future standards could require some local analysis so that the second could be made as safe as the first and third.

```cpp
const int& first(const int& passthrough)
{
    return passthrough;
}

int main() {
    const int& x1 = first();// safe if static
    const int& x2 = first(1);// safe if static
    return 0;
}
```
<!--
### The bonus solution

Split the one example ...

**6.8.7 Temporary objects [class.temporary] Paragraph 6 (6.8)** [^n5014]

*[Example 3 :*

```cpp
const int& x = (const int&)1; // temporary for value 1 has same lifetime as x
```
    
...  into two examples, over const.

```cpp
int& x = (int&)1; // temporary for value 1 has same lifetime as x
const int& y = (const int&)1; // temporary for value 1 has lifetime of static storage duration
```

```cpp
// temporary for value 1 is no longer temporary
// instead has lifetime of static storage duration
const int& x1 = (const int&)1;
const int& x2 = 1; // or simply
```

Further, some verbiage to this effect would be in order.
-->
## Wording

### Bonus wording

`6.8.7 Temporary objects [class.temporary] Paragraph 6 (6.8)` [^n5014]

...

<sup>6</sup> The third context is when a ~~reference~~++pure alias++ binds to a temporary object.<sup>25</sup>
    
++Pure alias types are types that consist of only references, pointers and no value members other than the size of referenced memory. The set of pure alias types includes known pure alias types such as reference, pointer, `initializer_list`, `reference_wrapper`, `basic_string_view`, `span`, `mdspan`, `function_ref`, `optional<&>`, many iterators since they are just pointers, many range views since they are just pairs of pointers and aggregates of only pure alias types.++

++A type `is definitely NOT lifetime extendable` when extending the lifetime of said object would change the semantic meaning of a program. Known types that would fall into this category are `jthread`, `lock_guard`, `scoped_lock`, `unique_lock`, `shared_lock`, `basic_ifstream`, `basic_ofstream`, `basic_fstream`; pretty much any type in which its destructor `closes` a non memory only OS resource.++

++A type `is lifetime extendable` when it is `is_trivially_destructible` or the destructor just frees memory by calling `delete` or `free`. The type must not be or composed of types that are definitely NOT lifetime extendable. Known types that fall into this category are those types that are `is_trivially_destructible`, also `Container` and `AssociativeContainer` when their contents are `lifetime extendable` and the collection itself doesn't have a functioning lock as that would make it definitely NOT lifetime extendable.++

++When the `pure alias type` is `const`, the type of the temporary `is lifetime extendable`, the temporary is being constructed via a `constexpr` or `consteval` constructor and all of the arguments to that constructor are entirely composed of `const` literals, then the lifetime of the temporary is static storage duration otherwise when a `pure alias` binds to a temporary object, t++~~T~~he temporary object to which the ~~reference~~++`pure alias`++ is bound or the temporary object that is the complete object of a subobject to which the ~~reference~~++`pure alias`++ is bound persists for the lifetime of the ~~reference~~++`pure alias`++ if the glvalue to which the ~~reference~~++`pure alias`++ is bound was obtained through one of the following:

...
...
...

Change example #3

*[Example 3 :*

```cpp
const int& x = (const int&)1; // temporary for value 1 has same lifetime as x
```
    
to

```cpp
// temporary for value 1 is no longer temporary
// instead has lifetime of static storage duration
const int& x1 = (const int&)1;
const int& x2 = 1; // or simply
```

...

### Initializer list only wording

`9.5.5 List-initialization [dcl.init.list] Paragraph 6` [^n5014]

*~~"The backing array has the same lifetime~~*++The backing array of an initializer_list has a lifetime of static storage duration unless one member of the array is initialized with a non `const` variable in which case the lifetime is the same++ *as any other temporary object (6.8.7), except that initializing an initializer_list object from the array extends the lifetime of the array exactly like binding a reference to a temporary."*
...

## Impact on the standard

The requested wording changes has the same impacts as stated and accepted in the original feature. **C.1.4 Clause 9: declarations [diff.cpp23.dcl.dcl]	Paragraph 2** [^n5014]

The language impact of the original feature would mirror the impact of requiring the object created in the expression `const T& = T;` to have static storage duration. Namely, *"Valid C++ 2023 code that relies on the result of pointer comparison between ~~backing arrays~~++objects++ may change behavior."* **C.1.4 Clause 9: declarations [diff.cpp23.dcl.dcl]	Paragraph 2** [^n5014]

The proposed changes are relative to the current working draft `N5014` [^n5014].

## References
<!-- Working Draft, Programming Languages -- C++ -->
[^n5014]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/n5014.pdf>
<!-- Static storage for braced initializers -->
[^p2752r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2752r3.html>
<!-- Static storage for braced initializers - 2.1. Workarounds -->
[^workarounds]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2752r3.html#workarounds>
<!-- Simpler implicit move -->
[^p2266r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2266r3.html>