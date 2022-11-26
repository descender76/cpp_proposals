<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2724R0</td>
</tr>
<tr>
<td>Date</td>
<td>2022-11-25</td>
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

# constant dangling

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

- [constant dangling](#constant-dangling)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Motivating examples](#Motivating-Examples)
  - [Proposed Wording](#Proposed-Wording)
  - [In Depth Rationale](#In-Depth-Rationale)
  - [Value Categories](#Value-Categories)
  - [Performance Considerations](#Performance-Considerations)
  - [Tooling Opportunities](#Tooling-Opportunities)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Changelog

### R0

- The constant content was extracted and merged from the `temporary storage class specifiers` [^p2658r0] and `implicit constant initialization` [^p2623r2] proposals.

## Abstract

This paper proposes the standard adds anonymous global constants to the language with the intention of automatically fixing a shocking type of dangling which occurs when constants or that which should be constants dangle. This is shocking because constant like instances should really have constant-initialization meaning that they should have static storage duration and consequently should not dangle. This trips up beginner code requiring teaching dangling on day one. It is annoying to non beginners. Constants are used as defaults in production code. Constants are also frequently used in test and example code. Further, many instances of dangling used by non `C++` language comparisons frequently use constants as examples.

## Motivation

There are multiple resolutions to dangling in the `C++` language.

1. Produce an error
    - `Simpler implicit move` [^p2266r3]
1. Fix with block/variable scoping
    - `Fix the range-based for loop, Rev2` [^p2012r2]
    - `Get Fix of Broken Range-based for Loop Finally Done` [^p2644r0]
1. Fix by making the instance global
    - **`This proposal`**

All are valid resolutions and individually are better than the others, given the scenario. This proposal is focused on the third option, which is to fix by making the instance global.

Dangling the stack is shocking because is violates our trust in our compilers and language, since they are primarily responsible for the stack. However, there are three types of dangling that are even more shocking than the rest.

1. Returning a **direct** reference to a local
    - partially resolved by `Simpler implicit move` [^p2266r3]
1. Immediate dangling
1. **Dangling Constants**

Making an instance global is a legitimate fix to dangling.

<table>
<tr>
<td>

**C++ Core Guidelines**<br/>**F.43: Never (directly or indirectly) return a pointer or a reference to a local object** [^cppcgrf43]

***Reason** To avoid the crashes and data corruption that can result from the use of such a dangling pointer.* [^cppcgrf43]

...

***Note** This applies only to non-static local variables. All static variables are (as their name indicates) statically allocated, so that pointers to them cannot dangle.* [^cppcgrf43]

</td>
</tr>
</table>

While making an instance global doesn't fix all dangling in the language, it is the only resolution that can fix all three most shocking types of dangling provided the instance in question is a constant. It is also the best fix for these instances.

Since `constexpr` was added to the language in `C++11` there has been an increase in the candidates of temporary instances that could be turned into a global constant. ROMability was in part the motivation for `constexpr` but the requirement was never made. Even if a `C++` architecture doesn't support ROM, it is still required by language to support `static storage duration` and `const`. Matter of fact, due to the immutable nature of constant-initialized constant expressions, these expressions/instances are constant for the entire program even though they, at present, don't have `static storage duration`, even if just <a href="#What-about-locality-of-reference" title="What-about-locality-of-reference">logically</a>. There is a greater need now that more types are getting constexpr constructors. Also types that would normally only be dynamically allocated, such as string and vector, since `C++20`, can also be `constexpr`. This has opened up the door wide for many more types being constructed at compile time.

## Motivating Examples

Before diving into the examples, let's discuss what exactly is being asked for. There are two features; one implicit and the other explicit.

**implicit constant initialization**

If a temporary argument is constant-initialized [^n4910] <sup>*(7.7 Constant expressions [expr.const])*</sup>
and its argument/instance type is a `LiteralType`
and its parameter/local/member type is `const` and not `mutable`
then the instance is implicitly created with `constant initialization`.

As such it has `static storage duration` and can't dangle.

**explicit constant initialization**

The `constinit` specifier can be applied to temporaries. Applying it asserts that the temporary was `const-initialized`, that the argument type is a `LiteralType` and its parameter type is `const`. This explicitly gives the temporary `static storage duration`.

While `implicit constant initialization` automatically fixes dangle, `constinit` allows the programmers to manually and explicitly fix some dangling. The former is better for programmers and the language, while the later favors code reviewers or programmers who copy an example and want to have the compiler, momentarily, verify whether it is correct.

So what sorts of dangling does this fix for us. Besides fixing some dangling, this also fixes some inconsistencies between string literals [^n4910] <sup>*(5.13.5 String literals [lex.string])*</sup> and other literal types.

```cpp
// does not dangle because string literals are lvalues with static storage duration
std::string_view sv = "hello world";
// immediate dangling reference
// even though "hello world"s is a constant-initialized,
// literal type that is only being used in a const fashion
std::string_view sv = "hello world"s;// fixed in this proposal
std::string_view sv = constinit "hello world"s;// fixed in this proposal
```

This is reasonable based on how programmers reason about constants being immutable variables and temporaries which are known at compile time and do not change for the life of the program. This also works with plain old references.

```cpp
struct X
{
    int a, b;
};

const int& get_a(const X& x)
{
    // sporadically dangling, returning indirect reference
    return x.a;
}

const int& a = get_a({4, 2});
a; // currently dangling, will be 4 with this proposal
```

*"Such a feature would also help to ... fix several bugs we see in practice:"* [^bindp]

*"Consider we have a function returning the value of a map element or a default value if no such element exists without copying it:"* [^bindp]

```cpp
const V& findOrDefault(const std::map<K,V>& m, const K& key, const V& defvalue);
```

*"then this results in a classical bug:"* [^bindp]

```cpp
std::map<std::string, std::string> myMap;
const std::string& s = findOrDefault(myMap, key, "none"); // runtime bug if key not found
```

Is this really a bug? With this proposal, it isn't! Here is why. The function `findOrDefault` **expects** a **`const`** `string&` for its third parameter. Since `C++20`, string's constructor is `constexpr`. It **CAN** be constructed as a constant expression. Since all the arguments passed to this `constexpr` constructor are constant expressions, in this case `"none"`, the temporary `string` `defvalue` **IS** also `constant-initialized` [^n4910] <sup>*(7.7 Constant expressions [expr.const])*</sup>. This paper advises that if you have a non `mutable` `const` that it is `constant-initialized`, that the variable or temporary undergoes `constant initialization` [^n4910] <sup>*(6.9.3.2 Static initialization [basic.start.static])*</sup>. In other words it has implicit `static storage duration`. The temporary would actually cease to be a temporary. As such this usage of `findOrDefault` **CAN'T** dangle.

The pain of immediate dangling associated with temporaries are especially felt when working with other anonymous language features of `C++` such as lambda functions and coroutines.

#### Lambda functions

Whenever a lambda function captures a reference to a temporary it immediately dangles before an opportunity is given to call it, unless it is a immediately invoked lambda/function expression.

```cpp
[&c1 = "hello"s](const std::string& s)// OK
{
    return c1 + " "s + s;
}("world"s);// immediately invoked lambda/function expression

auto lambda = [&c1 = "hello"s](const std::string& s)// immediate dangling
{
    return c1 + " "s + s;
}
// ...
lambda("world"s);
```

This problem is resolved when the scope of temporaries has `static storage duration` instead of the containing expression provided `c1` resolves to a `const std::string&` since `c1` was constant-initialized. The `constinit` specifier could ensure this.

#### Coroutines

Similarly, whenever a coroutine gets constructed with a reference to a temporary it immediately dangles before an opportunity is given for it to be `co_await`ed upon.

```cpp
generator<char> each_char(const std::string& s) {
    for (char ch : s) {
        co_yield ch;
    }
}

int main() {
    auto ec = each_char("hello world")// immediate dangling
    for (char ch : ec) {
        std::print(ch);
    }
}
```

This specific immediately dangling example is fixed by implicit constant initialization since the parameter `s` expects a `const std::string&` and it was constant-initialized.

It should be noted too that the current rules of temporaries discourages the use of temporaries because of the dangling it introduces. However, if the lifetime of temporaries was increased to a reasonable degree than programmers would use temporaries more. This would reduce dangling further because there would be fewer named variables that could be propagated outside of their containing scope. This would also improve code clarity by reducing the number of lines of code allowing any remaining dangling to be more clearly seen.

## Proposed Wording

**6.7.5.4 Automatic storage duration [basic.stc.auto]**

<sub>1</sub> Variables that belong to a block or parameter scope and are not explicitly declared static, thread_local, ~~or~~ extern ++or had not underwent implicit constant initialization (6.9.3.2)++ have automatic storage duration. The storage for these entities lasts until the block in which they are created exits.

...

**6.9.3.2 Static initialization [basic.start.static]**

...

<sub>2</sub> Constant initialization is performed ++explicitly++ if a variable or temporary object with static or thread storage duration is constant-initialized (7.7). ++Constant initialization is performed implicitly if a non mutable const variable or non mutable const temporary object is constant-initialized (7.7).++ If constant initialization is not performed, a variable with static storage duration (6.7.5.2) or thread storage duration (6.7.5.3) is zero-initialized (9.4). Together, zero-initialization and constant initialization are called static initialization; all other initialization is dynamic initialization. All static initialization strongly happens before (6.9.2.2) any dynamic initialization.

...

**9.2.7 The constinit specifer [dcl.constinit]**

<sub>1</sub> ++If the constinit specifer is applied to a temporary, it gives the temporary static storage duration, asserts that the argument is a `LiteralType` and asserts that the parameter type is not `mutable` and `const` otherwise++ the constinit specifer shall be applied only to a declaration of a variable with static or thread storage duration. If the specifer is applied to any declaration of a variable, it shall be applied to the initializing declaration. No diagnostic is required if no constinit declaration is reachable at the point of the initializing declaration.

...

<span style="color:red">**NOTE: Wording still need to capture that these temporaries are no longer temporaries and that their value category is `lvalue`.**</span>

## In Depth Rationale

There is a general expectation across programming languages that constants or more specifically constant literals are "immutable values which are known at compile time and do not change for the life of the program".  [^csharpconstants] In most programming languages or rather the most widely used programming languages, constants do not dangle. Constants are so simple, so trivial (English wise), that it is shocking to even have to be conscience of dangling. This is shocking to `C++` beginners, expert programmers from other programming languages who come over to `C++` and at times even shocking to experienced `C++` programmers.

There is already significant interest in this type of feature from programmers. Just look at `C23` as an example. For instance, the `Introduce storage-class specifiers for compound literals` [^n3038] and `The 'constexpr' specifier`[^n2917] allows `C` programmers to specify `static`, `constexpr` and `thread_local` as storage class specifiers on their compound literals. The compound literals equivalent in `C++` is `LiteralType` and temporaries. This paper reuses our existing keyword `constinit` over `static` because of what we all know from the `C++ Core Guidelines`[^cppcgrf42].

<table>
<tr>
<td>

**I.2: Avoid non-const global variables**[^cppcgrf42]

**Reason** Non-const global variables hide dependencies and make the dependencies subject to unpredictable changes.[^cppcgrf42]

</td>
</tr>
</table>

I also did not choose `constexpr`, though that may be better for greater `C` compatibility, since I wrote my proposals before my seeing the `C` paper. Also `constinit` better matches that which these features are doing in the context of existing `C++` terminology of constant initialization. Further, there are differences in what `constexpr` means to `C++` and `C`, at present.

It should also be noted that these concepts are already in the standard just not fully exposed in the language. For instance, strings literals already have static storage duration and attempting to modify one is undefined.

<table>
<tr>
<td>

`Working Draft, Standard for Programming Language C++` [^n4910]

**"*5.13.5 String literals [lex.string]*"**

"*<sub>9</sub> **Evaluating a string-literal results in a string literal object with static storage duration** (6.7.5). Whether all string-literals are distinct (that is, are stored in nonoverlapping objects) and whether successive evaluations of a string-literal yield the same or a diﬀerent object is unspecifed.*"

"*[Note 4: The effect of attempting to modify a string literal object is undefined. — end note]*"

</td>
</tr>
</table>

Further, this behavior happens all the time with evaluations of constant expressions but unfortunately we can't enjoy all the benefits thereof. 

<table>
<tr>
<td>

`Working Draft, Standard for Programming Language C++` [^n4910]

"***6.9.3.2 Static initialization [basic.start.static]***"

"*<sub>1</sub> Variables with static storage duration are initialized as a consequence of program initiation. Variables with thread storage duration are initialized as a consequence of thread execution. Within each of these phases of initiation, initialization occurs as follows.*"

"*<sub>2</sub> Constant initialization is performed if a variable or temporary object with static or thread storage duration is constant-initialized (7.7). ...*"

...
    
"*<sub>3</sub> An implementation is permitted to perform the initialization of a variable with static or thread storage duration as a static initialization even if such initialization is not required to be done statically, provided that ...*"

</td>
</tr>
</table>

These ROM-able instances do not dangle as globals but from our code perspective they currently look like dangling locals. This causes false positive with our static analyzers and the programmer's themselves. If we would just admit from a language standpoint that these are indeed constants than not only do we fix some dangling but also our mental model. This same reference also says "constant initialization is performed if a ... temporary object with static ... storage duration is constant-initialized". Programmers can't fully utilize this scenario because at present we can only use `static` on class members and locals but not temporary arguments. Since the code identified by this paper is already subject to constant initialization than there is no real chance of these changes causing any breakage.

## Value Categories

If some temporaries can be changed to have global scope than how does it affect their value categories? Currently, if the literal is a string than it is a `lvalue` and it has global scope. For all the other literals, they tend to be a `prvalue` and have statement scope.

<table>
<tr>
<td></td>
<td>

**movable**

</td>
<td>

**unmovable**

</td>
</tr>
<tr>
<td>

**named**

</td>
<td>xvalue</td>
<td>lvalue</td>
</tr>
<tr>
<td>

**unnamed**

</td>
<td>prvalue</td>
<td>?</td>
</tr>
</table>

From the programmers perspective, global temporaries are just anonymously named variables. When they are passed as arguments, they have life beyond the life of the function that it is given to. As such the expression is not movable. As such, the desired behavior described throughout the paper is that they are `lvalues` which makes sense from a anonymously named standpoint. However, it must be said that technically they are unnamed which places them into the value category that `C++` currently does not have; the unmovable unnamed. The point is, this is simple whether it is worded as a `lvalue` or an unambiguous new value category that behaves like a `lvalue`. Regardless of which, there are some advantages that must be pointed out.

### Avoids superfluous moves

The proposed avoids superfluous moves. Copying pointers and lvalue references are cheaper than performing a move which is cheaper than performing any non trivial value copy.

### Undo forced naming

The proposed makes using types that delete their `rvalue` reference constructor easier to use. For instance, `std::reference_wrapper` can not be created/reassigned with a `rvalue` reference, i.e. temporaries. Rather, it must be created/reassigned with a `lvalue` reference created on a seperate line. This requires superfluous naming which increases the chances of dangling. Further, according to the `C++ Core Guidelines`, it is developers practice to do the following:

- *ES.5: Keep scopes small* [^cppcges5]
- *ES.6: Declare names in for-statement initializers and conditions to limit scope* [^cppcges6]

```cpp
// currently not permitted; works as proposed
std::reference_wrapper<int> rwi1(5);
// current forced usage
int value1 = 5;
std::reference_wrapper<int> rwi2(value1);
if(randomBool())
{
    int value2 = 7;// ES.5, ES.6
    rwi2 = ref(value2);// dangles with block scope
    rwi2 = ref(7);// ok, safe and easy with global scope proposal
    rwi2 = 7;// might make sense to add back direct assignment operator
}
else
{
    int value3 = 9;// ES.5, ES.6
    rwi2 = ref(value3);// dangles with block scope
    rwi2 = ref(9);// ok, safe and easy with global scope proposal
    rwi2 = 9;// might make sense to add back direct assignment operator
}
```

Since the variable `value2` and `value3` is likely to be created manually at block scope instead of variable scope, it can accidentally introduce more dangling. Constructing and reassigning with a `global scoped` `lvalue` temporary avoids these common dangling possibilities along with simplifying the code.

## Performance Considerations

<!-- x86-64 clang (trunk) -std=c++20 -O3 -->
<!-- x86-64 gcc   (trunk) -std=c++20 -O3 -->

There are at least three ways to provide a non dangling globalish constant.

- ROM i.e. hardware
- `const` and `static` i.e. `C++` language
- assembly opcode with inline constant i.e. machine code level

While the first two are addressable, the last one isn't.

In the next three examples, the same assembly is produced regardless of whether the literal `5` was provided via a native literal, a `constexpr` or a `const` global. The following results were produced in [Compiler Explorer](https://godbolt.org/) using both "`x86-64 clang (trunk) -std=c++20 -O3`" and "`x86-64 gcc (trunk) -std=c++20 -O3`".

### values that are [logically] global constants

#### local constant but logically a global constant

```cpp
int main()
{
    return 5;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

#### constant expression i.e. logically a global constant

```cpp
constexpr int return5()
{
    return 5;
}

int main()
{
    return return5();
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

#### an actual global

<!--
```cpp
int GLOBAL = 5;

int main()
{
    return GLOBAL;
}
```

```assembly
main:                                   # @main
        mov     eax, dword ptr [rip + GLOBAL]
        ret
GLOBAL:
        .long   5                               # 0x5
```
-->
```cpp
const int GLOBAL = 5;

int main()
{
    return GLOBAL;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

The point is all three are logically non dangling, constant global. Now let's look at reference examples.

### immediate dangling

Not only do all three following examples produce the exact same assembly, they also provide the exact same assembly as the previous three examples. They are all essentially global constants from the assembly and programmer standpoint but the current standard says two of the three dangle, unnecessarily.

#### local constant but logically a global constant

```cpp
int main()
{
    const int& reflocal = 5;// immediate dangling
    return reflocal;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

```cpp
int main()
{
    const int local = 5;
    const int& reflocal = local;
    return reflocal;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

#### constant expression i.e. logically a global constant

```cpp
constexpr int return5()
{
    return 5;
}

int main()
{
    const int& reflocal = return5();// immediate dangling
    return reflocal;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

#### an actual global

```cpp
const int GLOBAL = 5;

int main()
{
    const int& reflocal = GLOBAL;
    return reflocal;
}
```

```assembly
main:                                   # @main
        mov     eax, 5
        ret
```

### indirect dangling of caller's local

Similarly to, the next three examples produce the same assembly in the 3 `clang` cases and 2 of the `gcc` cases. `GCC` would have produced the same result in its 2nd case had it had treated the `const` expected evaluation of a constant expression as a global constant as its third case did. 

#### local constant but logically a global constant

```cpp
const int& potential_dangler(const int& passthrough)
{
    return passthrough;
}

int main()
{
    const int local = 5;
    const int& reflocal = potential_dangler(local);
    return reflocal;
}
```

```assembly
potential_dangler(int const&):               # @potential_dangler(int const&)
        mov     rax, rdi
        ret
main:                                   # @main
        mov     eax, 5
        ret
```

#### constant expression i.e. logically a global constant

```cpp
constexpr int return5()
{
    return 5;
}

const int& potential_dangler(const int& passthrough)
{
    return passthrough;
}

int main()
{
    const int& reflocal = potential_dangler(return5());
    return reflocal;
}
```

##### x86-64 clang (trunk) -std=c++20 -O3

```assembly
potential_dangler(int const&):               # @potential_dangler(int const&)
        mov     rax, rdi
        ret
main:                                   # @main
        mov     eax, 5
        ret
```

##### x86-64 gcc (trunk) -std=c++20 -O3

<span style="color:red">**NOTE: Can't really say what GCC is doing with the `xor`. However, if GCC had treated the resolved constant expression which is const required as a const global as in the next example than the results would have been the same.**</span>

```assembly
potential_dangler(int const&):
        mov     rax, rdi
        ret
main:
        xor     eax, eax
        ret
```

#### an actual global

```cpp
const int GLOBAL = 5;

const int& potential_dangler(const int& passthrough)
{
    return passthrough;
}

int main()
{
    const int& reflocal = potential_dangler(GLOBAL);
    return reflocal;
}
```

```assembly
potential_dangler(int const&):               # @potential_dangler(int const&)
        mov     rax, rdi
        ret
main:                                   # @main
        mov     eax, 5
        ret
```

In all these logically constant global cases, no instance was actually stored global but was perfectly inlined as an assembly opcode constant. So the worst case performance of this proposal is what would be a local that is constantly being created and destroyed, potentially multiple times concurrently in different threads, would be a single upfront load time cost. Even this cost can go from 1 to 0 while the current non global local could result in superfluous dynamic allocations since `std::string` and `std::vector` are now `constexpr`. In short, the compiler/language already has all it needs to fix dangling constants. Compilers are already doing this but there is currently no verbiage in the standard that state that anonymous constants don't dangle because they are logically a constant global.

## Tooling Opportunities

There area a couple tooling opportunities especially with respect to the `constinit` specifier.

- A command line and/or IDE tool could analyze the code for `const`, `constexpr`/`LiteralType` and constant-initialized and if the conditions matches automatically add the `constinit` specifier for code reviewers.
- Another command line and/or IDE tool could strip `constinit` specifier from any temporaries for programmers.

Combined they would form a `constinit` toggle which wouldn't be all that much different from whitespace and special character toggles already found in many IDE(s).

## Summary

The advantages to `C++` with adopting this proposal is manifold.

- Safer
  - Eliminate dangling of what should be constants
  - Reduce immediate dangling when the instance is a constant
  - Reduce returning direct reference dangling when the instance is a constant
  - Reduce returning indirect reference dangling when the instance is a constant and was provided as an argument
  - Reduce indirect dangling that can occur in the body of a function
  - Reduce unitialized and delayed initialization errors
  - Increases safety by avoiding data races.
- Simpler
  - Make constexpr literals less surprising for new and old developers alike
  - Reduce the gap between `C++` and `C99` compound literals
  - Improve the potential contribution of `C++`'s dangling resolutions back to `C`
  - Make string literals and `C++` literals more consistent with one another
  - Taking a step closer to reducing undefined behavior in string literals
  - Simplify the language to match existing practice
  - Consequently, a “cleanup”, i.e. adoption of simpler, more general rules/guidelines
- Faster & More Memory Efficient
  - Reduce unnecessary heap allocations
  - Increase and improve upon the utilization of ROM and the benefits that entails

## Frequently Asked Questions

### What about locality of reference?

It is true that globals can be slower than locals because they are farther in memory from the code that uses them. So let me clarify, when I say `static storage duration`, I really mean **logically** `static storage duration`. If a type is a `PODType`/`TrivialType` or `LiteralType` than there is nothing preventing the compiler from copying the global to a local that is closer to the executing code. Rather, the compiler **must** ensure that the instance is always available; **effectively** `static storage duration`.

Consider this from an processor and assembly/machine language standpoint. A processor usually has instructions that works with memory. Whether that memory is ROM or is logically so because it is never written to by a program, then we have constants.

```cpp
mov <register>,<memory>
```

A processor may also have specialized versions of common instructions where a constant value is taken as part of the instruction itself. This too is a constant. However, this constant is guaranteed closer to the code because it is physically a part of it.

```cpp
mov <register>,<constant>
mov <memory>,<constant>
```

What is more interesting is these two examples of constants have different value categories since the ROM version is addressable and the instruction only version, clearly, is not. It should also be noted that the later unnamed/unaddressable version physically can't dangle.

### Won't this break a lot of existing code?

NO, if any. To the contrary, code that is broken is now fixed. Code that would be invalid is now valid, makes sense and can be rationally explained. Let me summarize:

#### Let constants be constants / free your constants / implicit constant initialization

This feature not only changes the point of destruction but also the point of construction. Instances that were of automatic storage duration, are now of static storage duration. Instances that were temporaries, are no longer temporaries. Surely, something must be broken! From the earlier section "Present", subsection "C Standard Compound Literals". Even the `C++` standard recognized that their are other opportunities for constant initialization.

<table>
<tr>
<td>

`Working Draft, Standard for Programming Language C++` [^n4910]

**"*6.9.3.2 Static initialization [basic.start.static]*"**

"*3 An implementation is permitted to perform the initialization of a variable with static or thread storage duration as a static initialization even if such initialization is not required to be done statically ...*"

</td>
</tr>
</table>

So, what is the point? For the instances that would benefit from implicit constant initialization, their are currently NO guarantees as far as their lifetime and as such is indeterminite. With this portion of the proposal, a guarantee is given and as such that which was non determinite becomes determinite.

It should also be noted that while this enhancement is applied implicitly, programmers has opted into this up to three times.

1. The programmer of the type must have provided a means for the type to be constructed at compile time likely by having a `constexpr` constructor.
1. The programmer of the variable or function parameter must have stated that they want a `const`.
1. The end programmer have `const-initialized` the variable or argument.

Having expressed contant requirements three times, it is pretty certain that the end programmer wanted a constant, even if it is anonymous.

### Who would even use these features? Their isn't sufficient use to justify these changes.

Everyone ... Quite a bit, actually

Consider all the examples littered throughout our history, these are what gets fixed.

- dangling reported on normal use of the `STL`
- dangling examples reported in the `C++` standard
- real world dangling reported in NAD, not a defect, reports

This doesn't even include the countless examples found in numerous articles comparing `C++` with other nameless programming languages which would be fixed. However, the best proof can be found in our usage and other proposals.

<table>
<tr>
<td>

`C++ Core Guidelines` [^cppcgrfin]

***F.16: For "in" parameters, pass** cheaply-copied types by value and others **by reference to const***

</td>
</tr>
</table>

In `C++`, we use `const` parameters alot. This is the first of three requirements of `implicit constant initialization`. What about the use of types that can be constructed at compile time?

- **`C++20`**: `std::pair`, `std::tuple`, `std::string`, `std::vector`
- **`C++23`**: `std::optional`, `std::variant`, `std::unique_ptr`

As their was sufficient use to justify making the constructors of any one of these listed above types to be `constexpr` than their would be sufficient use of the `implicit constant initialization` feature which would use them all as this satisfies its second and third requirement that the instances be constructable at compile time and `constant-initialized`.

### Why not just use a static analyzer?

Typically a static analyzer doesn't fix code. Instead, it just produces warnings and errors. It is the programmer's responsibility to fix code by deciding whether the incident was a false positive or not and making the corresponding code changes. This proposal does fix some dangling but others go unresolved and unidentified. As such this proposal and static analyzers are complimentary. Combined this proposal can fix some dangling and a static analyzer could be used to identify what is remaining. As such those who still ask, "why not just use a static analyzer", might really be saying **this proposal's language enhancements might break their static analyzer**. To which I say, the standard dictates the analyzer, not the other way around. That is true for all tools. However, let's explore the potential impact of this proposal on static analyzers.

The `C++` language is complex. It stands to reason that our tools would have some degree of complexity, since they would need to take some subset of our language's rules into consideration. In any proposal, mine included, fixes to any dangling would result in potential dangling incidents becoming false positives between those identified by a static analyzer that overlap with said proposal. The false positives would join those that a static analyzer already has for not factoring existing language rules into consideration just as it would for any new language rules.

With `implicit constant initialization`, existing static analyzers would need to be enhanced to track the `const`ness of variables and parameters, whether or not the types of variables and parameters can be constructed at compile time and whether or not instances were constant-initialized. Until that happens, an existing dangling incident reported by static analyzer will just be a false positive. The total number of incidents remain the same and the programmer just need to recognize that it was a false positive which should be easy to do since constants are trivial and these rules are simple.

### Can this even be implemented?

`C++` already provides static storage duration guarantee for instances of one type and allows it for many others.

- native string literals already have static storage duration
- compilers have been free for a long time to promote compile time constructed instances to have static storage duration
- any `LiteralType` instances that are constant-initialized are already prime candidates for compilers to promote to having static storage duration

### Doesn't the `implicit constant initialization` feature make it harder for programmers to identify dangling and thus harder to teach?

If there was no dangling than there would be nothing to teach with respect to any dangling feature. Even the whole standard is not taught. So the more dangling we fix in the language, the less dangling that has to be taught to beginners. Consider the following example, does the new features make it easier or harder to identify dangling?

```cpp
f({1,2});// implicit constant initialization
// vs.
int i = 1;
f({i, 2});// no implicit constant initialization
```

It is plain to see that `{1,2}` is constant-initialized as it is composed entirely of `LiteralType`(s). It is also plain to see that `{i,2}` is modifiable as its initialization statement is variable and dynamic due to the variable `i`. So the real questions are as follows:

- Is the first parameter to the function `f` const?
- Is the type of the first parameter to the function `f` a `LiteralType`?

The fact is some programmer had to have known the answer to both questions in order to have writtern  `f({1,2})` in the first place. The case could be made that it would be nice to be able to use the `constinit` keyword on temporary arguments, `f(constinit {1, 2})`, as this would allow those who don't write the code, such as code reviewers, to quickly validate the code. Even the programmer would benefit, some, if the code was copied. However, `constinit` would mostly be superfluous, if the `temporaries are just anonymously named variables` feature is added. As such, `constinit` should be optional. Consequently, any negative impact upon identifying and teaching dangling is negligible.

Yet, both `implicit and explicit constant initialization` feature, by itself, makes it easier to identify and teach dangling.

**C++ Core Guidelines**<br/>**F.43: Never (directly or indirectly) return a pointer or a reference to a local object** [^cppcgrf43]

...

***Note** This applies only to non-static local variables. All static variables are (as their name indicates) statically allocated, so that pointers to them cannot dangle.* [^cppcgrf43]

Instances that have static storage duration can't dangle. Currently in `C++`, instances that don't immediately dangle can still dangle later such as by returning. Using `static storage duration` short circuits the dangling identification process. An instance, once identified, doesn't need to be factored into any additional dangling decision making process. Using more `static storage duration` speeds up the dangling identification process. This would also be of benefit to static analyzers that goes through a similar thought process.

### Doesn't this make C++ harder to teach?

Until the day that all dangling gets fixed, any incremental fixes to dangling still would require programmers to be able to identify any remaining dangling and know how to fix it specific to the given scenario, as there are multiple solutions. Since dangling occurs even for things as simple as constants and immediate dangling is so naturally easy to produce, <!--than--> dangling resolution still have to be taught, even to beginners. As this proposal fixes these types of dangling, it makes teaching `C++` easier because it makes `C++` easier.

So, what do we teach now and what bearing does these teachings, the `C++` standard and this proposal have on one another.

**C++ Core Guidelines**<br/>**F.42: Return a `T*` to indicate a position (only)** [^cppcgrf42]<br/>***Note** Do not return a pointer to something that is not in the caller’s scope; see F.43.* [^cppcgrf43]

Returning references to something in the caller's scope is only natural. It is a part of our reference delegating programming model. A function when given a reference does not know how the instance was created and it doesn't care as long as it is good for the life of the function call (and beyond).  Unfortunately, scoping temporary arguments to the statement instead of the containing block doesn't just create immediate dangling but it provides to functions references to instances that are near death. These instances are almost dead on arrival. Having the ability to return a reference to a caller's instance or a sub-instance thereof assumes, correctly, that reference from the caller's scope would still be alive after this function call. The fact that temporary rules shortened the life to the statement is at odds with what we teach. This proposal restores to some temporaries the lifetime of anonymously named constants which is not only natural but also consistent with what programmers already know. It is also in line with what we teach as was codified in the C++ Core Guidelines. One such is as follows:

**C++ Core Guidelines**<br/>**F.43: Never (directly or indirectly) return a pointer or a reference to a local object** [^cppcgrf43]

***Reason** To avoid the crashes and data corruption that can result from the use of such a dangling pointer.* [^cppcgrf43]

Other than turning some of these locals into globals, this proposal does not solve nor contradict this teaching. If anything, by cleaning up the simple dangling it makes the remaining more visible.

Further, what is proposed is easy to teach because we already teach it and it makes `C++` even easier to teach.

- We already teach that native string literals don't dangle because they have static storage duration. This proposal just extends the concept to constants, as expected. This increases good consistency and reduces a bifurcation that is currently taught.

All of this can be done without adding any new keywords or any new attributes. We just use constant concepts that beginners are already familiar with. In fact, we will would be working in harmony with all that we already teach about globals in the `Core C++ Guidelines` [^cppcg].

- `I.2: Avoid non-const global variables` [^cppcgrf42]
- `I.22: Avoid complex initialization of global objects` [^cppcgrf42]
- `F.15: Prefer simple and conventional ways of passing information` [^cppcgrf42]
- `F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const` [^cppcgrf42]
- `F.43: Never (directly or indirectly) return a pointer or a reference to a local object` [^cppcgrf43]
- `R.5: Prefer scoped objects, don’t heap-allocate unnecessarily` [^cppcgrf42]
- `R.6: Avoid non-const global variables` [^cppcgrf42]
- `CP.2: Avoid data races` [^cppcgrf42]
- `CP.24: Think of a thread as a global container` [^cppcgrf42]
- `CP.32: To share ownership between unrelated threads use shared_ptr` [^cppcgrf42]

### How do these specifiers propagate?

These specifiers apply to the temporary immediately to the right of said specifier and to any child temporaries. It does not impact any parent or sibling temporaries. Consider these examples:

```cpp
// all of the temporaries has the default temporary scope as
// specified by the module attribute otherwise statement scope
f({1, { {2, 3}, 4}, {5, 6} });
// only 4 has constinit scope
f({1, { {2, 3}, constinit 4}, {5, 6} });
// only {2, 3}, 2, 3 has constinit scope
f({1, { constinit {2, 3}, 4}, {5, 6} });
// only { {2, 3}, 4}, {2, 3}, 2, 3, 4 has constinit scope
f({1, constinit { {2, 3}, 4}, {5, 6} });
// all of the arguments have has constinit scope
f(constinit {1, { {2, 3}, 4}, {5, 6} });
// only 5 has constinit scope
f({1, { {2, 3}, 4}, {constinit 5, 6} });
```

## References

<!--temporary storage class specifiers-->
[^p2658r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2658r0.html>
<!--implicit constant initialization-->
[^p2623r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2623r2.html>
<!--Fix the range-based for loop, Rev2-->
[^p2012r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2012r2.pdf>
<!--Get Fix of Broken Range-based for Loop Finally Done-->
[^p2644r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2644r0.pdf>
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
[^cppcg]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
<!--C++ Core Guidelines-->
[^cppcgrfin]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-in>
<!--C++ Core Guidelines - F.42: Return a T* to indicate a position (only) -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f42-return-a-t-to-indicate-a-position-only>
<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgrf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>
<!--Simpler implicit move-->
[^p2266r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2266r3.html>

<!--C++ Core Guidelines - I.2: Avoid non-const global variables -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i2-avoid-non-const-global-variables>
<!--C++ Core Guidelines - I.22: Avoid complex initialization of global objects -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i22-avoid-complex-initialization-of-global-objects>

<!--C++ Core Guidelines - F.15: Prefer simple and conventional ways of passing information -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f15-prefer-simple-and-conventional-ways-of-passing-information>
<!--C++ Core Guidelines - F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f16-for-in-parameters-pass-cheaply-copied-types-by-value-and-others-by-reference-to-const>

<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgrf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>

<!--C++ Core Guidelines - R.5: Prefer scoped objects, don’t heap-allocate unnecessarily -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r5-prefer-scoped-objects-dont-heap-allocate-unnecessarily>
<!--C++ Core Guidelines - R.6: Avoid non-const global variables -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r6-avoid-non-const-global-variables>
<!--C++ Core Guidelines - CP.2: Avoid data races -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp2-avoid-data-races>
<!--C++ Core Guidelines - CP.24: Think of a thread as a global container -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp24-think-of-a-thread-as-a-global-container>
<!--C++ Core Guidelines - CP.32: To share ownership between unrelated threads use shared_ptr -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp32-to-share-ownership-between-unrelated-threads-use-shared_ptr>
<!--C++ Core Guidelines - Glossary - global variable -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#glossary>
<!-- global variable: technically, a named object in namespace scope. -->
<!--Introduce storage-class specifiers for compound literals-->
[^n3038]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3038.htm>
<!--The `constexpr` specifier-->
[^n2917]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2917.pdf>