<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2878R4</td>
</tr>
<tr>
<td>Date</td>
<td>2023-7-4</td>
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

# Reference checking

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

- [Reference checking](#Reference-checking)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivational Example](#Motivational-Example)
  - [Motivation](#Motivation)
  - [Other Proposals](#Other-Proposals)
    - [Impact on general lifetime safety](#Impact-on-general-lifetime-safety)
    - [Impact on Contracts](#Impact-on-Contracts)
  - [Technical Details](#Technical-Details)
    - [Structs and Classes](#Structs-and-Classes)
    - [Lambdas revisited](#Lambdas-revisited)
    - [Impact on Pattern Matching](#Impact-on-Pattern-Matching)
  - [Resolution](#Resolution)
  - [Usage](#Usage)
  - [Viral Attribution Effort](#Viral-Attribution-Effort)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Changelog

### R4

- revised recursive example
- minor formatting changes
- improved FAQ: Why not pointers?

### R3

- added resolution to the do expression/pattern matching dangling problem
- added check for assigning a temporary to a named variable produces an error
- added recursive function call example to illustrate functions being analyzed in isolation
- added the `Lambdas revisited` example
- added a `Usage` section
- added a `Viral Attribution Effort` section

### R2

- fixed `f3` example of returning a `struct`
- revised comments of `4th check` example
- added resolved resolution example

## Abstract

This paper proposes that we allow programmers to provide explicit lifetime dependence information to the compiler for the following reasons:

- Standardize the documentation of lifetimes of API(s) for developers
- Standardize the specification of lifetimes for proposals
- Greatly reduce the dangling of the stack for references

This paper is **NOT** about the following:

- Fixing dangling of the stack that can only be discovered at runtime
- Directly fixing danging of the heap when not using RAII
- Fixing dangling that can occur when using pointers and pointer like types

Rather it is about making those instances of dangling references to the stack which are always bad code, detectable as errors in the language, instead of warnings.

## Motivational Example

<span style="color:red">***Disclaimer: I am not a RUST expert. Any RUST examples provided here is to illustrate this feature as a language feature instead of an attribute. This proposal will repeatedly refer to this as an attribute but there is no preference on which, rather it is important to gain the functionality.***</span>

What is being asked for is similar to but not exactly like Rust's feature called `explicit lifetimes`.

Example taken from `Why are explicit lifetimes needed in Rust?` [^so31609137]
```rust
fn foo<'a, 'b>(x: &'a u32, y: &'b u32) -> &'a u32 {
    x
}
```

Similar but better functionality has been requested in a variety of C++ proposals.

<!--indirect dangling identification-->
[^p2742r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2742r2.html>

1. `Bind Returned/Initialized Objects to the Lifetime of Parameters, Rev0` [^p0936r0]

```cpp
const unsigned int& foo(const unsigned int& x lifetimebound, const unsigned int& y) {
   return x;
}
```

2. `indirect dangling identification` [^p2742r2]

```cpp
[[parameter_dependency(dependent{"return"}, providers{"x"})]]
const unsigned int& foo(const unsigned int& x, const unsigned int& y) {
   return x;
}
```

3. `Towards memory safety in C++` [^p2771r0]

```cpp
[[dependson(x)]] const unsigned int& foo(const unsigned int& x, const unsigned int& y) {
   return x;
}
```

<!--Towards memory safety in C++-->
[^p2771r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2771r0.html>

Rust also allows providing `explicit lifetimes` on struct(s) but that is **NOT** being asked for in this proposal.

Example taken from `Why are explicit lifetimes needed in Rust?` [^so31609137]
```rust
struct Foo<'a> {
    x: &'a i32,
}

fn main() {
    let f : Foo;
    {
        let n = 5;  // variable that is invalid outside this block
        let y = &n;
        f = Foo { x: y };
    };
    println!("{}", f.x);
}
```

## Motivation

Having these checks in the language is highly desirable because it is highly effective. Other proposals are advocating for a seperate tool whether that be a static analysis tool or a runtime tool. Those solutions, while needed, are less effective because they are at best only `PPE`, personal protective equipment that only works if you have it installed, turned on, configured, used and acted upon. This proposal, while limited, is highly effective because it eliminates these instances of safety issues as illustrated on the *[exponential]* safety scale [^cdchoc].

![Hierarchy of Controls](https://upload.wikimedia.org/wikipedia/commons/3/36/NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg)

This proposal is also in line with what that C++ community believes and teaches.

<table>
<tr>
<td>

*"**In.force: Enforcement**"* [^cppcg-inforce-enforcement]

...

*"This adds up to quite a few dilemmas. We try to resolve those using tools. Each rule has an **Enforcement** section listing ideas for enforcement. **Enforcement might be done by code review, by static analysis, by compiler, or by run-time checks. Wherever possible, we prefer 'mechanical' checking (humans are slow, inaccurate, and bore easily) and static checking. Run-time checks are suggested only rarely where no alternative exists**; we do not want to introduce 'distributed bloat'."* [^cppcg-inforce-enforcement]

...

**P.5: Prefer compile-time checking to run-time checking** [^cppcg-p5-prefer-compile-time-checking-to-run-time-checking]

---

**C++ Core Guidelines** [^cppcg-inforce-enforcement]<br/>*Bjarne Stroustrup, Herb Sutter*

</td>
</tr>
</table>


## Other Proposals

Before going into the technical details, it would be good to consider this in light of other safety related proposals. 

### Impact on general lifetime safety

This proposal does not conflict with `Lifetime safety: Preventing common dangling` [^p1179r1]. To the contrary, it is complimentary.

<table>
<tr>
<td>

*"**1.1.4 Function calls**"*
*"Finally, since **every function is analyzed in isolation**, we have to have some way of reasoning about function calls when a function call returns a Pointer. **If the user doesn’t annotate** otherwise, **by default we assume that a function returns values that are derived from its arguments.**"*


---

**Lifetime safety: Preventing common dangling** [^p1179r1]<br/>*Herb Sutter*

</td>
</tr>
</table>

Just like the `Lifetime safety: Preventing common dangling` [^p1179r1] paper, this proposal analyze each function in isolation. While that proposal is focused on when programmers do not annotate/document their code, this proposal is focused on when programmers do document their code. That proposal "*assume that a function returns values that are derived from its arguments*" and because of this assumption has to produce warnings. This proposal makes no assumption at all and consequently can produce errors from the library authors documented intentions. This is both complimentary, independent and both proposals are desired by the C++ community. Since both related features are independent than there is no reason why the compile checks proposal couldn't be added before the runtime checks proposal.

### Impact on Contracts

This proposal also does not conflict with contracts. Matter of fact this proposal could be described in terms of contracts, just applied to existing language features. Since, none of the current contract proposals allow applying contracts to existing and future non functional language features such as `return` and `do return` [^p2806r1], this proposal is complimentary. 

<table>
<tr>
<td>

*"**7.1 LIFETIME**"*
*"These sources of undefined behavior pertain to accessing an object outside its lifetime or validity of a pointer. By their very nature, they are not directly syntactic. **The approach suggested in this proposal is to prohibit the use of certain syntactic constructs which might – under the wrong circumstances -- lead to undefined behavior. Those restrictions are syntactic, so clearly will prohibit cases that someone might find useful.**"* [^p2680r1]

---

**Contracts for C++: Prioritizing Safety** [^p2680r1]<br/>*Gabriel Dos Reis*

</td>
</tr>
</table>

The `Contracts for C++: Prioritizing Safety` [^p2680r1] proposal envisions a predicate called `object_address` that could be applied via contracts to functions. In contract like terms, this proposal would be advocating for `is_global_object_address`, `is_local_object_address`, `is_temporary_object_address`, `is_not_global_local_temporary_object_address` and then applying these predicates to `return`, reference dereference and the points of reference use. Since all of this is compile time information and the places where applied would always produce errors than there is no need for the programmer to add such checks anywhere because the compiler can do it automatically. Further, as this proposal only serve to identify code that is definitely bad, then this proposal does not "*prohibit cases that someone might find useful*".

## Technical Details

In order to make this proposal work, 2 bits of information is needed per reference at compile time. The 2 bits represents an enumeration of 4 possible lifetime values. 

1. Is global
1. Is local
1. Is temporary
1. Is all other i.e. not global, local or temporary? i.e. unknown, `nullptr` and dynamic

This lifetime enumeration gets associated with each reference at the point of construction since references have to be initialized, can't be `nullptr` and can't be rebound.

```cpp
const int GLOBAL = 42;

void f(int* ip, int& ir/* unknown lifetime */)
{
    int local = 42;
    int& r1 = *ip;// unknown lifetime
    int& r2 = ir;// unknown lifetime
    int& r3 = GLOBAL;// global lifetime
    int& r4 = local;// local lifetime
}
```

The next step is copying a reference copies its lifetime metadata.

```cpp
const int GLOBAL = 42;

void f(int* ip, int& ir/* unknown lifetime */)
{
    int local = 42;
    int& r1 = *ip;// unknown lifetime
    int& r2 = ir;// unknown lifetime
    int& r3 = GLOBAL;// global lifetime
    int& r4 = local;// local lifetime
    int& r5 = r1;// unknown lifetime
    int& r6 = r2;// unknown lifetime
    int& r7 = r3;// global lifetime
    int& r8 = r4;// local lifetime
}
```

**1st check: Returning a reference to a local produces an error.**

```cpp
const int GLOBAL = 42;

int& f(int* ip, int& ir/* unknown lifetime */)
{
    int local = 42;
    int& r1 = *ip;// unknown lifetime
    int& r2 = ir;// unknown lifetime
    int& r3 = GLOBAL;// global lifetime
    int& r4 = local;// local lifetime
    int& r5 = r1;// unknown lifetime
    int& r6 = r2;// unknown lifetime
    int& r7 = r3;// global lifetime
    int& r8 = r4;// local lifetime
    return r8;// error: dangling a local
}
```

This error doesn't give programmers much as C++ addressed this in C++23 with `Simpler implicit move` [^p2266r3]. However, since `Simpler implicit move` [^p2266r3] was framed in terms of value categories than any error message would also be in terms of value categories. This proposal advises such an error would be expressed in terms of dangling which is more human readable for programmers.

Things get really interesting when programmers are allowed to provide explicit lifetime dependence information to the compiler. Unlike Rust's `explicit lifetimes`, this feature, `explicit lifetime dependence`, allows a reference to be tied to the lifetimes of multiple other references. In these cases, the lifetime is the most constrained as in `temporary` is more constrained than `local` which is more constrained than `global`.

    global > local > temporary

This is also time to add three more checks.

**2nd check: Returning a reference to a `temporary` produces an error.
<br/>3rd check: Using a reference to a `temporary` after it has been assigned, i.e. on another line of code which is not the full-expression, produces an error.<br/>4th check: Assigning a `temporary` to a named variable produces an error.**

```cpp
const int GLOBAL = 42;

[[dependson(left, right)]]
const int& f1(const int& left/* unknown lifetime */, const int& right/* unknown lifetime */)
{
    if(randomBool())
    {
        return left;
    }
    else
    {
        return right;
    }
}

int& f2()
{
    int local = 42;
    const int& r1 /*local*/ = f1(local, local);
    const int& r2 /*global*/ = f1(GLOBAL, GLOBAL);
    const int& r3 /*temporary*/ = f1(42, 42);// error: can't assign temporary to a named instance
    const int& r4 /*local*/ = f1(local, GLOBAL);
    const int& r5 /*temporary*/ = f1(local, 42);// error: can't assign temporary to a named instance 
    const int& r6 /*temporary*/ = f1(GLOBAL, 42);// error: can't assign temporary to a named instance 
    if(randomBool())
    {
        return r1;// error: can't return local
    }
    if(randomBool())
    {
        return r2;// OK its a reference to a global
    }
    if(randomBool())
    {
        return r3;// error: can't return temporary
    }
    if(randomBool())
    {
        return r4;// error: can't return local
    }
    if(randomBool())
    {
        return r5;// error: can't return temporary
    }
    if(randomBool())
    {
        return r6;// error: can't return temporary
    }
    int x1 = r3 + 43;// error: can't use reference to a temporary
    int x2 = r5 + 44;// error: can't use reference to a temporary
    int x3 = r6 + 45;// error: can't use reference to a temporary
    return f1(f1(GLOBAL, 4), f1(local, 2));// error: can't return temporary
}
```

Besides fixing indirect dangling of a local, this also fixes indirect dangling of temporaries which causes immediate dangling.

<table>
<tr>
<td>

**6.7.7 Temporary objects [class.temporary]**

...

<sub>4</sub> When an implementation introduces a temporary object of a class that has a non-trivial constructor (11.4.5.2, 11.4.5.3), it shall ensure that a constructor is called for the temporary object. Similarly, the destructor shall be called for a temporary with a non-trivial destructor (11.4.7). **Temporary objects are destroyed as the last step in evaluating the full-expression** (6.9.1) that (lexically) contains the point where they were created. This is true even if that evaluation ends in throwing an exception. The value computations and side eﬀects of destroying a temporary object are associated only with the full-expression, not with any specifc subexpression.

...

(6.12) - A temporary bound to a reference in a *new-initializer* (7.6.2.8) persists until the completion of the **full-expression** containing the *new-initializer*.

[*Note 7*: **This might introduce a dangling reference.** - *end note*]
<!--
[*Example 5*:

```cpp    
struct S { int mi; const std::pair<int,int>& mp; };
S a { 1, {2,3} };
S* p = new S{ 1, {2,3} }; // creates dangling reference
```

-- *end example*]
-->
---

**Working Draft, Standard for Programming Language C++** [^n4910]

</td>
</tr>
</table>

Before going further, let's consider a recursive example in order to illustrate why each function is considered in isolation.

```cpp
const int GLOBAL = 42;

[[dependson(input)]]
const int& recursive(const int& input/* unknown lifetime */)
{
    if(randomBool())
    {
        return GLOBAL;// globals always fine
    }
    int local = randomInt();
    if(randomBool())
    {
        return local;// error: can't return local
    }
    if(randomBool())
    {
        return 42;// error: can't return temporary
    }
    if(randomBool())
    {
        return input;// handled by attribute lower on call stack
    }
    return recursive(input);// unknown
    // doesn't matter
    // local's and temporaries lower on the stack are safe
    // dynamic would have to be cleaned up after the fact anyway
    // and if dynamic was deleted prior than that would be a problem
    // prior to this call; in all cases caller's responsibility
}

int& f()
{
    int local = 42;
    const int& r1 /*global*/ = recursive(GLOBAL);
    const int& r2 /*local*/ = recursive(local);
    const int& r3 /*temporary*/ = recursive(42);// error: can't assign temporary to a named instance
    if(randomBool())
    {
        return r1;// OK, its a reference to a global
    }
    if(randomBool())
    {
        return r2;// error: can't return local
    }
    if(randomBool())
    {
        return r3;// error: can't return temporary
    }
    int x1 = r3 + 43;// error: can't use reference to a temporary
    return recursive(42);// error: can't return temporary
}
```

If we fixed nothing else identified in this proposal, this would be a welcome reprieve. However, much more can be done simply. Let's say, instead of adding this lifetime metadata to each reference, we add it to each instance. For references, lifetime metadata would still say that the reference refers to an instance with a particular lifetime but for non reference and non pointer instances, lifetime metadata would indicate that the instance is dependent upon another instance. Let's see what type of dangling this would mitigate.

### Structs and Classes

Before adding this metadata to instances in general, consider a struct that contains references. This shows that the composition and decomposition of references can be handled by the compiler provided the reference is accessible such as `public`.

```cpp
struct S { int& first; const int& second; };

const int GLOBAL = 42;

[[dependson(left, right)]]
const int& f1(const int& left/* unknown lifetime */, const int& right/* unknown lifetime */)
{
    if(randomBool())
    {
        return left;
    }
    else
    {
        return right;
    }
}

int& f2()
{
    int local = 42;
    S s1{GLOBAL, local};
    S s2{local, f1(GLOBAL, 24)};// error: can't assign temporary to a named instance
    const int& r1 /*global*/ = s1.first;
    const int& r2 /*local*/ = s1.second;
    const int& r3 /*local*/ = s2.first;
    const int& r4 /*temporary*/ = s2.second;// error: can't assign temporary to a named instance
    if(randomBool())
    {
        return r1;// OK its a reference to a global
    }
    if(randomBool())
    {
        return r2;// error: can't return local
    }
    if(randomBool())
    {
        return r3;// error: can't return local
    }
    if(randomBool())
    {
        return r4;// error: can't return temporary
    }
    int x = r4 + 43;// error: can't use reference to a temporary
    return 42;// error: can't return temporary
}

S f3()
{
    int local = 42;
    S s1{GLOBAL, local};
    S s2{local, f1(GLOBAL, 24)};// error: can't assign temporary to a named instance
    if(randomBool())
    {
        return s1;// error: still returning a local
    }
    return s2;// error: still returning a local
    // error: still returning a temporary
}

// prevents returning lambda that captures or returns a local or temporary
auto lambda()
{
    int local = 42;
    const int& ref_temporary = f1(GLOBAL, 24);// error: can't assign temporary to a named instance
    return [&local, &ref_temporary]() -> const int&
    {
        if(randomBool())
        {
            return local;// error: can't return local
        }
        return ref_temporary;// error: can't return temporary
    };// error: still returning a local
    // error: still returning a temporary
}

// prevents returning coroutine that captures or returns a local or temporary
auto coroutine()
{
    int local = 42;
    const int& ref_temporary = f1(GLOBAL, 24);// error: can't assign temporary to a named instance
    return [&local, &ref_temporary]() -> generator<const int&>
    {
        if(randomBool())
        {
            co_return local;// error: can't return local
        }
        co_return ref_temporary;// error: can't return temporary
    };// error: still returning a local
    // error: still returning a temporary
}
```

Is there any way we can mitigate dangling when the reference has been hidden by abstractions such as for reference like classes via `protected`, `private` access and `public` accessors. If lifetime metadata are also appied to instances in general, than constructors, conversion operators and factory functions could be annotated to say a returned reference like type is dependent upon another type. Consider the `std::string_view` and `sts::string`.

**GIVEN**

```cpp
constexpr std::string::operator [[dependson(this)]] std::basic_string_view<CharT, Traits>() const noexcept;
```

```cpp
std::string_view sv /* dependent upon temporary */ = "hello world"s;// error: can't assign temporary to a named instance
sv.size();// error: can't use a temporary outside of the line it was created on
```

This would also work with other reference like types such as `std::span` and `function_ref` [^p0792r14]. Just as this proposal does not address pointers because they are more run time than compile time, this proposal would not address `std::reference_wrapper` since it is rebindable. Nor would this proposal address, `std::shared_ptr` and `std::unique_ptr` which like pointers are nullable and rebindable, even though they already have runtime safeties built in.<!--However, if the standard ever adopted shared_ref and unique_ref which would have to be initialized to a non-null and wasn't rebindable than this proposal would be invaluable.-->

Even though this proposal does not address pointers and pointer like types, it is still useful to non owning versions of these constructs when they are const constructed because they would need to be non null initialized to be usable and wouldn't be rebindable because they were `const`. For instance, `std::reference_wrapper` is rebindable like a pointer so it tends to be more runtime than compile time. However, a `const std::reference_wrapper` can't be rebound so it would make sense if a programmer bound its lifetime to another instance.

```cpp
[[dependson(left, right)]]
const std::reference_wrapper<const int> f(const int& left/* unknown lifetime */, const int& right/* unknown lifetime */)
{
    if(randomBool())
    {
        return std::cref(left);
    }
    else
    {
        return std::cref(right);
    }
}
```

This is also time to mention our final check.

**5th check: You can't use a temporary in a `new` expression if the type being instantiated will become dependent upon that temporary.**

<table>
<tr>
<td>

**6.7.7 Temporary objects [class.temporary]**

...

<sub>4</sub> ... (11.4.7). **Temporary objects are destroyed as the last step in evaluating the full-expression** (6.9.1) ...

...

(6.12) - A temporary bound to a reference in a *new-initializer* (7.6.2.8) persists until the completion of the **full-expression** containing the *new-initializer*.

[*Note 7*: **This might introduce a dangling reference.** - *end note*]

[*Example 5*:

```cpp    
struct S { int mi; const std::pair<int,int>& mp; };
S a { 1, {2,3} };
S* p = new S{ 1, {2,3} }; // creates dangling reference
```

-- *end example*]

---

**Working Draft, Standard for Programming Language C++** [^n4910]

</td>
</tr>
</table>

In the previous example an instance of type S became dependent upon the temporary, `{2, 3}`, because it retained a reference to the temporary. This is detectable because the member `mp` is publicly available since the type `S` is a struct.

For a class where members `mi` and `mp` are abstracted away, the dependence can be expressed in its constructor.

```cpp    
class S
{
public:
    [[parameter_dependency(dependent{"this"}, providers{"mp"})]]
    S(int mi, const std::pair<int,int>& mp);
private:
    int mi;
    const std::pair<int,int>& mp;
};

S a { 1, {2,3} }; // error: can't assign temporary to a named instance
some_other_function(a); // error: can't use `a` as it is dependent upon a temporary
S* p = new S{ 1, {2,3} }; // error: instance dependent upon temporary
```

### Lambdas revisited

Lambdas can be expressed in terms of the previous examples in two different ways depending upon whether the lambdas are capturing or not.

```cpp
struct S
{
    int& first;
    const int& second;
    [[dependson(input)]]
    const int& f(const int& input/* unknown lifetime */);
};

[[dependson(input)]]
const int& f(const int& input/* unknown lifetime */);

// a non capturing lambda is no different than a function
// and in this case this `lambda` is essentially the same as `f`
auto non_capturing_lambda()
{
    int local = 42;
    auto lambda = [](const int& input) -> [[dependson(input)]] const int&
    {
        return input;// OK, will be handled by caller
    };
    if(randomBool())
    {
        return lambda(local);// error: can't return local
    }
    return lambda(24);// error: can't return temporary
}

// a capturing lambda is no different than a functor
// and in this case this `lambda` is essentially the same as an instance of `S`
auto capturing_lambda()
{
    int local = 42;
    const int& ref_temporary = f1(GLOBAL, 24);// error: can't assign temporary to a named instance
    auto lambda = [&local, &ref_temporary](const int& input) -> [[dependson(input)]] const int&
    {
        if(randomBool())
        {
            return local;// error: can't return local
        }
        if(randomBool())
        {
            return ref_temporary;// error: can't return temporary
        }
        return input;// OK, will be handled by caller
    };
    if(randomBool())
    {
        return lambda(local);// error: can't return local
    }
    return lambda(24);// error: can't return temporary
}
```

This set of examples illustrates that lambdas are no different than the previous function and class examples other than that they are anonymous and the compiler has greater responsibility in handling dangling correctly.

### Impact on Pattern Matching

This feature can be enhanced further to work with potential future `C++` language features such as pattern matching.

<span style="color:red">**WARNING: If pattern patching can allow propagating references from inner scopes to containing scopes than there will be a new category of dangling added to C++ and consequently a weakening of the safety of references.**</span>

This paper references the `do expressions` [^p2806r1] paper for the relevant portion of pattern matching because it is more explicit. If the C++ standard goes with an implicit syntax it can still be an issue if it allow propagating references from inner scopes to containing scopes.

The simplest example is as follows.

```cpp

int x = do { do return 42; };

```

The simplest example we have to be on guard against is as follows.

```cpp

int& x = do
{
    do return 42;// dangling likely caught by `Simpler implicit move`
};

```

This paper can similarly handle the indirect cases as was performed for returns.

```cpp

const int& x = do
{
    int local = 42;
    const int& r1 /*local*/ = f1(local, local);
    const int& r2 /*global*/ = f1(GLOBAL, GLOBAL);
    const int& r3 /*temporary*/ = f1(42, 42);// error: can't assign temporary to a named instance
    const int& r4 /*local*/ = f1(local, GLOBAL);
    const int& r5 /*temporary*/ = f1(local, 42);// error: can't assign temporary to a named instance
    const int& r6 /*temporary*/ = f1(GLOBAL, 42);// error: can't assign temporary to a named instance
    if(randomBool())
    {
        do return r1;// error: can't do return local
    }
    if(randomBool())
    {
        do return r2;// OK its a reference to a global
    }
    if(randomBool())
    {
        do return r3;// error: can't do return temporary
    }
    if(randomBool())
    {
        do return r4;// error: can't do return local
    }
    if(randomBool())
    {
        do return r5;// error: can't do return temporary
    }
    if(randomBool())
    {
        do return r6;// error: can't do return temporary
    }
    int x1 = r3 + 43;// error: can't use reference to a temporary
    int x2 = r5 + 44;// error: can't use reference to a temporary
    int x3 = r6 + 45;// error: can't use reference to a temporary
    return f1(f1(GLOBAL, 4), f1(local, 2));// error: can't return temporary
};

```

Do expressions i.e. pattern matching expressions are not completely self contained like functions. They can directly use locals from containing scopes. Do returning these locals **COULD** be allowed, while `do return`ing locals in the same scope of the `do expression` should **DEFINITELY** be disallowed.

```cpp

const int& f()
{
    int local1 = 42;
    const int& rint1 = do
    {
        int local2 = 42;
        const int& rint2 = do
        {
            int local3 = 42;
            if(randomBool())
            {
                do return local1;// OK, definitively safe
            }
            if(randomBool())
            {
                do return local2;// OK for now, problem later
            }
            do return local3;// error: can't do return local
        };
        if(randomBool())
        {
            do return local1;// OK, definitively safe
        }
        if(randomBool())
        {
            do return local2;// error: can't do return local
        }
        do return rint2;// ?error or runtime unknown? could be local1 or local2
    };
}

```

The problem on `rint2` is that what should its lifetime response be; an error or unknown? The fact is even that can remain a local and the dangling can be identified by adding an integer to the two bits of compile time metadata.

1. First of all, each scope is given a `level identifier`.
1. Multiple scopes will share the same number if they are on the same level. 
1. Starting at 1, each number is applied to each inner scope in a `breadth first search` i.e. `level order` manner.
1. Next each local also receive a `level identifier` equal to the `level identifier` of the scope they were created in. Similarly, references gets a `level identifier` equal to the `level identifier` of the instance that they reference.
1. While this metadata is meant only for locals, it may make it easier in one's implementations to assign 0 for globals and max int for temporaries.
1. A reference returned from a `do` is set to the maximum value of all the `level identifier`'s of all the references and variables `do return`ed. 
1. Errors result from `do return`s if the variable or reference has a `level identifier` greater than or equal the the `do return`(s) `do`'s `level identifier`.
1. Errors also result from `do`(s) if the aggregated `level identifier` is greater than or equal to the do's `level identifier`.

See below for the previous example modified with all of this new metadata.

<!--
I could think of a couple of solutions.

- Prevent `do expressions` whether explicit or implicit in some other pattern matching proposals from `do returning` references to locals that are not declared before the top most nested `do`. "An ounce of prevention is worth a pound of cure."
- Do nothing as this proposal is not meant to fix runtime dangling of the stack

Regardless of the decision that would need to be made, so much definitively bad dangling has been identified and made more transparent.
-->

```cpp

const int& f()
{// scope level 1
    int local1 = 42;// local with scope level 1
    const int& rint1 = do// reference with scope level 3
    // reference scope level 3 = max(1, 2, 3) from the do return(s)
    // error at the do: can't return reference to one or more locals
    // rint1's scope level 3 >= this do's scope level 2
    {// scope level 2
        int local2 = 42;// local with scope level 2
        const int& rint2 = do// reference with scope level 3
        // reference scope level 3 = max(1, 2, 3) from the do return(s)
        // error at the do: can't return reference to one or more locals
        // rint2's scope level 3 == this do's scope level 3
        {// scope level 3
            int local3 = 42;// local with scope level 3
            if(randomBool())
            {// scope level 4
                do return local1;// OK, definitively safe
                // local1's scope level 1 > this do's scope level 3
            }
            if(randomBool())
            {// scope level 4
                do return local2;// OK for now, problem later
                // local2's scope level 2 > this do's scope level 3
            }
            do return local3;// error: can't do return local
            // local3's scope level 3 == this do's scope level 3
        };
        if(randomBool())
        {// scope level 3
            do return local1;// OK, definitively safe
            // local1's scope level 1 > this do's scope level 2
        }
        if(randomBool())
        {// scope level 3
            do return local2;// error: can't do return local
            // local2's scope level 2 == this do's scope level 2
        }
        do return rint2;// error: can't do return local
        // reference has scope level 3 ... 2 when do return local3 changed
        // rint2's scope level 3 >= this do's scope level 2
    };
}

```

Consequently, a lot of definitively bad dangling has been identified and made more transparent.

---

<!--

https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2447r0.html
-->
So, how does this proposal stack up to the design group's opinion on safety for C++.

<table>
<tr>
<td>

- *Do not radically break backwards compatibility – compatibility is a key feature and strength of C++ compared to more modern and more fashionable languages.* [^p2759r1]
  - **This proposal does not break any correct code. It only produces errors for code that definitely dangle.**
- Do not deliver safety at the cost of an inability to express the abstractions that are currently at the core of C++ strengths. [^p2759r1]
  - **This proposal does not compromise nor remove any of C++ features so abstractions are still there.**
- Do not leave us with a “safe” subset of C that eliminates C++’s productivity advantages. [^p2759r1]
  - **This proposal works with all existing code. It is purely optin.**
- Do not deliver a purely run-time model that imposes overheads that eliminate C++’s strengths in the area of performance. [^p2759r1]
  - **This proposal is completely compile time.**
- Do not imply that there is exactly one form of “safety” that must be adopted by all. [^p2759r1]
  - **This proposal does not conflict with existing safety designs. While most others are runtime, this proposal is purely compile time. As such they are complimentary.**
- Do not promise to deliver complete guaranteed type-and-resource safety for all uses. [^p2759r1]
  - **This proposal only address dangling of the stack via references which can be easily achieved if library authors provide the compiler a little more information that is needed by library users anyway.**
- Do offer paths to gradual and partial adoption – opening paths to improving the billions of lines of existing C++ code. [^p2759r1]
  - **This proposal is opt in on a per function basis.**
- Do not imply a freeze on further development of C++ in other directions. [^p2759r1]
  - **Runtime checks can be performed concurrently.**
- Do not imply that equivalent-looking code written in different environments will have different semantics (exception: some environments may give some code “undefined behavior” while others give it (a single!) defined behavior). [^p2759r1]
  - **Bad code is bad code regardless of the environment. This proposal makes such bad code more transparent.**

---

**DG OPINION ON SAFETY FOR ISO C++** [^p2759r1]<br/>*H. Hinnant, R. Orr, B. Stroustrup, D. Vandevoorde, M. Wong*

</td>
</tr>
</table>

## Resolution

Now that these types of dangling can be detected, there are some tools that could be provided to developers to make it easier to fix these detected instances of dangling which are **NOT** a part of this proposal. Let's go back to our first example.

```cpp
const int GLOBAL = 42;

[[dependson(left, right)]]
const int& f1(const int& left/* unknown lifetime */, const int& right/* unknown lifetime */)
{
    if(randomBool())
    {
        return left;
    }
    else
    {
        return right;
    }
}

const int& f2()
{
    int local = 42;
    const int& r1 /*local*/ = f1(local, local);
    const int& r2 /*global*/ = f1(GLOBAL, GLOBAL);
    const int& r3 /*temporary*/ = f1(42, 42);// error: can't assign temporary to a named instance
    const int& r4 /*local*/ = f1(local, GLOBAL);
    const int& r5 /*temporary*/ = f1(local, 42);// error: can't assign temporary to a named instance
    const int& r6 /*temporary*/ = f1(GLOBAL, 42);// error: can't assign temporary to a named instance
    if(randomBool())
    {
        return r1;// error: can't return local
    }
    if(randomBool())
    {
        return r2;// OK its a reference to a global
    }
    if(randomBool())
    {
        return r3;// error: can't return temporary
    }
    if(randomBool())
    {
        return r4;// error: can't return local
    }
    if(randomBool())
    {
        return r5;// error: can't return temporary
    }
    if(randomBool())
    {
        return r6;// error: can't return temporary
    }
    int x1 = r3 + 43;// error: can't use reference to a temporary
    int x2 = r5 + 44;// error: can't use reference to a temporary
    int x3 = r6 + 45;// error: can't use reference to a temporary
    return f1(f1(GLOBAL, 4), f1(local, 2));// error: can't return temporary
}
```

Since locals and temporaries should not be returned from functions, most functions that possess this type of dangling may be in need of some refactoring, perhaps using movable value types. For dangling that occurs in the body of a function, locals need to be moved up in scope and temporaries need to be changed into locals and then perhaps moved up in scope. This results in more lines of code, superfluous naming and excessive refactoring. If the fixed temporary is only ever used in a constant fashion and if it is a literal type and constant initialized than it would likely be manually turned into a global and moved far from the point of use. All of this could be made easier upon programmers with the following features.

1. Temporaries that are initially constant referenced, where the type is a literal type and the instance could be constant initialized, then the compiler would automatically promote these to having static storage duration [^p2724r1] just like a string and hopefully in the future like `std::initializer_list` [^p2752r1].
1. C23 introduced storage-class specifiers for compound literals. [^n3038] If C++ followed suit, than we could be able to apply `static` and `constexpr` to our temporaries. Since these two would frequently be used together it could be shortened to `constant` or `constinit`. C++ could go even farther by introducting a new specifier perhaps called `var` for variable scope that would turn the temporary into a anonymously named variable with the same life of the left most instance in the full expression. [^p2658r1]

Let's see how these resolutions simply fixes some of the dangling in the previous example. In the following example, the keyword `constinit` is used to represent explicit constant initialization while the keyword `variable` is used to represent variable scope i.e. explicit lifetime extension. All of the `constinit` keywords in the example could be removed if there was implicit constant initialization.

```cpp
const int GLOBAL = 42;

[[dependson(left, right)]]
const int& f1(const int& left/* unknown lifetime */, const int& right/* unknown lifetime */)
{
    if(randomBool())
    {
        return left;
    }
    else
    {
        return right;
    }
}

const int& f2()
{
    int local = 42;
    const int& r1 /*local*/ = f1(local, local);
    const int& r2 /*global*/ = f1(GLOBAL, GLOBAL);
    const int& r3 /*local*/ = f1(constinit 42, variable 42);// was temporary
    const int& r4 /*local*/ = f1(local, GLOBAL);
    const int& r5 /*local*/ = f1(local, constinit 42);// was temporary
    const int& r6 /*local*/ = f1(GLOBAL, variable 42);// was temporary
    if(randomBool())
    {
        return r1;// error: can't return local
    }
    if(randomBool())
    {
        return r2;// OK its a reference to a global
    }
    if(randomBool())
    {
        return r3;// error: can't return local// NOTE: was temporary
    }
    if(randomBool())
    {
        return r4;// error: can't return local
    }
    if(randomBool())
    {
        return r5;// error: can't return local// NOTE: was temporary
    }
    if(randomBool())
    {
        return r6;// error: can't return local// NOTE: was temporary
    }
    int x1 = r3 + 43;// FIXED: was a reference to a temporary, now local
    int x2 = r5 + 44;// FIXED: was a reference to a temporary, now local
    int x3 = r6 + 45;// FIXED: was a reference to a temporary, now local
    return f1(f1(GLOBAL, variable 4), f1(local, constinit 2));// error: can't return local
    // NOTE: was temporary instead of local
}
```

This proposal and these three resolutions all satisfy the design group’s opinion on safety for C++.

<table>
<tr>
<td>&<span style="color:green">&#x2713;</span></td>
<td>implicit<br/>constant</td>
<td>explicit<br/>constant</td>
<td>var</td>
<td>Opinion</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

*Do not radically break backwards compatibility – compatibility is a key feature and strength of C++ compared to more modern and more fashionable languages.* [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not deliver safety at the cost of an inability to express the abstractions that are currently at the core of C++ strengths. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not leave us with a “safe” subset of C that eliminates C++’s productivity advantages. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not deliver a purely run-time model that imposes overheads that eliminate C++’s strengths in the area of performance. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not imply that there is exactly one form of “safety” that must be adopted by all. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not promise to deliver complete guaranteed type-and-resource safety for all uses. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do offer paths to gradual and partial adoption – opening paths to improving the billions of lines of existing C++ code. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not imply a freeze on further development of C++ in other directions. [^p2759r1]

</td>
</tr>
<tr>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td style="color:green;text-align:center">&#x2713;</td>
<td>

Do not imply that equivalent-looking code written in different environments will have different semantics (exception: some environments may give some code “undefined behavior” while others give it (a single!) defined behavior). [^p2759r1]

</td>
</tr>
</table>

What's more, `implicit constant` is the most effective. While both '&<span style="color:green">&#x2713;</span>' and `implicit constant` can be categorized as `elimination`, only `implicit constant` automatically fixes instances of dangling in a non breaking way that is logical to end programmers.

![Hierarchy of Controls](https://upload.wikimedia.org/wikipedia/commons/3/36/NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg)

While `explicit constant` and `var` are only `Personal Protective Equipment`, they are highly useful with or without this proposal as generic tools programmers can use to fix instances of dangling simply. They especially compliment this proposal as tools to fix dangling identified by this proposal.

## Usage

A common complaint is that programmers would never use any attribution feature based on past experience with similar attribution efforts that existed outside of the standard. One usage of this proposal is applying its annotations to the STL itself. Taking just the `std::string` class as an example, following are the functions made safer with this proposal.

- Element access
  1. `at`
  1. `operator []`
  1. `front`
  1. `back`
  1. `data`
  1. `c_str`
  1. `operator basic_string_view`
- Iterators
  1. `begin`
  1. `cbegin`
  1. `end`
  1. `cend`
  1. `rbegin`
  1. `crbegin`
  1. `rend`
  1. `crend`
- Operations
  1. `insert`
  1. `insert_range`
  1. `erase`
  1. `append`
  1. `append_range`
  1. `operator +=`
  1. `replace`
  1. `replace_with_range`
- Input/output
  1. `getline`

The point is programmers who consume safer libraries don't have to do anything. The programmer of a library should apply this proposals annotations to its public top most API for documentation and if standardized for specification purposes. However, if a library producer does add it to its implementation than the library producer themselves would also benefit.

## Viral Attribution Effort

Another common complaint with this and really any annotation based proposal is the work involved in annotating everything, in this proposal's case, functions. In this proposal's case, this attribution effort is just good documentation that a library producer should already be doing anyway to at least their public API. This proposal moves this documentation from non standard comments and/or non standard external documentation to standardized annotations. Other static analysis tools produce mostly warnings which would require some type of viral attribution effort in the code to tell those tools to ignore false positives.

<table>
<tr>
<td>

```cpp
//NOSONAR
```

</td>
<td>

Sonar

</td>
</tr>
<tr>
<td>

```java
@SuppressWarnings({"", "", ""})
```

</td>
<td>

Java [^javasuppresswarnings] [^baeldungjavasuppresswarnings]

</td>
</tr>
<tr>
<td>

```c#
#pragma warning disable 0000
// code
#pragma warning restore 0000
```

</td>
<td>

C# [^so1378634]

</td>
</tr>
<tr>
<td>

```c#
[SuppressMessage("", "", Justification = "")]
```

</td>
<td>

C# [^so1378634]

</td>
</tr>
</table>

This proposal produces errors for definitely bad code and as such avoids the need for additional viral attribution efforts for ignoring false positives.

## Summary

The advantages of adopting said proposal are as follows:

1. Standardize the documentation of lifetimes of API(s) for developers
1. Standardize the specification of lifetimes for proposals
1. Produce more meaningful return error messages that doesn't involve value categories
1. Empowers programmers with tools to identify indirect occurences of immediate dangling of references to the stack, simply
1. Empowers programmers with tools to identify indirect occurences of return dangling of references to the stack, simply

## Frequently Asked Questions

### Why not pointers?

References have to be initialized, can't be `nullptr` and can't be rebound which means by default the lifetime of the instance the reference points to is fixed at the moment of construction which has to exist lower on the stack i.e. prior to reference creation which is known at compile time. This is very safe by default. Pointers and reference classes that has pointer semantics are none of these things. Since they are so dynamic, the relevant metadata would more frequently be needed at run time.

That having been said, pointers used in a reference like way could benefit from the checks proposed in this paper.

<table>
<tr>
<td>

*"**a reference is similar to a const pointer such as `int* const p`** (as opposed to a pointer to const such as `const int* p`)"* [^reseatingrefs]

---

**How can you reseat a reference to make it refer to a different object?** [^reseatingrefs]<br/>*ISOCPP > Wiki Home > References*

</td>
</tr>
</table>

This would require increasing the two bits of metadata to include a third bit in order to handle `nullptr` and uninitialized. This is not currently in scope of this proposal.

### Why not `explicit lifetime dependence` on `struct`(s)?

1. References are rarely used in class definitions. Instead programmers use std::reference_wrapper, smart pointers and plain old pointers. Since all of these are rebindable, they are more runtime than compile time and as such is not the subject of this proposal. See `Why not pointers?` for more information.
1. `Explicit lifetime dependence` on `struct`(s)/`class`[es] breaks abstraction because the user of a library need to know class/struct implementation details that are likely `protected`, `private` and internal. For public reference members of `struct` instances, the compiler with these enhancements can already propagate these references. Further, `explicit lifetime dependence` on functions allows propagating the needed information when applied to constructors, conversion operators and factory functions.

## References

<!--Hierarchy of Controls-->
[^cdchoc]: <https://www.cdc.gov/niosh/topics/hierarchy/default.html>
<!--Hierarchy of hazard controls-->
[^wikihohc]: <https://en.wikipedia.org/wiki/Hierarchy_of_hazard_controls>
<!--Hierarchy of hazard controls-->
[^wikihohclicense]: <https://en.wikipedia.org/wiki/File:NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg>
<!--Hierarchy of hazard controls-->
[^wikihohclicenseimage]: <https://upload.wikimedia.org/wikipedia/commons/3/36/NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg>
<!--C Dangling Reduction-->
[^p2750r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2750r2.html>
<!--indirect dangling identification-->
[^p2742r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2742r2.html>
<!--Simpler implicit dangling resolution-->
[^p2740r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2740r2.html>
<!--variable scope-->
[^p2730r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2730r1.html>
<!--constant dangling-->
[^p2724r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2724r1.html>
<!--implicit constant initialization-->
[^p2623r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2623r2.html>
<!--temporary storage class specifiers-->
[^p2658r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2658r1.html>
<!--C++ is the next C++-->
[^p2657r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2657r1.html>
<!--C++ Core Guidelines - In.force: Enforcement-->
[^cppcg-inforce-enforcement]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#inforce-enforcement>
<!--C++ Core Guidelines - P.5: Prefer compile-time checking to run-time checking-->
[^cppcg-p5-prefer-compile-time-checking-to-run-time-checking]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p5-prefer-compile-time-checking-to-run-time-checking>
<!--C++ Core Guidelines - P.8: Don’t leak any resources-->
[^cppcg-p8-dont-leak-any-resources]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p8-dont-leak-any-resources>
<!--C++ Core Guidelines - P.10: Prefer immutable data to mutable data-->
[^cppcg-p10-prefer-immutable-data-to-mutable-data]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p10-prefer-immutable-data-to-mutable-data>
<!--C++ Core Guidelines - P.11: Encapsulate messy constructs, rather than spreading through the code-->
[^cppcg-p11-encapsulate-messy-constructs-rather-than-spreading-through-the-code]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p11-encapsulate-messy-constructs-rather-than-spreading-through-the-code>
<!--C++ Core Guidelines - In.force: Enforcement-->
[^cppcg-inforce-enforcement]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#inforce-enforcement>
<!--C++ Core Guidelines - TODO-->
[^cppcg-TODO]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#TODO>
<!--C++ Core Guidelines - TODO-->
[^cppcg-TODO]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#TODO>
<!--C++ Core Guidelines - TODO-->
[^cppcg-TODO]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#TODO>
<!--C++ Core Guidelines - TODO-->
[^cppcg-TODO]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#TODO>
<!--Contracts for C++: Prioritizing Safety-->
[^p2680r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2680r1.pdf>
<!--A call to action: Think seriously about “safety”; then do something sensible about it-->
[^p2739r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2739r0.pdf>
<!--Towards memory safety in C++-->
[^p2771r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2771r0.html>
<!--Safety Profiles: Type-and-resource Safe programming in ISO Standard C++-->
[^p2816r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2816r0.pdf>
<!--Lifetime safety: Preventing common dangling-->
[^p1179r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1179r1.pdf>
<!--Leaving no room for a lower-level language: A C++ Subset-->
[^p1105r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1105r0.html>
<!--DG OPINION ON SAFETY FOR ISO C++ - 3 Basic Tenets-->
[^p2759r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2759r1.pdf>
<!--Why are explicit lifetimes needed in Rust?-->
[^so31609137]: <https://stackoverflow.com/questions/31609137/why-are-explicit-lifetimes-needed-in-rust>
<!--Bind Returned/Initialized Objects to the Lifetime of Parameters, Rev0-->
[^p0936r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0936r0.pdf>
<!--Simpler implicit move-->
[^p2266r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2266r3.html>
<!--Working Draft, Standard for Programming Language C++-->
[^n4910]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4910.pdf>
<!--do expressions-->
[^p2806r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2806r1.html>
<!--function_ref: a type-erased callable reference-->
[^p0792r14]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0792r14.html>
<!--Introduce storage-class specifiers for compound literals-->
[^n3038]: <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3038.htm>
<!--Static storage for braced initializers-->
[^p2752r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2752r1.html>
<!--SuppressWarnings-->
[^javasuppresswarnings]: <https://docs.oracle.com/javase/8/docs/api/java/lang/SuppressWarnings.html>
<!--Java @SuppressWarnings Annotation-->
[^baeldungjavasuppresswarnings]: <https://www.baeldung.com/java-suppresswarnings>
<!--Is there a way to suppress warnings in C# similar to Java's @SuppressWarnings annotation?-->
[^so1378634]: <https://stackoverflow.com/questions/1378634/is-there-a-way-to-suppress-warnings-in-c-sharp-similar-to-javas-suppresswarnin>
<!--How can you reseat a reference to make it refer to a different object?-->
[^reseatingrefs]: <https://isocpp.org/wiki/faq/references#reseating-refs>
<!---->
<!--
[^]: <>
-->
