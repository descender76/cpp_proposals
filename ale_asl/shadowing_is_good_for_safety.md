<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2951R3</td>
</tr>
<tr>
<td>Date</td>
<td>2023-9-2</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>SG23 Safety and Security<br/>Evolution Working Group (EWG)<br/>Library Evolution Working Group (LEWG)</td>
</tr>
</table>

# Shadowing is good for safety
<!--
# Shadowing to safety
# Shadowing enhances safety
-->

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

- [Shadowing is good for safety](#Shadowing-is-good-for-safety)
  - [Changelog](#Changelog)
  - [Abstract](#Abstract)
  - [Motivational Examples](#Motivational-Examples)
    - [Removing Names](#Removing-Names)
    - [Reinitialization](#Reinitialization)
    - [Same Level Shadowing](#Same-Level-Shadowing)
    - [Conditional Casting](#Conditional-Casting)
    - [The checked range based for loop](#The-checked-range-based-for-loop)
  - [Safety and Security](#Safety-and-Security)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
    - [Why dename instead of unname?](#Why-dename-instead-of-unname)
    - [Why as instead of is?](#Why-as-instead-of-is)
  - [References](#References)

## Changelog

### R3

- minor verbiage changes and clarifying examples

### R2

- added the `5th request` for checked range based for loops

### R1

- updated `1st request`, explaining advantage of language solution over library solution in terms of error message
- added `std::optional` example to `2nd request`
- added the `4th request` and its corresponding FAQ
- added a `Safety and Security` section
- revised `Summary` section

## Abstract

Removing arbitrary limitations in the language associated with shadowing can improve how programmers deal with invalidation safety issues. It can also benefit a programmer's use of const correctness which impacts immutability and thread safety concerns. Regardless, it promotes simple and succinct code.

## Motivational Examples

### Removing Names

**1st request:** It would be beneficial if programmers could shadow a variable with void initialization instead of having to resort to a tag class.

<table>
<caption>removing names</caption>
<tr>
<th>

Present Workaround

</th>
<th>

Request

</th>
</tr>
<tr>
<td>

```cpp
#include <string>
#include <vector>

using namespace std;

struct dename{};// alt: unname, useless

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    dename vs;
    // or
    //decltype(ignore) vs;
    // this helps prevents iterator
    // and reference invalidation
    // as the programmer can't use
    // the container being worked on
  }
}
```

</td>
<td>

```cpp
#include <string>
#include <vector>

using namespace std;

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    void vs;
    // or
    // auto vs;
    // this helps prevents iterator
    // and reference invalidation
    // as the programmer can't use
    // the container being worked on
  }
}
```

</td>
</tr>
</table>

This feature allows programmers to relinquish access to the variable preventing all further operations that could jeopardize safety.

Even if this feature could not be standardized as a language feature, by removing a non breaking restriction, the tag class is adequate, provided the tag class name could be standardized as a library feature.

The advantage of having the language feature over the library feature would appear in the error message.

<table>
<tr>
<th>void vs;</th>
<td><span style="color:red">error: 'vs' was not declared in this scope</span></td>
</tr>
<tr>
<th>dename vs;</th>
<td><span style="color:red">error: 'struct dename' has no member named '*****'</span></td>
</tr>
</table>

In the language case, `void vs;`, the focus is on the variable that was denamed, which is better. With the library case, `dename vs;`, the focus is on the non existent member.
<!--
Either standardize `dename` as a library or allow `void` initialization.
-->

### Reinitialization
 
**2nd request:** It would be beneficial if programmers could initialize shadowed variables with the variable that is being shadowed.

<table>
<caption>shadowing limitation: initialization</caption>
<tr>
<th>

Present and Request

</th>
<th>

Present Workaround

</th>
</tr>
<tr>
<td>

```cpp
#include <string>
#include <vector>
#include <optional>

using namespace std;

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    // allow programmer to only call
    // non container changing methods
    const vector<string>& vs = vs;// error
  }
  //...
  auto s = optional<string>{"Godzilla"};
  if(s)
  {
    auto s = *s;// error
    // after tested the optional
    // is not needed anymore
  }
  return 0;
}
```

</td>
<td>

```cpp
#include <string>
#include <vector>
#include <optional>

using namespace std;

struct dename{};

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    // allow programmer to only call
    // non container changing methods
    const vector<string>& vs1 = vs;
    dename vs;
    // superfluous naming
    // superfluous line of code
  }
  //...
  auto os = optional<string>{"Godzilla"};
  if(os)
  {
    auto s = *os;
    dename os;
    // superfluous naming
    // superfluous line of code
  }
  return 0;
}
```

</td>
</tr>
</table>

By temporarily restricting access to non `const` methods, programmers prevent unintended mutations that could jeopardize safety.

### Implementation Experience

<table>
<tr>
<th>

gcc

</th>
<td>

this works in gcc

</td>
</tr>    
<tr>
<th>

clang

</th>
<td>

warning: reference 'vs' is not yet bound to a value when used within its own initialization [-Wuninitialized]

</td>
</tr>    
<tr>
<th>

msvc

</th>
<td>

warning C4700: uninitialized local variable 'vs' used

</td>
</tr>    
</table>

### Same Level Shadowing

**3rd request:** It would be beneficial if programmers could shadow variables without having to involve a child scope.

<table>
<caption>shadowing limitation: same level</caption>
<tr>
<th>

Present and Request

</th>
<th>

Present Workaround

</th>
</tr>
<tr>
<td>

```cpp
#include <string>
#include <vector>

using namespace std;

int main()
{
  vector<string> vs{"1", "2", "3"};
  // done doing complex initializaton
  // want it immutable here on out
  const vector<string>& vs = vs;// error
  return 0;
}
```

</td>
<td>

```cpp
#include <string>
#include <vector>

using namespace std;

struct dename{};

int main()
{
  vector<string> vs{"1", "2", "3"};
  // done doing complex initializaton
  // want it immutable here on out
  {
    const vector<string>& vs1 = vs;
    dename vs;
    // superfluous naming
    // superfluous lines of code
    // superfluous level
  }
  // OR use immediately invoked
  // lambda function
  [](const vector<string>& vs)
  {
    // superfluous lines of code
    // superfluous level
    // initialization split in two
    // variable declaration at begining of lambda
    // variable initialization at end of lambda
  }(vs);
  return 0;
}
```

</td>
</tr>
</table>

By relinquish access to non `const` methods as quickly as possible, programmers prevent unintended mutations that could jeopardize safety.

The error in the `Present and Request` example may read something like <span style="color:red">"error: conflicting declaration 'const vector&lt;string&gt; vs'//note: previous declaration as 'vector&lt;string&gt; vs'".</span>

### Conditional Casting

**4th request:** All of the previous requests have either been hiding a variable altogether or replacing it with an unconditionally casting. It would be beneficial if programmers had a mechanism for conditional casting.

<table>
<caption>NEW shadowing feature: conditional casting</caption>
<tr>
<th>

Request #4

</th>
<th>

Request #2

</th>
</tr>
<tr>
<td>

```cpp
#include <string>
#include <optional>
#include <memory>

using namespace std;

int main()
{
  auto s = optional<string>{"Godzilla"};
  if(s as string)// or if(s is string)
  {
    // s is string&
  }
  else
  {
    // s is still optional<string>
  }
  auto i = shared_ptr<int>{42};
  if(i as int)// or if(i is int)
  {
    // i is int&
  }
  else
  {
    // i is still shared_ptr<int>
  }
  return 0;
}
```

</td>
<td>

```cpp
#include <string>
#include <optional>
#include <memory>

using namespace std;

int main()
{
  auto s = optional<string>{"Godzilla"};
  if(s)
  {
    auto s = *s;
    // s is string&
  }
  else
  {
    // s is still optional<string>
  }
  auto i = shared_ptr<int>{42};
  if(i)
  {
    auto i = *i;
    // s is int&
  }
  else
  {
    // i is still shared_ptr<int>
  }
  return 0;
}
```

</td>
</tr>
</table>

This request is all about being simple and succint. The programmer need not know which conversion, `*`, `get()` or `value()`, gets paired with which test.

This is similar to what Herb Sutter proposed in `Pattern matching using is and as` [^as]. While that proposal was focused on pattern matching, this proposal is focused on shadowing and the resulting safety benefits presented.

This functionality exists in the Kotlin [^typecasts] and other programming languages.

### The checked range based for loop

**5th request:** This final request is very similar to the second request in example. Minimize the invalidation errors associated with range based `for` loop by limiting the usage of the instance being iterated over to const access only.

Three pieces are required for invalidation errors to occur.

1. A mutable instance
1. A reference instance that refers to the mutable instance and can be impacted by mutations in the mutable instance
1. Mutating the mutable instance

Because changes occur overwhelming at runtime instead of compile time, it is no big surprise that safety measures are applied at runtime via external test tools or by safer libraries with extra runtime checks built in.

In the range based `for` loop, the iterators, i.e. the reference instances, are concealed in the `for` construct. The following example is stripped from the 2nd example.

<table>
<caption>checked range based for loop</caption>
<tr>
<th>

Request

</th>
<th>

Present Workaround

</th>
</tr>
<tr>
<td>

```cpp
#include <string>
#include <vector>
#include <optional>

using namespace std;

int main()
{
  vector<string> vs{"1", "2", "3"};
  //[[checked]]
  //for (auto &s : vs) {
  // or
  cfor (auto &s : vs) {
    // automatically narrow vs to a constant &
  }
  return 0;
}
```

</td>
<td>

```cpp
#include <string>
#include <vector>
#include <optional>

using namespace std;

struct dename{};

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    const vector<string>& vs1 = vs;
    dename vs;
  }
  return 0;
}
```

</td>
</tr>
</table>

The challenge with implementing this request is that while simple variable names being iterated over could be shadowed easily, more complicated expressions would require pattern matching the expression as a whole and a combination of its components. Consequently, there are three increasing degrees of checks that would help programmers.

1. automatically shadow simple expressions such as `vs`
1. prevent complex expressions such as `vs.member.function().member` from being used elsewhere in the checked range based for loop in a non constant basis
1. prevent combinations of the expression from being used in the loop in a non constant manner

Examples of the latter two cases follows:

```cpp
[[checked]]
for (auto &s : vs.member.function().member) {
  // shouldn't be able to use `vs.member.function().member`
  // in a non const fashion
  vs.member.function().member.non_const_method();// error
}
// or
cfor (auto &s : vs.member.function().member) {
  auto first2 = vs.member;
  auto remainder = first2.function().member;
  // shouldn't be able to use `vs.member.function().member`
  // in a non const fashion
  remainder.non_const_method();// error
}
```

Even if these two more complicated scenarios were not handled, the single variable scenario is still of value even if programmers have to opt into this with `cfor` or `[[checked]]`.

## Safety and Security

Does these requests provide any safety or security?

*"In information security, computer science, and other fields, the principle of least privilege (PoLP), also known as the principle of minimal privilege (PoMP) or the principle of least authority (PoLA), requires that in a particular abstraction layer of a computing environment, every module (such as a process, a user, or a program, depending on the subject) must be able to access only the information and resources that are necessary for its legitimate purpose."* [^Principle_of_least_privilege]

Typically, when someone considers this principle it is in the context that someone is restricting the access of someone or something else. However, it also applies to someone who have been given access, then relinquishing that access or a portion of it when it is no longer needed. The later context is what shadowing is about. When a programmer dename/void their access to a variable they are relinquishing access for the remainder of the scope. When a programmer restrict their access to a variable to only its `const` methods than they relinquish part of their access. In both cases, it means you can't mutate an instance, which means the defensive programmer proactively avoid iterator, reference and pointer invalidation. Attempting to call a member that one does not have access to results in an error being provided by the compiler instead of by some external tool.

This is also related to borrow checking. Borrow checking is more about aliasing and restricting ownership to just 1. Shadowing allows programmers to reduce their aliases, actually accessible reference count, to 1 just like borrow checking and in some cases even farther to just 0.

Shadowing is of benefit of certain analyzers. Analyzer processing time is usually directly proportional to some number of instances or relationships between instances. For some analyzers, denaming instances could be used to reduces number of concerned instances. Shadowing in these cases would essentially be a analyzer hint.

## Summary

Since all of these requests are independent and compatible, the C++ community could adopt any combination of them. This proposal brings all these requests together because they are related.

The advantages of adopting said proposal are as follows:

1. It better allows programmers to use the compiler to avoid, debug and identify iterator, reference and pointer invalidation issues.
1. This proposal aids in const correctness which is good for immutability and thread safety.
1. More shadowing promotes simple and succinct code.

## Frequently Asked Questions

### Why dename instead of unname?

1. dename [^dename]: To remove the name from
2. unname [^unname]: To cease to name; to deprive (someone or something) of their name

While both names would work, dename seems simpler to me than unname. Personally, I think using `void` or slightly less so `auto` would be the more intuitive `C++` name.

### Why `as` instead of `is`?

1. `as` focuses on the casting and hence what a variable will be
2. `is` focuses on the testing and hence what a variable was

Since the `if` clause is more focused on what the variable will be instead of what it was, going with `as` over `is` seemed like a intuitive choice.

## References

<!--dename-->
[^dename]: <https://en.wiktionary.org/wiki/dename>
<!--unname-->
[^unname]: <https://en.wiktionary.org/wiki/unname>
<!--Principle of least privilege-->
[^Principle_of_least_privilege]: <https://en.wikipedia.org/wiki/Principle_of_least_privilege>
<!--Pattern matching using is and as-->
[^as]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2392r1.pdf>
<!--Type checks and casts-->
[^typecasts]: <https://kotlinlang.org/docs/typecasts.html>
<!---->
<!--
[^]: <>
-->
