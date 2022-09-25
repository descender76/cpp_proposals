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
<td>2022-09-24</td>
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

# F43 direct dangling reduction

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

- [F43 direct dangling reduction](#f43-dangling-reduction)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [Summary](#summary)
  - [Frequently Asked Questions](#frequently-asked-questions)
  - [References](#references)

## Abstract

*"Lifetime issues with references to temporaries can lead to fatal and subtle runtime errors."* [^bindp]

This paper proposes fixing one such cause of dangling references and pointers.

## Motivating Examples

**C++ Core Guidelines**<br/>***F.43: Never (directly or indirectly) return a pointer or a reference to a local object*** [^cppcgrf43]

***Reason** To avoid the crashes and data corruption that can result from the use of such a dangling pointer.* [^cppcgrf43]

***Example, bad** After the return from a function its local objects no longer exist:* [^cppcgrf43]

```cpp
int* f()
{
    int fx = 9;
    return &fx;  // BAD
}

void g(int* p)   // looks innocent enough
{
    int gx;
    cout << "*p == " << *p << '\n';
    *p = 999;
    cout << "gx == " << gx << '\n';
}

void h()
{
    int* p = f();
    int z = *p;  // read from abandoned stack frame (bad)
    g(p);        // pass pointer to abandoned stack frame to function (bad)
}
```
<!--
*Here on one popular implementation I got the output:* [^cppcgrf43]

```cpp
*p == 999
gx == 999
```

*I expected that because the call of `g()` reuses the stack space abandoned by the call of `f()` so `*p` refers to the space now occupied by `gx`.* [^cppcgrf43]

- *Imagine what would happen if `fx` and `gx` were of different types.* [^cppcgrf43]
- *Imagine what would happen if `fx` or `gx` was a type with an invariant.* [^cppcgrf43]
- *Imagine what would happen if more that dangling pointer was passed around among a larger set of functions.* [^cppcgrf43]
- *Imagine what a cracker could do with that dangling pointer.* [^cppcgrf43]
-->
...

*Fortunately, most (all?) modern compilers catch and warn against this simple case.* [^cppcgrf43]

***Note** This applies to references as well:* [^cppcgrf43]

```cpp
int& f()
{
    int x = 7;
    // ...
    return x;  // Bad: returns reference to object that is about to be destroyed
}
```

***Note** This applies only to non-static local variables. All static variables are (as their name indicates) statically allocated, so that pointers to them cannot dangle.* [^cppcgrf43]

***Example, bad** Not all examples of leaking a pointer to a local variable are that obvious:* [^cppcgrf43]
<!--
```cpp
int* glob;       // global variables are bad in so many ways

template<class T>
void steal(T x)
{
    glob = x();  // BAD
}

void f()
{
    int i = 99;
    steal([&] { return &i; });
}

int main()
{
    f();
    cout << *glob << '\n';
}
```

*Here I managed to read the location abandoned by the call of `f`. The pointer stored in `glob` could be used much later and cause trouble in unpredictable ways.* [^cppcgrf43]
-->
***Note** The address of a local variable can be “returned”/leaked by a return statement, by a `T&` out-parameter, as a member of a returned object, as an element of a returned array, and more.* [^cppcgrf43]
<!--
***Note** Similar examples can be constructed “leaking” a pointer from an inner scope to an outer one; such examples are handled equivalently to leaks of pointers out of a function.<br/>A slightly different variant of the problem is placing pointers in a container that outlives the objects pointed to.* [^cppcgrf43]

***See also:** Another way of getting dangling pointers is pointer invalidation. It can be detected/prevented with similar techniques.* [^cppcgrf43]
-->
...

***Enforcement*** [^cppcgrf43]
- *Compilers tend to catch return of reference to locals and could in many cases catch return of pointers to locals.* [^cppcgrf43]
- *Static analysis can catch many common patterns of the use of pointers indicating positions (thus eliminating dangling pointers)* [^cppcgrf43]

---

This paper proposes that any non static local that is not a pointer or a reference can't be returned as a pointer or a reference. This is an **error** not a warning.

```cpp
int* return_pointer()
{
  int local;
  return &local;// error 
}

int& return_reference()
{
  int local;
  return local;// error 
}
```

This should be an error instead of a warning because there is no scenario in which it is right. It is always wrong.

As the C++ Core Guidelines pointed out: *Fortunately, most (all?) modern compilers catch and warn against this simple case.* [^cppcgrf43] As such we would just be standardizing existing practice.

This does not propose fixing indirect references to locals such as when the local is passed in as a parameter or when a member of the local is accessed as these would require access to metadata that exist outside of the function in question while with the direct reference all that is needed is the metadata that exist as part of the function compilation. For instance even accessing a member via the `->` operator can be indirect since that operator can be overloaded. Similarly, member access via the `.` operator is not part of the proposal because the `C++` community has not given up on being able to overload the `.` operator which would make something that is direct more indirect in a possible future. Of course, nothing is preventing compilers to producing more errors for dangling but in those cases the compiler would be responsible for ensuring that it is truly an error and not a potential false positive.

### If this feature is so limited why even do it?

1. There is no scenario where permitting this dangling is ever correct.
1. Any reduction in dangling is a win.
1. This is the simplest of dangling and as such aid teaching dangling.

Let me elaborate on that last point. Do we want to teach dangling on day one, likely hour one, for a new `C++` programmer? When this is an error the compiler becomes the teacher. Teaching dangling gets to be delayed some allowing the entry programmer to gain more experience before delving into the dark world of dangling. When it is time to teach dangling, we actually start with the compiler detected dangling. From that spring board, we advance for ever more complex examples of dangling and their resolutions.

Further, failing to fix even this most basic form of dangling begs the questions. Will we ever fix any dangling if we are unwilling to fix the most basic? How can one lead the `C++` community if we fail to lead when it comes to problems that have been plaguing programmers for decades.
<!--
## Summary

There are a couple of principles repeated throughout this proposal.

1. Constants really should have static storage duration so that they never dangle.
1. Temporaries are expected to be just anonymously named variables / `C99` compound literals lifetime rule
    - **variable scope**: sometimes, especially in blocks / not arguments, it is better for a temporary to have automatic storage duration associated with the enclosing block of the variable to which the temporary is assigned instead of being associated with the enclosing block of the temporary expression

The advantages to `C++` with adopting this proposal is manifold.

- Empower programmers to be able to fix most dangling simply
- Reduce the gap between `C++` and `C99` compound literals
- Reduce the gap between `C++` and `C23` storage-class specifiers
- Improve the potential contribution of `C++`'s new specifiers back to `C`
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

### Is `variable_scope` easy to teach?

<table>
<tr>
<td>

**values**

</td>
<td>

**pointers with `C99` &**

</td>
<td>

**references with non existent reference reassignment operator preferably completely new and unique operator since the variable's type may have a custom `&=` operator defined**

</td>
</tr>
<tr>
<td>

```cpp
int i = 5;
if(whatever)
{
  i = 7;
}
else
{
  i = 9;
}
// use i
```

</td>
<td>

```cpp
int* i = &5;// or uninitialized
if(whatever)
{
  i = variable_scope &7;
}
else
{
  i = variable_scope &9;
}
// use i
```

</td>
<td>

```cpp
int& i = 5;
if(whatever)
{
  i &= variable_scope 7;
}
else
{
  i &= variable_scope 9;
}
// use i
```

</td>
</tr>
</table>

In the `values` example, there is no dangling. Programmers trust the compiler to allocate and deallocate instances on the stack. They have to because the programmer has little to no control over deallocation. Neither of the `pointers` or `references` examples compile without utility functions to convert from reference to pointer and immediately invoked lambda functions to do complex initialization of references since they have to be initialized and their references can't currently be reassigned no matter how useful that would be. That boiler plate exist in an earlier example in this proposal and has been removed for clarity. With the current `C++` statement scope rules or the `C99` block scope rule, both the `pointers` and `references` examples dangle. In other words, the compilers who are primarily responsible for the stack has rules that causes dangling and embarrassing worse immediate dangling. This violates the programmer's trust in their compiler. Variable scope is better because it restores the programmer's trust in their compiler/language by causing temporaries to match the value semantics of variables. Further, it avoids dangling throughout the body of the function whether it is anything that introduces new blocks/scopes be that `if`, `switch`, `while`, `for` statements and the nesting of these constructs.

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

### Doesn't this make C++ harder to teach?

Until the day that all dangling gets fixed, any new tools to assist developer's in fixing dangling would still require programmers to be able to identify any dangling and know how to fix it specific to the given scenario, as there are multiple solutions. Since dangling occurs even for things as simple as constants and immediate dangling is so naturally easy to produce than dangling resolution still have to be taught, even to beginners.

So, what do we teach now and what bearing does these teachings, the `C++` standard and this proposal have on one another.

**C++ Core Guidelines**<br/>**F.42: Return a `T*` to indicate a position (only)** [^cppcgrf42]<br/>***Note** Do not return a pointer to something that is not in the caller’s scope; see F.43.* [^cppcgrf43]

Returning references to something in the caller's scope is only natural. It is a part of our reference delegating programming model. A function when given a reference does not know how the instance was created and it doesn't care as long as it is good for the life of the function call and beyond. Unfortunately, scoping temporary arguments to the statement instead of the containing block doesn't just create immediate dangling but it provides to functions references to instances that are near death. These instances are almost dead on arrival. Having the ability to return a reference to a caller's instance or a sub-instance thereof assumes, correctly, that reference from the caller's scope would still be alive after this function call. The fact that temporary rules shortened the life to the statement is at odds with what we teach. This proposal allows programmers to restore to temporaries the lifetime of anonymously named variables which is not only natural but also consistent with what programmers already know. It is also in line with what we teach as was codified in the C++ Core Guidelines.
-->
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
