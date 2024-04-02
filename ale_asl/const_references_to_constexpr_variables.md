<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3218R0</td>
</tr>
<tr>
<td>Date</td>
<td>2024-04-02</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Evolution Working Group (EWG)<br/>Core Working Group (CWG)</td>
</tr>
</table>

# const references to constexpr variables

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

- [const references to constexpr variables](#const-references-to-constexpr-variables)
  - [Abstract](#Abstract)
  - [Simpler](#Simpler)
  - [Safer](#Safer)
  - [Relationship to constexpr wrapper](#Relationship-to-constexpr-wrapper)
  - [Related Work](#Related-Work)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Abstract

This proposal is an addendum to the `"constexpr structured bindings and references to constexpr variables"` [^p2686r3] proposal with a focus on simplicity and safety.

## Simpler

The `P2686`, `constexpr structured bindings and references to constexpr variables` [^p2686r3], proposal outlines 4 solutions to its problem.

1. `"Allowing static and non-tuple constexpr structured binding"`
1. `"Making constexpr implicitly static"`
1. `"Always re-evaluate a call to get?"`
1. `"Symbolic addressing"`

Of these four, `P2686` [^p2686r3] recommends that the first and the last be implemented. Concerning the first, the authors wrote; "Independently of the other solutions presented here, this option would be useful and should be done". Concerning the last, the authors wrote; "The most promising option - the one we think should be pursued".

This proposal does not argue for or against the first and the last. To the contrary, this proposal argues that there is a place in at least some [limited] capacity for the second solution, `Making constexpr implicitly static`.

### given

```cpp
struct point
{
    int x;
    int y;
};
    
const point& dangler(const point& p)
{
    // ...
    return p;
}

const std::string& dangler(const std::string& s)
{
    // ...
    return s;
}
```

### usage

```cpp
// {4, 2} is static
const point& always_safe = {4, 2};
const point& safe = dangler({4, 2});
// "42"s is static
const std::string& always_safe = "42"s;
const std::string& safe = dangler("42"s);
```

NOTE: While this proposal is only concerned about const references to constant expressions implicity creating constants, it would make sense to end programmers for const variables as well.

<table>
<tr>
<td>

```cpp
// {4, 2} is static
const point& always_safe = {4, 2};
```

</td>
<td>

```cpp
// {4, 2} can be static
const point always_safe{4, 2};
```

</td>
</tr>
<tr>
<td>

```cpp
// "42"s is static
const std::string& always_safe = "42"s;
```

</td>
<td>

```cpp
// "42"s can be static
const std::string always_safe{"42"};
```

</td>
</tr>
</table>

It should be noted that the first option is simpler than the last, at least in wording but is more complex than the last in the sense that it explictly requires the programmer to litter their code with `static`. The second option is simpler than both the first and the last options because the static is implied just as it is at namespace scope resulting in simplified wording and simplified usage by the end programmers. Are the reasons presented against the second option totally legitimate or could those rare exceptions be handled in such a way that simplicity wouldn't be lost for the greater whole? Lets consider each point in turn.

<table>
<tr>
<td>

**constexpr structured bindings and references to constexpr variables** [^p2686r3]

***"1. Making constexpr implicitly static"***

*"We could make constexpr variables implicitly static, but doing so would most certainly break existing code, in addition to being inconsistent with the meaning of `constexpr`:"*

```cpp
int f() {
    constexpr struct S {
      mutable int m ;
    } s{0};
    return ++s.m;
}

int main() {
    assert(f() + f() == 2); // currently 2. Becomes 3 if 's' is made implicitly static
}
```

*"So this solution is impractical. We could make constexpr static only in some cases to alleviate some of the breakages or even make only constexpr bindings static, not other variables, but this option feels like a hack rather than an actual solution."*

</td>
</tr>
</table>

### *"doing so would most certainly break existing code"*

To the contrary, code that is broken is now fixed. Code that would be invalid is now valid, makes sense and can be rationally explained. Let me explain. This feature not only changes the point of destruction but also the point of construction. Instances that were of automatic storage duration, are now of static storage duration. Instances that were temporaries, are no longer temporaries. Surely, something must be broken! The `C++` standard already recognized that their are other opportunities for constant initialization.

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

Next, a function that takes an argument by reference does not know whether the caller of said function will pass to it a global, local, temporary or a dynamically created object. Rather it expects that said parameter would exist for the life of the function and if the reference is returned in part or in whole than the life of the object needs to be longer.

### *"inconsistent with the meaning of `constexpr`"*

Really! One only have to read the original motivations for `constexpr` to realize that `static` is consistent with its meaning.

#### n1511
    
<table>
<tr>
<td>

***`Literals for user-defined types`*** [^n1511]

**2003**

*"This note proposes a notion of **user-defined literals** based on literal constructors without requiring new syntax. If combined with the separate proposal for generalized initializer lists, it becomes a generalization of the **C99 notion of compound literals**."*

*"However, a constructor is a very general construct and there have been many requests for a way to express literals for user-defined types in such a way that a programmer can be **confident that a value <u>will be</u> constructed at compile time** and **potentially stored in ROM**. For example:"*

```cpp
complex z(1,2); // the variable z can be constructed at compile time
const complex cz(1,2); // the const cz can potentially be put in ROM
```

*"**Personally, I prefer (1): basically, a value is a literal if it is composed out of literals and implemented by a literal constructor. The problem with that is that some people will not trust compilers to do proper resolution, placement in ROM, placement in text segment**"*

*"**C99 compound literals**"*

*"In C99, it is explicitly allowed to take the address of a compound literal. For example:"*

```cpp
f(&(struct foo) { 1,2 });
```

*"**This makes sense only if we assume that the {1,2} is stored in a data segment (like a string literal**, but different from a int literal). **I see no problem allowing that**"*

*"**It would be tempting to expand this rule to user-defined literals bound to references.**"*

</td>
</tr>
</table>


#### n2235

<table>
<tr>
<td>

***`Generalized Constant Expressions—Revision 5`*** [^n2235]

**2007**

*"This paper generalizes the notion of constant expressions to include constant-expression functions and **user-defined literals**"*

*"The goal is ... to **increase C99 compatibility.**"*

...

*"**simplify the language** definition in the area of constant expression **to match existing practice**"*

...

*"**3. Static initialization of objects with static storage.**"*

*"... However **we strive for more uniform and consistency among related C++ language features and compatibility**"*

*"**3 Problems**"*

...

*"**3.4 Unexpected dynamic initialization**"*

*"**However, it is possible to be surprised by expressions that (to someone) “look const” but are not.**"*

*"**3.5 Complex rules for simple things**"*

*"The focus of this proposal is to address the issues mentioned in preceding sections. However, discussions in the Core Working Group at the Berlin meeting (April 2006) concluded that the current rules for integral constant expressions are too complicated, and source of several Defect Reports. **Consequently, a “cleanup”, i.e. adoption of simpler, more general rules is suggested.**"*

...

*"As for other const variables, storage need not be allocated for a constant expression datum, unless its address is taken."*

```cpp
// the &x forces x into memory
```

*"When the initializer for an ordinary variable (i.e. not a constexpr) happens to be a constant, the compiler can choose to do dynamic or static initialization (as ever)."*

*"Declaring a constructor constexpr will help compilers to identify static initialization and perform appropriate optimizations (like putting literals in read-only memory.) Note that since “ROM” isn’t a concept of the C++ Standard and what to put into ROM is often a quite subtle design decision, this proposal simply allows the programmer to indicate what might be put into ROM (constant-expression data) rather than trying to specify what actually goes into ROM in a particular implementation."*

*"We do not propose to make constexpr a storage-class-specifier because it can be combined with either static or extern or register, much like const."*

</td>
</tr>
</table>

One of the motivations of main motivations for `constexpr` was for objects to be placed in `ROM`. What is read only memory, if not const and static storage duration!

### *"the mutable example"*

```cpp
int f() {
    constexpr struct S {
      mutable int m ;
    } s{0};
    return ++s.m;
}

int main() {
    assert(f() + f() == 2); // currently 2. Becomes 3 if 's' is made implicitly static
}
```

It's a shame that mutable is permitted to be used in constant expressions as it is rarely used, actively discouraged and potentially disabled by future safety enhancements. It is barely rational to have mutation associated with something associated with ROM as that typically brings about segmentation faults. It is also unnatural since template, constexpr and consteval functions are C++'s closest thing to pure functions. One can hardly say in the example f() is pure if it sometimes return 1 and another time returns 2 given the same arguments. The damage is done but also mostly irrelevant with respect to this proposal. We'll see this in the next point but before moving on to it keep in mind that the example is not an example of taking a reference to constexpr variable more less a const reference to constexpr variable.

### *"We could make constexpr static only in some cases to alleviate some of the breakages or even make only constexpr bindings static, not other variables, but this option feels like a hack rather than an actual solution."*

Actually, no it doesn't. It is rather consistent with what is already proposed for C++26.

<table>
<tr>
<td>

**P2752R3<br/>Static storage for braced initializers** [^p2752r3]

**4. Mutable members**

...

Vendors are expected to deal with this by simply disabling their promote-to-shared-storage optimization when the element type (recursively) contains any mutable bits.

...

**7. Proposed wording relative to the current C++23 draft**

...

[Example 12:

```cpp
    void f(std::initializer_list<double> il);
    void g(float x) {
      f({1, x, 3});
    }
    void h() {
      f({1, 2, 3});
    }

    struct A {
      mutable int i;
    };
    void q(std::initializer_list<A>);
    void r() {
      q({A{1}, A{2}, A{3}});
    }
```

The initializations can be implemented in a way roughly equivalent to this:

```cpp
    void g(float x) {
      const double __a[3] = {double{1}, double{x}, double{3}}; // backing array
      f(std::initializer_list<double>(__a, __a+3));
    }
    void h() {
      static constexpr double __b[3] = {double{1}, double{2}, double{3}}; // backing array
      f(std::initializer_list<double>(__b, __b+3));
    }
    void r() {
      const A __c[3] = {A{1}, A{2}, A{3}}; // backing array
      q(std::initializer_list<A>(__c, __c+3));
    }
```

</td>
</tr>
</table>

This already accepted solution to the mutable problem could also be used as the solution to the mutable problem of this proposal.

## Safer

Besides simplicity, why does this proposal recommend that const references to constexpr variables have static storage duration? One word, safety. These objects would be impossible to dangle and also would be thread safe. This also fixes one of C++'s most embarassing forms of dangling; dangling constants. This problem is unique to C++. Practically all languages, except C++, doesn't immediately dangle their constants, not even assembly, C, or even the earliest version of C++, cfront. Things are even worse in C++ because as programmers we can't look at the code and know for sure whether an object is static or not. This requires C++ programmers even beginners to have to look at machine code to see how the compiler decided where to store the object. Not only does this vary from compiler to compiler, it also varies within any given compiler. Frequently in optimized release builds these constant objects are given static duration but made a dangling local in debug builds. This is totally backwards as the increased safety is expected in debug builds. Futher if the optimized is the safest and the better performant than why shouldn't it be available always. Dangling constants is like returning from a function using an input parameter before C++11 because programmers did not have sufficient assurance that rvalue moving would occur. Similarly, programmers have to name the currently unnamed and move it far from the point of use just to ensure dangling doesn't occur in both debug and release builds. This habit works against the recommendation to use unnamed [temporary] and move semantics. 

## Relationship to constexpr wrapper

This proposal is related to the `std::constexpr_wrapper` [^p2781r4] proposal on the safety level. While that proposal explicity creates safe anonymously named constants as a library, this proposal would be an implicit language feature. As a language feature, there are a couple of advantages.

1. `std::constexpr_wrapper` [^p2781r4] only works with structural types as those are currently the only types permitted for non type template parameters. This proposal can work with all types that haven't deleted their reference of operators.
1. simpler and more consistent, not having to resort to different libraries for structural and non structural types

<table>
<tr>
<td>&nbsp;</td>
<td>

`this proposal`

</td>
<td>

`std::constexpr_wrapper` [^p2781r4]

</td>
</tr>    
<tr>
<td>

`structural type`

</td>
<td>

```cpp
// {4, 2} is static
const point& safe = dangler({4, 2});
```

</td>
<td>

```cpp
// {4, 2} is a temporary
const point& safe = dangler(std::cw<{4, 2}>);
```

</td>
</tr>    
<tr>
<td>

non `structural type`

</td>
<td>

```cpp
// "42"s is static
const std::string& safe = dangler("42"s);
```

</td>
<td>

```cpp
// "42"s is a temporary
const std::string& safe = dangler([] /*consteval*/ -> auto const & { static constinit const std::string anonymous{"42"}; return g; }());
```

</td>
</tr>    
</table>

As demonstrated in the previous examples, the result of this proposal is that the same simple syntax is used regardless of whether the type was structural or not, while the current state of affairs require using a lambda function in order to provide an anonymously named constant.

## Related Work

A similar but more complicated feature exists in the Rust programming languge known as `rvalue static promotion` [^rvalue_static_promotion]. This makes sense for Rust as all types are by default movable. Similarly, the proposed feature would work for C++ since by default all types are referable.

```rust
let x: &'static u32 = &42;
```

The proposed feature would be simpler than `rust` as it doesn't require an explicit `'static` lifetime but rather would be implied because the would be temporary was `constexpr` capable, initialized with only constants and was initially assigned to a `const &`.

More verbose information on the motivation for this proposed feature also exists in the previous `constant dangling` [^p2724r1] proposal.

## Summary
    
"Making constexpr implicitly static" [^p2686r3] for constants make the language simpler and at least for constant references makes the language safer. Like the `Static storage for braced initializers` [^p2752r3] proposal it would minimize "CPU cycles", "stack space" and heap allocations caused by types that dynamically allocate such `std::string` and `std::vector` when they are better off being a constant.

## Frequently Asked Questions

### Who would even use these features? Their isn't sufficient use to justify these changes.

The best proof can be found in our usage and other proposals.

<table>
<tr>
<td>

**C++ Core Guidelines**
***F.16: For "in" parameters, pass** cheaply-copied types by value and others **by reference to const*** [^cppcgrfin]

</td>
</tr>
</table>

In `C++`, we use `const` parameters alot. This is the first of three requirements of `implicit constant initialization`. What about the use of types that can be constructed at compile time?

- **`C++20`**: `std::pair`, `std::tuple`, `std::string`, `std::vector`
- **`C++23`**: `std::optional`, `std::variant`, `std::unique_ptr`

As their was sufficient use to justify making the constructors of any one of these listed above types to be `constexpr` than their would be sufficient use of the `implicit constant initialization` feature which would use them all as this satisfies its second and third requirement that the instances be constructable at compile time and `constant-initialized`.

### Doesn't this make C++ harder to teach?

Until the day that all dangling gets fixed, any incremental compile time fixes to dangling still would require programmers to be able to identify any remaining dangling and know how to fix it specific to the given scenario, as there are multiple solutions. Since dangling occurs even for things as simple as constants than dangling resolution still have to be taught, even to beginners. As this proposal fixes these types of dangling, it makes teaching `C++` easier because it makes `C++` easier.

So, what do we teach now and what bearing does these teachings, the `C++` standard and this proposal have on one another.

<table>
<tr>
<td>

**C++ Core Guidelines**<br/>**F.42: Return a `T*` to indicate a position (only)** [^cppcgrf42]<br/>***Note** Do not return a pointer to something that is not in the caller’s scope; see F.43.* [^cppcgrf43]

</td>
</tr>
</table>

Returning references to something in the caller's scope is only natural. It is a part of our reference delegating programming model. A function when given a reference does not know how the instance was created and it doesn't care as long as it is good for the life of the function call (and beyond).  Unfortunately, scoping temporary arguments to the statement instead of the containing block doesn't just create immediate dangling but it provides to functions references to instances that are near death. These instances are almost dead on arrival. Having the ability to return a reference to a caller's instance or a sub-instance thereof assumes, correctly, that reference from the caller's scope would still be alive after this function call. The fact that temporary rules shortened the life to the statement is at odds with what we teach. This proposal extends the lifetimes of some temporaries, making them global which is not only natural but also consistent with what programmers already know. It is also in line with what we teach as was codified in the C++ Core Guidelines.

Other types of dangling can still occur. One simple type is directly called out in the C++ Core Guidelines.

<table>
<tr>
<td>

**C++ Core Guidelines**<br/>**F.43: Never (directly or indirectly) return a pointer or a reference to a local object** [^cppcgrf43]

***Reason** To avoid the crashes and data corruption that can result from the use of such a dangling pointer.* [^cppcgrf43]

...

***Note** This applies only to non-static local variables. <u>All static variables</u> are (as their name indicates) statically allocated, so that pointers to them <u>cannot dangle</u>.* [^cppcgrf43]

</td>
</tr>
</table>

Other than turning some of these locals into globals, this proposal does not solve nor contradict this teaching. If anything, by cleaning up, the remaining dangling is made all the more visible.

Further, what is proposed is easy to teach because we already teach it and it makes `C++` even easier to teach.

- We already teach that native string literals don't dangle because they have static storage duration. This proposal just extends the concept to constants, as expected. This increases good consistency and reduces a bifurcation that is currently taught.
- Somehow, we already teach the temporary lifetime extension rules which consist of numerous paragraphs and exception examples. This is just another lifetime extension but instead of extending an object's lifetime from being scoped to the statement to instead be scoped globally and not the block.

All of this can be done without adding any new keywords or any new attributes. We just use constant concepts that beginners are already familiar with. In fact, we will would be working in harmony with all that we already teach about globals in the `Core C++ Guidelines` [^cppcg].

- `I.2: Avoid non-const global variables` [^cppcgi2]
- `I.22: Avoid complex initialization of global objects` [^cppcgi22]
- `F.15: Prefer simple and conventional ways of passing information` [^cppcgf15]
- `F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const` [^cppcgf16]
- `F.43: Never (directly or indirectly) return a pointer or a reference to a local object` [^cppcgrf43]
- `R.5: Prefer scoped objects, don’t heap-allocate unnecessarily` [^cppcgr5]
- `R.6: Avoid non-const global variables` [^cppcgr6]
- `CP.2: Avoid data races` [^cppcgcp2]
- `CP.24: Think of a thread as a global container` [^cppcgcp24]
- `CP.32: To share ownership between unrelated threads use shared_ptr` [^cppcgcp32]

## References

<!--constexpr structured bindings and references to constexpr variables-->
[^p2686r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2686r3.pdf>
<!--Static storage for braced initializers-->
[^p2752r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2752r3.html>
<!--std::constexpr_wrapper-->
[^p2781r4]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2781r4.html>
<!--rvalue_static_promotion-->
[^rvalue_static_promotion]: <https://rust-lang.github.io/rfcs/1414-rvalue_static_promotion.html>
<!--constant dangling-->
[^p2724r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2724r1.html>
<!--Literals for user-defined types-->
[^n1511]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1511.pdf>
<!--Generalized Constant Expressions—Revision 5-->
[^n2235]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf>
<!--Working Draft, Standard for Programming Language C++-->
[^n4910]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4910.pdf>
<!--C++ Core Guidelines: Programming at Compile Time with constexpr-->
[^guidelines]: <https://www.modernescpp.com/index.php/c-core-guidelines-programming-at-compile-time-with-constexpr>
<!--C++ Core Guidelines-->
[^cppcg]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
<!--C++ Core Guidelines-->
[^cppcgrfin]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-in>
<!-- I.2: Avoid non-const global variables -->
[^cppcgi2]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i2-avoid-non-const-global-variables>
<!-- I.22: Avoid complex initialization of global objects -->
[^cppcgi22]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i22-avoid-complex-initialization-of-global-objects>
<!-- F.15: Prefer simple and conventional ways of passing information -->
[^cppcgf15]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f15-prefer-simple-and-conventional-ways-of-passing-information>
<!-- F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const -->
[^cppcgf16]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f16-for-in-parameters-pass-cheaply-copied-types-by-value-and-others-by-reference-to-const>
<!--C++ Core Guidelines - F.42: Return a T* to indicate a position (only) -->
[^cppcgrf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f42-return-a-t-to-indicate-a-position-only>
<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgrf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>
<!-- R.5: Prefer scoped objects, don’t heap-allocate unnecessarily -->
[^cppcgr5]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r5-prefer-scoped-objects-dont-heap-allocate-unnecessarily>
<!-- R.6: Avoid non-const global variables -->
[^cppcgr6]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r6-avoid-non-const-global-variables>
<!-- CP.2: Avoid data races -->
[^cppcgcp2]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp2-avoid-data-races>
<!-- CP.24: Think of a thread as a global container -->
[^cppcgcp24]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp24-think-of-a-thread-as-a-global-container>
<!-- CP.32: To share ownership between unrelated threads use shared_ptr -->
[^cppcgcp32]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp32-to-share-ownership-between-unrelated-threads-use-shared_ptr>
<!--
<table>
<tr>
<td>

**I.2: Avoid non-const global variables**[^cppcgrf42]

**Reason** Non-const global variables hide dependencies and make the dependencies subject to unpredictable changes.[^cppcgrf42]

</td>
</tr>
</table>

-->
