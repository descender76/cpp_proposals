<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2740R0</td>
</tr>
<tr>
<td>Date</td>
<td>2022-12-11</td>
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

# Simpler implicit dangling resolution

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

- [Simpler implicit dangling resolution](#Simpler-implicit-dangling-resolution)
  - [Abstract](#Abstract)
  - [Motivation](#Motivation)
  - [Motivating examples](#Motivating-Examples)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Abstract

The `Simpler implicit move` proposal [^p2266r3] provided some dangling relief to the `C++` language. However, this relief was a consequence of fixing `implicit move`. Consequently, it did not clearly address similarly related types of dangling. This paper asks that similar fixes apply to related scenarios in order to ease more dangling in the language. This by no means fixes all dangling in general but it does compliment other proposals that address other contributing factors.

## Motivation

There are multiple resolutions to dangling in the `C++` language.

1. Produce an error
    - `Simpler implicit move` [^p2266r3]
    - **This proposal**
1. Fix with block/variable scoping
    - `Fix the range-based for loop, Rev2` [^p2012r2]
    - `Get Fix of Broken Range-based for Loop Finally Done` [^p2644r0]
1. Fix by making the instance global

All are valid resolutions and individually are better than the others, given the scenario. This proposal is focused on the first option, which is to produce errors when code is clearly wrong.

We first need to examine the dangling fixes provided by the `Simpler implicit move` [^p2266r3] proposal. These fixes helps to mitigate returning references to locals which is always wrong and as such should be errors in the programming language.

<table>
<tr>
<td>

**C++ Core Guidelines**<br/>**F.43: Never (directly or indirectly) return a pointer or a reference to a local object** [^cppcgrf43]

***Reason** To avoid the crashes and data corruption that can result from the use of such a dangling pointer.* [^cppcgrf43]

...

***Note** This applies only to non-static local variables.* [^cppcgrf43]

</td>
</tr>
</table>

The resolution in this **first example** addresses **directly** returning a reference to a local.

```cpp
int& h(bool b, int i) {
  static int s;
  if (b) {
    return s;  // OK
  } else {
    return i;  // error: i is an xvalue
  }
}
```

The resolution in this **second example** addresses one additional level of **indirectly** returning a `std::reference_wrapper` to a local.

```cpp
std::reference_wrapper<Widget> fifteen() {
    Widget w;
    return w;  // OK until CWG1579; OK after LWG2993. Proposed: ill-formed
}
```

Both fixes are welcomed but much more can be gained at minimal cost.

## Motivating Examples

Any given function, already knows whether it returns a pointer or a reference. It knows whether any instances within the function are locals or globals. It also knows the lifetimes of these instances and the pointers and references that refers to them. In other words, a compiler does not need to go outside of the function in question for this information with the exception of globals. **Direct** pointers/references are easy but **indirect** requires computing a graph of instance dependencies. With the **indirect** exception of the **second example**, which is only one level away from its instance, this proposal is focused on that which is easy for all compilers to compute and verify.

### The wish

If the `Simpler implicit move` [^p2266r3] proposal does not fix the following examples than it would be beneficial if `C++` was enhanced to include these fixes, along similar lines.

Fix the pointer version of the **first example**.

```cpp
int* h(bool b, int i) {
  static int s;
  if (b) {
    return &s;  // OK
  } else {
    return &i;  // error: instance `i` dies before pointer `p`
  }
}
```

Just as the pointer or reference to a local should not be returned because it exits the scope of the lifetime of the local, this should also occur in the body of any given function.

```cpp
void h(bool b, int i) {
  int* p = nullptr;
  if (b) {
    static int s;
    p = &s;  // OK
  } else {
    int i = 0; 
    p = &i;  // error: instance `i` dies before pointer `p`
  }
}
```

In order to address these types of dangling, in the language, we need to add a rule into the standard.

<span style="color:red">**RULE: You can not directly assign the address of an instance to a pointer or a reference if the instance dies before the pointer or reference dies.**</span>

At worse, this is dangling. At best, this is a logic error.

I am not asking in this proposal for a resolution for a `std::optional<std::reference_wrapper<int>>` version of the last example. While that does need to be fixed that would get into two additional levels of indirection. I am limiting this proposal just to one level as the `Simpler implicit move` [^p2266r3] proposal did. Any more levels of indirection would require compilers, at greater compilation cost, to process the instance dependency graph and then do more when programmers can contribute to the dependency information via an attribute that documents whether returns and parameters are dependent upon one another.

The next set of examples are related to the **second example** that has just one additional level of **indirect**ion. Just like `std::reference_wrapper`, lambda functions and coroutines that have pointers or references to locals should not be able to be returned from a function.

```cpp
auto lambda() {
    int i = 0;
    return [&i]() -> &int
        {
            return i;
        };  // error: instance `i` dies before the returned lambda
}
```

```cpp
auto coroutine() {
    int i = 0;
    return [&i]() -> generator<int>
        {
            co_return i;
        };  // error: instance `i` dies before the returned coroutine
}
```

Consequently, another rule needs to be added.

<span style="color:red">**RULE: You can not return a lambda or coroutine that captures a pointer or reference to a local.**</span>

While a combination of these two rules would mean `std::optional` of a lambda or coroutine that captures a pointer or reference to a local should also be invalid, this proposal does not ask for that to be fixed since it would get into two additional levels of indirection. I am limiting this proposal just to one additional level of indirection as the `Simpler implicit move` [^p2266r3] proposal did. Any more levels of indirection would require compilers, at greater compilation cost, to process the instance dependency graph and then do more when programmers can contribute to the dependency information via an attribute that documents whether returns and parameters are dependent upon one another.

## Summary

`C++` can fix more dangling simply in the language without the need for any additional non standard static analyzers. The cost is minimal but like all dangling these defects are big. Enforcing these rules will improve the safety of `C++`. Further, the pointer versions of these safety checks could be contributed back to `C` thus making it safer and improving our ecosystem as a whole.

## Frequently Asked Questions

### Why not handle more indirect cases?

Compilers and the standard should. Forget the increased compile time, compilers should create and analyze the instance dependency graph to produce as many errors as possible to make `C++` much safer. Library authors should be able to contribute to this graph for more of the indirect cases. This can be done in three phases.

- direct dangling and low indirection level count dangling as exemplified by the `Simpler implicit move` [^p2266r3]
- maximum indirect dangling detection without library author contributions
- maximum indirect dangling detection with library author contributions

As the first phase is cheaper and easier to do than we should get this and similar proposals into the language as quickly as possible as a triage measure. Later, the other phases can be addressed in turn whether in the same or succeeding releases.

## References

<!--Simpler implicit move-->
[^p2266r3]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2266r3.html>
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
<!--A type trait to detect reference binding to temporary-->
[^p2255r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2255r2.html>
<!--Fix the range‐based for loop, Rev1-->
[^p2012r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2012r1.pdf>
<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgrf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>
