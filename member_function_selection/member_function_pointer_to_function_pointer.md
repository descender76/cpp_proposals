<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2603R1</td>
</tr>
<tr>
<td>Date</td>
<td>2022-09-01</td>
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

# member function pointer to function pointer

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

- [member function pointer to function pointer](#member-function-pointer-to-function-pointer)
  - [Abstract](#abstract)
  - [Motivating example](#motivating-example)
    - [Scenario #1 - Unknown State](#scenario-1-unknown-state)
    - [Scenario #2 - Known State](#scenario-2-known-state)
      - [Scenario #2a - Known State - Exact](#scenario-2a-known-state-exact)
      - [Scenario #2b - Known State - Derived](#scenario-2b-known-state-derived)
  - [Solution](#solution)
  - [Summary](#summary)
  - [Prior work](#prior-work)
  - [References](#references)

## Changelog

### R1

- shortened `member_function_pointer_to_free_function_pointer` to `to_free_function_pointer`
- clarified `consteval`
- added support of free function pointer passthrough

## Abstract

`C++` allows one to call a base member function from a derived instance at compile time.

`Working Draft, Standard for Programming Language C++` [^n4910]

"*11.7.3 Virtual functions [class.virtual]"*

"*16 Explicit qualification with the scope operator (7.5.4.3) suppresses the virtual call mechanism."*

"*[Example 10:"*

```cpp
class B { public: virtual void f(); };
class D : public B { public: void f(); };

void D::f() { /* ... */ B::f(); }
```

*"Here, the function call in D::f really does call B::f and not D::f. — end example]"*

Currently `C++` **does not** allow calling a base member function from a derived instance at runtime. The member function pointer always resolves to the member function of the derived instance. While this does make sense for traditional runtime polymorphism, this behavior is less desired when selecting member functions for other types of runtime polymorphism as it occurs a superfluous dual dispatch.

Just as the programmer decides whether to call the virtual function virtually or not at compile time, the programmer should be able to do the same at runtime.

## Motivating Example

<!--
`"Direct" vs "virtual" call to a virtual function` [^directvsvirtual]
-->

<!--
```cpp
//import std;
//import <iostream>;
#include <iostream>;

class Base
{ 
public:
    virtual void some_pure_virtual_function() = 0;
    virtual void some_virtual_function()
    {
        std::cout << "Base::some_virtual_function\n";
    }
    virtual void some_virtual_function() const
    {
        std::cout << "Base::some_virtual_function const\n";
    }
};

class Derived : public Base
{
public:
    void some_pure_virtual_function() override
    {
        std::cout << "Derived::some_pure_virtual_function\n";
    }
    void some_virtual_function() override
    {
        //Base::some_virtual_function();
        std::cout << "Derived::some_virtual_function\n";
    }
    void some_virtual_function() const override
    {
        //Base::some_virtual_function();
        std::cout << "Derived::some_virtual_function const\n";
    }
};

int main()
{
    Derived derived;
    Derived* pderived = &derived;
    Derived& rderived = derived;

    derived.some_virtual_function(); // Direct call to Derived::some_virtual_function
    pderived->some_virtual_function(); // Virtual call in general case to Derived::some_virtual_function
    rderived.some_virtual_function(); // Virtual call in general case to Derived::some_virtual_function

    derived.Base::some_virtual_function(); // Direct call to Base::some_virtual_function
    pderived->Base::some_virtual_function(); // Direct call to Base::some_virtual_function
    rderived.Base::some_virtual_function(); // Direct call to Base::some_virtual_function

    void (Base::*bmfp)() = &Base::some_virtual_function;
    void (Derived::*dmfp)() = &Derived::some_virtual_function;
    void (Derived::*dmfpc)() const = static_cast<void (Derived::*)() const>(&Derived::some_virtual_function);

    (derived.*bmfp)();// Derived::some_virtual_function
    (derived.*dmfp)();// Derived::some_virtual_function
    (derived.*dmfpc)();// Derived::some_virtual_function const

    return 0;
}
```
-->

```cpp
class Abstract
{ 
public:
    virtual void some_virtual_function() = 0;
    virtual void some_virtual_function() const = 0;
};

class Base : public Abstract
{ 
public:
    void some_virtual_function() override { }
    void some_virtual_function() const override { }
};

class Derived : public Base
{
public:
    void some_virtual_function() override { }
    void some_virtual_function() const override { }
    void some_deducing_this_member_function(this Derived) { }
};

int main()
{
    Derived derived;
    Derived* pderived = &derived;
    Derived& rderived = derived;

    derived.some_virtual_function(); // Direct call to Derived::some_virtual_function
    pderived->some_virtual_function(); // Virtual call in general case to Derived::some_virtual_function
    rderived.some_virtual_function(); // Virtual call in general case to Derived::some_virtual_function

    // NOTE: A derived class can call its base class member function directly
    // at compile time using a qualifier in these cases Base:: 
    derived.Base::some_virtual_function(); // Direct call to Base::some_virtual_function
    pderived->Base::some_virtual_function(); // Direct call to Base::some_virtual_function
    rderived.Base::some_virtual_function(); // Direct call to Base::some_virtual_function

    void (Base::*bmfp)() = &Base::some_virtual_function;
    void (Derived::*dmfp)() = &Derived::some_virtual_function;
    void (Derived::*dmfpc)() const = static_cast<void (Derived::*)() const>(&Derived::some_virtual_function);

    // NOTE: A derived class can NOT call its base class member function at runtime
    (derived.*bmfp)();// Derived::some_virtual_function
    (derived.*dmfp)();// Derived::some_virtual_function
    (derived.*dmfpc)();// Derived::some_virtual_function const

    return 0;
}

void Base_some_virtual_function(Base& instance)
{
     // Direct call to Base::some_virtual_function
    instance.Base::some_virtual_function();
}

void const_Base_some_virtual_function(const Base& instance)
{
     // Direct call to Base::some_virtual_function
    instance.Base::some_virtual_function();
}
```

The most relevant lines in question are the following:

```cpp
Derived derived;

void (Base::*bmfp)() = &Base::some_virtual_function;

(derived.*bmfp)();// Derived::some_virtual_function
```

Even though the programmer explicitly stated that they want to call `Base`'s `some_virtual_function`, `Derived`'s `some_virtual_function` is always called. The programmer was never given the same choice that they have at compile time. Again this is correct for traditional runtime polymorphism. However, for a callback using raw pointers, `function_ref` [^p0792r9], or `nontype function_ref` [^p2472r3], the end programmer wants the choice and really should choose based on the scenario.

### Scenario #1 - Unknown State

For instance, if the programmer received some pointer or reference to some instance and as such doesn't know the exact type of the instance  than the logical and safe choice is to call the method virtually even though it incurs dual dispatch costs. This scenario **occurs frequently** when integrating with 3rd party libraries that are unaware of the callback type and as such the callback type is instantiated **late**. 

```cpp
function_ref<void()> factory(Base& base)
{
  // base could be of type Base, Derived or some other class not known at this function's creation
  // in which case dual dispatch is still a good idea
  function_ref<void()> fr = {nontype<&Base::some_virtual_function, some_virtual_tag_class>, base};
  return fr;
}
```

A library could provide an overload via a tag class, in this example `some_virtual_tag_class`, in order to allow the programmer to choose whether the function will be called virtually or directly, in this case the former.

### Scenario #2 - Known State

However, if the programmer knows the instantiated type, likely because the programmer was the creator, then the programmer wants to avoid the superfluous cost of calling the member function through the member function pointer. This scenario **occurs even more frequently** when the programmer is calling his own code, his team member's code or a 3rd party library that is aware of the callback type and provide callback instances. As such the callback type is instantiated **early**. 

#### Scenario #2a - Known State - Exact

In this scenario, the state is known and is the same type of the class that houses the member function. As such there is no need resolve the member function at runtime since the type is known.

```cpp
Base base;
function_ref<void()> fr = {nontype<&Base::some_virtual_function, some_direct_tag_class>, base};
```

Again a library could provide an overload via a tag class, in this example `some_direct_tag_class`, in order to allow the programmer to choose whether the function will be called virtually or directly, in this case the later.

#### Scenario #2b - Known State - Derived

This would also work safely for derived state calling their base class member functions at runtime without the additional member function pointer costs. After all, `Derived` is still a `Base`.

```cpp
Derived derived;
function_ref<void()> fr = {nontype<&Base::some_virtual_function, some_direct_tag_class>, derived};
```

It should ne noted, that besides callbacks of single functions this functionality could be of benefit to callbacks of n-ary functions such as found in `proxy` [^p0957r7], `dyno` [^dyno] and `boost ext te` [^boostte].

## Solution

**Unlike** calling base class member functions with derived instances at compile time via a qualifier, it is **undesirable** to add keywords/vernacular/qualifiers at the point of function call, that is to choose direct or virtual at call time. This incurs additional runtime costs of potentially increased member function pointer size and execution time, even for those that don't need the choice as in traditional runtime polymorphism.

```cpp
Derived derived;

void (Base::*bmfp)() = &Base::some_virtual_function;

// NOTE: This is NOT desired
(derived.*bmfp)() direct;// Base::some_virtual_function
(derived.*bmfp)() virtual;// Derived::some_virtual_function
```

It is also **undesirable** to add keywords/vernacular at the point of member function ponter initialization. This also incurs additional runtime costs of potentially increased member function pointer size and execution time, even for those that don't need the choice as in traditional runtime polymorphism.

```cpp
Derived derived;

void (Base::*bmfp1)() = &Base::some_virtual_function direct;
void (Base::*bmfp2)() = &Base::some_virtual_function virtual;
```

What is desired is no change to member function pointer, at all! Rather, a new intrinsic `consteval` function would be created called `to_free_function_pointer`. This function would not take member function pointer at runtime but only a member function pointer initialization statement, `&class_name::member_function_name`, at compile time. Technically, it could also take a static_cast of a member function pointer initialization to a specific member function pointer type to allow explicit choosing of overloaded methods. What gets returned is just a free function pointer that points to the actual member function or a thunk where the `this` reference is the first parameter. For `Deducing this` [^p0847r7] member functions, the `this` type could be a value instead of a reference and as such this new function would just be a passthrough. This intrinsic function would also be overloaded for free functions in which case it would just be a passthrough. 

```cpp
Derived derived;
// initialized with member functions
void (*bfp1)(Base&) = to_free_function_pointer(&Base::some_virtual_function);
void (*dfp)(Derived&) = to_free_function_pointer(&Derived::some_virtual_function);
void (*dfpc)(const Derived&) = to_free_function_pointer(static_cast<void (Derived::*)() const>(&Derived::some_virtual_function));
void (*ddtfp)(Derived) = to_free_function_pointer(&Derived::some_deducing_this_member_function);
// initialized with free functions (always passthrough)
void (*bfp2)(Base&) = to_free_function_pointer(Base_some_virtual_function);
void (*bfp3)(const Base&) = to_free_function_pointer(const_Base_some_virtual_function);
```

With this, `Base::some_virtual_function` can be called at runtime, simply initialized, like [member] function pointers, without having to use a lambda. The end result is member function pointers' initialization syntax can be used to select member functions with the knowledge that the selected is the one that actually will be called.

NOTE: The following is invalid code because there is no function when the member function declaration is pure.

```cpp
void (*bfp)(Abstract&) = to_free_function_pointer(&Abstract::some_virtual_function);
```

The brevity of the name of the intrinsic function is less relevant as it will in all likelihood be concealed in library implementations rather than used directly.
<!--
Though, I wouldn't want to rule anything out.
-->

## Summary

The advantages to `C++` with this proposal is manifold.

- A seemingly oversight in `C++` gets fixed by allowing calling a base member function from a derived instance at runtime
- Mitigates a bifurcation by allowing one to interact with member functions regardless of whether they are a `Deducing this` [^p0847r7] member function or a legacy member function
- Mitigates another bifurcation by allowing one to interact with functions regardless of whether they are a member function or a free function
- Matches existing practice by allowing users to use member function pointer initialization to select member functions with the confidence that it is the function that will be called

## Prior work

GCC has support acquiring the function pointer from a member function pointer via its `bound member function` [^boundmemberfunctions] feature. While their implementation supports the functionality at both runtime and at compile time, this proposal is solely concerned about compile time. Also, while GCC uses a casting mechanism, this proposal is asking for a intrinsic function to better support automatic type deduction.

## References

<!--"Direct" vs "virtual" call to a virtual function-->
[^directvsvirtual]: <https://stackoverflow.com/questions/36584670/direct-vs-virtual-call-to-a-virtual-function>
<!--Working Draft, Standard for Programming Language C++-->
[^n4910]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4910.pdf>
<!--Extracting the Function Pointer from a Bound Pointer to Member Function-->
[^boundmemberfunctions]: <https://gcc.gnu.org/onlinedocs/gcc/Bound-member-functions.html>
<!--function_ref: a non-owning reference to a Callable-->
[^p0792r9]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0792r9.html>
<!--make function_ref more functional-->
[^p2472r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2472r3.html>
<!--Proxy: A Polymorphic Programming Library-->
[^p0957r7]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0957r7.pdf>
<!--Deducing this-->
[^p0847r7]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html>
<!--Dyno: Runtime polymorphism done right-->
[^dyno]: <https://github.com/ldionne/dyno>
<!--boost-ext/te: TE: C++17 Run-time polymorphism-->
[^boostte]: <https://github.com/boost-ext/te>
<!--Language support for customisable functions-->
[^p2547r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2547r0.pdf>
<!---->
<!--
[^]: <>
-->
<!--
recipients
std-proposals@lists.isocpp.org
C++ Library Evolution Working Group <lib-ext@lists.isocpp.org>
Vittorio Romeo <vittorio.romeo@outlook.com>
Zhihao Yuan <zy@miator.net>
Jarrad Waterloo <descender76@gmail.com>
Gašper Ažman
<gasper.azman@gmail.com>
Sy Brand
<sibrand@microsoft.com>
Ben Deane, ben at elbeno dot com
<ben@elbeno.com>
Barry Revzin
<barry.revzin@gmail.com>
Mingxin Wang <mingxwa@microsoft.com>
-->