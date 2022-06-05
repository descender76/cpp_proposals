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
<td>2022-06-05</td>
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
      - [n1511](#n1511)
      - [n2235](#n2235)
    - [Present](#present)
      - [C Standard Compound Literals](#c-standard-compund-literals)
      - [C++ Standard](#c-standard)
    - [Future](#future)
      - [Proposal #1: `C++` with `static storage duration`](#proposal-1-c-with-static-storage-duration)
      - [Proposal #2: `C` `compound literals` with `static storage duration`](#proposal-2-c-compound-literals-with-static-storage-duration)
      - [Optional Addendum #1 - Deduplication](#optional-addendum-1-deduplication)
      - [Optional Addendum #2 - Undefined Strings](#optional-addendum-2-undefined-strings)
      - [Optional Addendum #3 - Arrays](#optional-addendum-3-arrays)
      - [Optional Addendum #4 - Address of literal](#optional-addendum-4-address-of-literal)
      - [Optional Addendum #5 - Delayed Initialization](#optional-addendum-5-delayed-initialization)
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

It is clear from this `string_view` example that it dangles because `sv` is a reference and `""s` is a temporary.
***What is being proposed is that same example doesn't dangle!***
That given simple reasonable conditions, the lifetime of the temporary gets automatically extended.

## The conditions

1. the argument would, prior to this proposal, be a temporary
1. the type of the parameter is a constant reference or constant pointer
1. type of the temporary argument has a constexpr constructor
1. **[Optional]** type of the temporary argument has compile time comparison; constexpr <=>

What is being proposed is that the storage duration of the argument, under these specific conditions, be changed from automatic to static!
In other words, the temporary argument would be constructed with the constexpr constructor, stored statically in read only memory and automatically deduplicated if this unnamed constant expression was used more than once provided the type has an optional `constexpr` spaceship operator.

This is reasonable because a constant reference/pointer parameter doesn't care how the object was constructed. Further, if the compiler has the choice to constexpr construct the object once rather than repeatedly throughout the execution of the program, than why wouldn't a developer want that.

This feature is also similar to existing features/concepts that programmers are familiar with. This feature is an implicit anonymous `constant`. It is more like an implicit anonymous `static local`.

There is interest in eliminating, if not reducing dangling references. Consider:

1. `Why lifetime of temporary doesn't extend till lifetime of enclosing object?` [^soauto]
1. `Bind Returned/Initialized Objects to the Lifetime of Parameters` [^bindp]
1. `Lifetime safety: Preventing common dangling` [^lifetimesafety]

These talks about identify such issues but this proposal would address a small portion of them especially ones that would be considered surprising to programmers.

## Why not before

Only recently have we had all the pieces to make this happen. Further there is greater need now that more types are getting constexpr constructors. Also types that would normally be dynamically allocated such as string and vector has opened up the door wide for many more types being constructed at compile time. 

**C++11**

* constexpr

**C++14**

* relaxed constexpr restrictions

**C++20**

* The spaceship operator
* `Making std::string constexpr` [^string]
* `Making std::vector constexpr` [^vector]

**C++23**

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

What is proposed here is increasing the interning that C++ already does, but is not limited to just strings, in order to reduce dangling references. The fact is, literals in `C++` is needless complicated with clearly unnecessary memory safety issues that impedes programmers coming to `C++` from other languages, even `C`.

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

Following is a slightly modified `constexpr` example taken from the `p2255r2` [^p2255r2] proposal. Only the suffix `s` has been added. It is followed by a non `constexpr` example. Currently, such an example is immediately dangling. Via `p2255r2` [^p2255r2], both examples become ill formed. However, with this proposal the `constexpr` example becomes valid.

<table>
<tr>
<td>
</td>
<td>

**constexpr**

</td>
<td>

**runtime**

</td>
</tr>
<tr>
<td>

**Examples**

</td>
<td>

```cpp
std::tuple<const std::string&> x("hello"s);
```

</td>
<td>

```cpp
std::tuple<const std::string&> x(factory_of_string_at_runtime());
```

</td>
</tr>
<tr>
<td>

**Before**

</td>
<td>

```cpp
// dangling
```

</td>
<td>

```cpp
// dangling
```

</td>
</tr>
<tr>
<td>

`p2255r2` [^p2255r2]

</td>
<td>

```cpp
// ill-formed
```

</td>
<td>

```cpp
// ill-formed
```

</td>
</tr>
<tr>
<td>

`p2255r2` [^p2255r2] and this proposal

</td>
<td>

```cpp
// correct
```

</td>
<td>

```cpp
// ill-formed
```

</td>
</tr>
</table>

The proposed valid example is reasonable from many programmers perspective because `"hello"s` is a literal just like `"hello"` is a safe literal in C++ and C99 compound literals are **safer** literals because the lifetime is the life of the block instead of the expression. More on that latter.

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

There is a general expectation that constant expressions are already of static storage duration. While that is partially wrong, that expectation is still there. Consider the following examples. Everywhere you see ROMable, you might as well say constant and global/static.

**December 19, 2012**

`Using constexpr to Improve Security, Performance and Encapsulation in C++` [^smartbear]

*"One of the advantages of **user-defined literals** with a small memory footprint is that an implementation can store them in the system’s **ROM**. Without a **constexpr constructor**, the object would require dynamic initialization and therefore wouldn’t be **ROM-able**."*

...

*"**Compile-time evaluation of expressions** often leads to more efficient code and enables the compiler to store the result in the system’s **ROM**."*

**May 21, 2015**

`Bitesize Modern C++ : constexpr` [^bitesize]

*"**ROM-able** types"*

...

*"<u>Since everything required to construct the **Rommable** object is known at **compile-time** it can be constructed in **read-only memory**.</u>"*

**4 February 2019**

`C++ Core Guidelines: Programming at Compile Time with constexpr` [^guidelines]

*"A constant expression"*

-  *"**can** be evaluated at **compile time**."*
-  *"give the compiler deep insight into the code."*
-  *"are implicitly thread-safe."*
-  *"**can** be constructed in the **read-only memory (ROM-able)**."*

It isn't just that resolved constant expressions **can** be placed in ROM which makes programmers believe these **should** be stored globally but also the fact that fundamentally **these expressions are executed at compile time**. Along with templates, constant expressions are the closest thing `C++` has to **pure** functions. That means the results are the same given the parameters, and since these expressions run at compile time, than the resultant values are the same, no matter where or when in the `C++` program. This is essentially global to the program; technically across programs too.

This proposal just requests that at least in specific scenarios that instead of resolved constant expressions **CAN** be ROMable but rather that they **HAVE** to be or at least the next closest thing; constant and `static storage duration`.

Let's also consider the view of compiler writers briefly.

**Asked 9 years, 10++ months ago**

`How can I get GCC to place a C++ constexpr in ROM?` [^gccconstexprrom]

...

*"As far as I understand the ffx object in the code below should end up in **ROM (code)**, but instead it is placed in **DATA**."*

...

*"This is indeed fixed in gcc 4.7.0"*

So there is at least some understanding among compiler writers that instances produced from constexpr expressions
were ROMable. What is more interesting is that it doesn't matter if it is in the ROM/code/text segment or the DATA segment, it is still global regardless of which. One is just read only. If we can guarantee `static storage duration` for constant temporaries we can reduce danglings.

### Past

A brief consideration of the proposals that led to `constexpr` landing as a feature bears weight.

#### n1511
***`Literals for user-defined types`*** [^n1511]

**2003**

*"This note proposes a notion of **user-defined literals** based on literal constructors without requiring new syntax. If combined with the separate proposal for generalized initializer lists, it becomes a generalization of the **C99 notion of compound literals**."*

*"However, a constructor is a very general construct and there have been many requests for a way to express literals for user-defined types in such a way that a programmer can be **confident that a value <u>will be</u> constructed at compile time** and **potentially stored in ROM**. For example:"*

```cpp
complex z(1,2); // the variable z can be constructed at compile time
const complex cz(1,2); // the const cz can potentially be put in ROM
```

*"**Personally, I prefer (1): basically, a value is a literal if it is composed out of literals and implemented by a literal constructor. The problem with that is that some people will not trust compilers to do proper resolution, placement in ROM, placement in text segment**, etc. Choosing that solution would require text in the standard to constrain and/or guide implementations."*

*"**C99 compound literals**"*

*"In C99, it is explicitly allowed to take the address of a compound literal. For example:"*

```cpp
f(&(struct foo) { 1,2 });
```

*"**This makes sense only if we assume that the {1,2} is stored in a data segment (like a string literal**, but different from a int literal). **I see no problem allowing that, as long as it is understood that unless & is explicitly used or the literal, a user-defined literal is an rvalue** with which the optimizer has a free hand."*

*"**It would be tempting to expand this rule to user-defined literals bound to references.** For example:"*

```cpp
 void f(complex&);
 // …
 f(complex(1,2)); // ok?
```

*"However, this would touch upon some rather brittle parts of the overload resolution rules to do with rvalue vs. lvalue. For example:"*

```cpp
 vector<int> v;
 // ...
 vector<int>().swap(v); // ok
 swap(vector<int>(), v); // would become ok
```

*"I suggest we don’t touch this **unless we are looking at the rvalue/lvalue rules for other reasons.**"*

The other reason why we should re-evaluate user-defined literals bound to references is to reduce dangling pointers and dangling references. It is also surprising that user defined literals do not work simply without memory issues and that they currently work better in C and practically in every other language than it does in C++. It wouldn't hurt to fix any brittle rules and in the 10++ years since we got `constexpr` some of these may have already been fixed and it could be time to review these finer points.

Even if these brittle concerns haven't already been addressed, two things could be performed to mitigate these concerns. For instance a automatic conversion from lvalue reference to pointer would allow passing implicit and explicit, via `&`, references to pointer arguments. Along with static duration, `C++` would then support the address of `C99` compound literal syntax.

A more complicated solution for references would require compilers not just to look at 2 types but instead 3: the type of the resolved `constexpr`, the type of the argument and the explicit '&' in front of the `constexpr` when the argument is a pointer.

#### n2235
***`Generalized Constant Expressions—Revision 5`*** [^n2235]

**2007**

*"This paper generalizes the notion of constant expressions to include constant-expression functions and user-defined literals"*

*"The goal is ... to **increase C99 compatibility.**"*

*"This paper generalizes the notion of constant expressions to include calls to “sufficiently simple” functions (constexpr functions) and objects of user-defined types constructed from “sufficiently simple” constructors (constexpr constructors.)"*

*"**simplify the language** definition in the area of constant expression **to match existing practice**"*

*"Any enhancement of the notion of constant expressions has to carefully consider the entanglement of many different notions, but strongly related. Indeed, the notion of constant expression appears in different contexts:"*

*"3. Static initialization of objects with static storage."*

*"Similarly, we do not propose to change the already complex and subtle distinction between “static initialization” and “dynamic initialization”. However **we strive for more uniform and consistency among related C++ language features and compatibility**"*

*"3 Problems"*

*"Most of the problems addressed by this proposal have been discussed in previous papers, especially the initial proposal for Generalized Constant Expressions [DR03], the proposal for Literals for user-defined types [Str03], Generalized initializer lists [DRS03], Initializer lists [SDR05]. What follows is a brief summary."*

*"3.4 Unexpected dynamic initialization"*

*"**However, it is possible to be surprised by expressions that (to someone) “look const” but are not.**"*

*"3.5 Complex rules for simple things"*

*"The focus of this proposal is to address the issues mentioned in preceding sections. However, discussions in the Core Working Group at the Berlin meeting (April 2006) concluded that the current rules for integral constant expressions are too complicated, and source of several Defect Reports. **Consequently, a “cleanup”, i.e. adoption of simpler, more general rules is suggested.**"*

*"4 Suggestions for C++0x"*

*"Second, we introduce “literals for user-defined type” based on the notion of constant expression constructors."*

*"4.2 Constant-expression data"*

*"A constant-expression value is a variable or data member declared with the constexpr specifier."*

*"As for other const variables, storage need not be allocated for a constant expression datum, unless its address is taken."*

```cpp
// the &x forces x into memory
```

*"When the initializer for an ordinary variable (i.e. not a constexpr) happens to be a constant, the compiler can choose to do dynamic or static initialization (as ever)."*

*"Declaring a constructor constexpr will help compilers to identify static initialization and perform appropriate optimizations (like putting literals in read-only memory.) Note that since “ROM” isn’t a concept of the C++ Standard and what to put into ROM is often a quite subtle design decision, this proposal simply allows the programmer to indicate what might be put into ROM (constant-expression data) rather than trying to specify what actually goes into ROM in a particular implementation."*

*"We do not propose to make constexpr a storage-class-specifier because it can be combined with either static or extern or register, much like const."*

Most of these references are given to give us a better idea of the current behavior of constexpr. However, it should be noted that the motivations of this proposal is similar to the motivations for `constexpr` itself. Mainly ...

- Increase C99 compatibility
- Simplify the language to match existing practice
- Lessen the surprise of unreasonable lifetimes by expressions that look and are const
- Consequently, a “cleanup”, i.e. adoption of simpler, more general rules

### Present

This proposal should also be considered in the light of the current standards. A better idea of our current rules is necessary to understanding how they may be simplied for the betterment of `C++`.

#### C Standard Compound Literals

Let's first look at how literals specifically compound literals behave in `C`. There is still a gap between `C99` and `C++` and closing or reducing that gap would not only increase our compatibility but also reduce dangling.

`2021/10/18 Meneide, C Working Draft` [^n2731]

*"6.5.2.5 Compound literals"*

**paragraph 5**

*"The value of the compound literal is that of an unnamed object initialized by the initializer list. If the compound literal occurs outside the body of a function, the object has static storage duration; otherwise, it has **automatic storage duration associated with the enclosing block**."*

The lifetime of this "enclosing block" is longer than that of `C++`. In `C++` under `6.7.7 Temporary objects [class.temporary]` specifically `6.12` states a *temporary bound to a reference in a new-initializer (7.6.2.8) persists until the completion of the full-expression containing the new-initializer*.

`GCC` [^gcccompoundliterals] describes the result of this gap.

*"**In C, a compound literal designates an unnamed object with static or automatic storage duration. In C++, a compound literal designates a temporary object that only lives until the end of its full-expression. As a result, well-defined C code that takes the address of a subobject of a compound literal can be undefined in C++**, so G++ rejects the conversion of a temporary array to a pointer."*

Simply put `C` has fewer dangling than `C++`. Consequently, the remaining dangling should be easier to spot for developers not having to look at superfluous dangling. What is more is that `C`'s  solution covers both const and non const temporaries! Even though it is `C`, it is more like `C++` than what people give this feature credit for because it is tied to blocks/braces, just like RAII. This adds more weight that the `C` way is more intuitive.

GCC even takes this a step forward which is closer to what this proposal is advocating. The last reference also says the following.


*"**As a GNU extension, GCC allows initialization of objects with static storage duration by compound literals (which is not possible in ISO C99 because the initializer is not a constant).** It is handled as if the object were initialized only with the brace-enclosed list if the types of the compound literal and the object match. **The elements of the compound literal must be constant.** If the object being initialized has array type of unknown size, the size is determined by the size of the compound literal."*

While adopting more of `C99` compound lierals would reduce dangling just like this proposal, there is still the expectation that constant expressions are of by default of static storage duration. As such this proposal and `C99` compound literals are complimentary. Regardless, dangling would still be possible, especially for non constant expressions. Those could be fixed by some future non constant expression proposal.

For instance, stackoverflow has a good example of dangling with `Compund literals storage duration in C` [^sogcccompoundliterals].

```cpp
/* Example 2 - if statements with braces */

double *coefficients, value;

if(x){
    coefficients = (double[3]) {1.5, -3.0, 6.0};
}else{
    coefficients = (double[3]) {4.5, 1.0, -3.5};
}
value = evaluate_polynomial(coefficients);
```

While this is an example of dangling, it is also an example of uninitialized or more specifically delayed initialization. Interestingly, in the future, the block in question could be the block where `coefficients` is defined instead of the block where the uninitialized `coefficients` is initiated.

#### C++ Standard

It is also good to consider how the `C++` standard impacts this proposal and how the standard may be impacted by such a proposal.

`Working Draft, Standard for Programming Language C++` [^n4910]

String literals are traditionally one of the most common literals and in `C++` they have static storage duration. This leads developers to believe that all literals have static storage duration and that is the case in many programming languages.

*"5.13.5 String literals [lex.string]"*

*"9 Evaluating a string-literal results in a string literal object with **static storage duration** (6.7.5). Whether all string-literals are distinct (that is, are stored in nonoverlapping objects) and whether successive evaluations of a string-literal yield the same or a diﬀerent object is unspecifed."*

*"[Note 4: **The eﬀect of attempting to modify a string literal object is undefined.** — end note]"*

This proposal aligns or adjusts literals not only with `C` compound literals but also with `C++` string literals. It too should be noted that `C++` may be silent on whether other literals are like string. Are array literals of `static storage duration`? What about if the array was of characters? If currently unspecifed, this proposal favors making both native and custom literals that are constant expressions into `static storage duration`.

`C++` also says the *"eﬀect of attempting to modify a string literal object is undefined."* With us having constant and for so long, there is no reason for this to be undefined. A string literal could be stored in global memory twice once for constant and non constant use cases. Developers should favor constant because it ensures it is only there once. So there is no need for undefined behavior when the behavior can be clearly defined. If a compiler stored a string literal once in ROM and allowed it go to a non constant character pointer than that should be a language mandated warning. 

---

The section of the `C++` standard on temporary objects has an example of dangling.

*"6.7.7 Temporary objects [class.temporary]"*

*"(6.12) — A temporary bound to a reference in a new-initializer (7.6.2.8) persists until the completion of the full-expression containing the new-initializer."*

*"[Note 7: This might introduce a dangling reference. — end note]"*

*"[Example 5:"*

```cpp
struct S { int mi; const std::pair<int,int>& mp; };
S a { 1, {2,3} };
S* p = new S{ 1, {2,3} }; // creates dangling reference
```

*"— end example]"*

It should be noted that this example is not an example of dangling in `C99` or with my proposal.

---

*"9.4 Initializers [dcl.init]"*

Similarly, the section of the `C++` standard on initializer has multiple examples of dangling.

*"9.4.1 General [dcl.init.general]"*

```cpp
A a1{1, f()}; // OK, lifetime is extended
A a2(1, f()); // well-formed, but dangling reference
A a3{1.0, 1}; // error: narrowing conversion
A a4(1.0, 1); // well-formed, but dangling reference
A a5(1.0, std::move(n)); // OK
```

Provided that A has a constexpr constuctor, with this proposal, `a2` would not be dangling if f() too was a constant expression. Also `a4` would also not be dangling. The `a4` example does not need jumping to the signature of a function to figure out if it is a constant expression as is the case of the `a2` example. Really the `a4` example is surprising to current developers that it would be dangling since the native literals 1.0 and 1 can be constant expressions.

It should also be noted that the `C++11` brace initialization does not or should not create another block scope.

*"9.4.5 List-initialization [dcl.init.list]"*

Similarly, the section of the `C++` standard on list-initialization has an example of dangling.

```cpp
struct A {
std::initializer_list<int> i4;
A() : i4{ 1, 2, 3 } {} // ill-formed, would create a dangling reference
};
```

According to this proposal, just like native arrays list initializers should not dangle either by being implicitly constexpr or explicitly created so.

---

*"11.9.6 Copy/move elision [class.copy.elision]"*

Similarly, the section of the `C++` standard on copy and move elision has examples of dangling.

```cpp
constexpr A a; // well-formed, a.p points to a
constexpr A b = g(); // error: b.p would be dangling (7.7)
void h() {
A c = g(); // well-formed, c.p can point to c or be dangling
}
```

Not only would `b` no longer dangle but the anbiguity of `c.p` could "point to c or be dangling" would be gone.

### Future

There are many paths to reducing dangling via using `static storage duration`. The question that need answering is too what degree do we do it and whether or not we also reduce the gap between `C` and `C++`.

#### Proposal #1: `C++` with `static storage duration`

##### Wording

6.7.7 Temporary objects [class.temporary]

(6.12) — A temporary bound to a reference in a new-initializer (7.6.2.8) persists until the completion of the full-expression containing the new-initializer ++unless the expected type is constant and the temporary possesses a constexpr constructor in which case the temporary is no longer a temporary but rather is of static storage duration++.

NOTE: All examples of dangling need to be revised for not involving constant expressions.

#### Proposal #2: `C compound literals` with `static storage duration`

##### Wording

6.7.7 Temporary objects [class.temporary]

(6.12) — A temporary bound to a reference in a new-initializer (7.6.2.8) persists until the completion of the ++enclosing block of the++ full-expression containing the new-initializer ++unless the expected type is constant and the temporary possesses a constexpr constructor in which case the temporary is no longer a temporary but rather is of static storage duration++.

NOTE: All examples of dangling need to be revised for not involving constant expressions.

#### Optional Addendum #1 - Deduplication

##### Wording

5.13.5 String literals [lex.string]

9 Evaluating a string-literal results in a string literal object with static storage duration (6.7.5). Whether ~~all~~ string-literals ++bound to a character pointer++ are distinct (that is, are stored in nonoverlapping objects) and whether successive evaluations of a string-literal yield the same or a diﬀerent object is unspecifed. ++All successive evaluations of a string-literals bound to a const pointer should yield the same object. Similarly too, all successive evaluations of a custom string-like-literals bound to a const pointer should yield the same object. Further, std::basic_string_view should be stored in overlapping objects.++

[Note 4: The eﬀect of attempting to modify a string literal object is undefned. — end note]

#### Optional Addendum #2 - Undefined Strings

##### Wording

5.13.5 String literals [lex.string]

9 ++Evaluating a string-literal bound to a character pointer results in a string literal object with automatic storage duration (6.7.5).++ Evaluating a string-literal ++bound to a const character pointer++ results in a string literal object with static storage duration (6.7.5). Whether all string-literals are distinct (that is, are stored in nonoverlapping objects) and whether successive evaluations of a string-literal yield the same or a diﬀerent object is unspecifed.

~~[Note 4: The eﬀect of attempting to modify a string literal object is undefined. — end note]~~

#### Optional Addendum #3 - Arrays

Similar wording as for string-literals is needed for native arrays, const std::array, const std::span and initializer lists.

#### Optional Addendum #4 - Address of literal

In order to allow taking the address of a literal, either a new conversion needs adding or the lvalue reference composition rules need revising.

```cpp
f(&(struct foo) { 1,2 });
```

++7.3.16 Lvalue-to-pointer conversion [conv.lval.pointer]++

++1 An lvalue of type T can be converted to a type “pointer to T”.++

++or++

++Taking the explicit address, `&`, of a lvalue reference yields the lvalue reference unless a pointer of compatible type is expected.++

#### Optional Addendum #5 - Delayed Initialization

Assigning a literal to a non const uninitialiated variable results in the lifetime of the literal being the block that contains the declaration of said variable. This one is for that earlier example found on stack overflow.

```cpp
/* Example 2 - if statements with braces */

double *coefficients, value;

if(x){
    coefficients = (double[3]) {1.5, -3.0, 6.0};
}else{
    coefficients = (double[3]) {4.5, 1.0, -3.5};
}
value = evaluate_polynomial(coefficients);
```

## Frequently Asked Questions

### Why not just extend the lifetime as descibed in p0936r0?

`Bind Returned/Initialized Objects to the Lifetime of Parameters` [^bindp]

In that proposal, a question was raised.

*"Lifetime Extension or Just a Warning?"*
*"We could use the marker in two ways:"*

1. *"Warn only about some possible buggy behavior."*
1. *"Fix possible buggy behavior by extending the lifetime of temporaries"*

In reality, there are three scenarios; warning, **error** or just fix it by extending the lifetime.

However, things in the real world tend to be more complicated. Depending upon the scenario, at least theoretically some could be fixed, some could be errors and some could be warnings. Further, waiting on a more complicated solution that can fix everything may never happen, so shouldn't we fix what we can, when we can; i.e. low hanging fruit. Also, fixing everything the same way may not even be desirable. Let's consider a real scenario. Extending one's lifetime could mean 2 different things.

1. Change automatic storage duration such that a instances' lifetime is just moved higher up the stack as prescribed in p0936r0.
1. Change automatic storage duration to static storage duration. [This is what I am proposing but only for those that it logically applies to.]

If only #1 was applied holistically via p0936r0, -Wlifetime or some such then that would not be appropriate/reasonable for those that really should be fixed by #2. Likewise #2 can't fix all but MAY make sense for those that it applies to.

Any reduction in undefined behavior or dangling references should be welcomed, at least as long it can be explained simply and rationally.

### Why not just make the evaluation of all constant expressions by default of `static storage duration`?

This too would reduce dangling in the same way as this proposal and has the added benefit of being even simpler rules. [To be decided]

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
Bjarne Stroustrup Jens Maurer
bs@ms.com
Gabriel Dos Reis
gdr@microsoft.com
-->
