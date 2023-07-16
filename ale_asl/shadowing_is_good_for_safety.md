<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2951R1</td>
</tr>
<tr>
<td>Date</td>
<td>2023-7-15</td>
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
  - [Safety and Security](#Safety-and-Security)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Changelog

### R1

- updated `1st request`, explaining advantage of language solution over library solution in terms of error message
- added `std::optional` example to `2nd request`
- added a `Safety and Security` section
- revised `Summary` section

## Abstract

Removing arbitrary limitations in the language associated with shadowing can improve how programmers deal with invalidation safety issues. It can also benefit a programmer's use of const correctness which impacts immutability and thread safety concerns. Regardless, it promotes simple and succinct code.

## Motivational Examples

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

In the language, `void vs;`, case, the focus is on the variable that was denamed, which is better. While the library, `dename vs;`, case, the focus is on the non existent member.
<!--
Either standardize `dename` as a library or allow `void` initialization.
-->
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

**3rd request:** It would be beneficial if programmers could shadow variables without having involve a child scope.

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
  }(vs);
  return 0;
}
```

</td>
</tr>
</table>

The error in the `Present and Request` example may read something like <span style="color:red">"error: conflicting declaration 'const vector&lt;string&gt; vs'//note: previous declaration as 'vector&lt;string&gt; vs'".</span>
    
## Safety and Security

Does these requests provide any safety or security?

*"In information security, computer science, and other fields, the principle of least privilege (PoLP), also known as the principle of minimal privilege (PoMP) or the principle of least authority (PoLA), requires that in a particular abstraction layer of a computing environment, every module (such as a process, a user, or a program, depending on the subject) must be able to access only the information and resources that are necessary for its legitimate purpose."* [^Principle_of_least_privilege]

Typically when someone considers this principle it is in the context that someone else is restricting the access of someone or something else. However, it also applies to someone who have been given access, then relinquishing that access or a portion of it when it is no longer needed. The later context is what shadowing is about. When a programmer dename/void their access to a variable they are relinquishing access for the remainder of the scope. When a programmer restrict their access to a variable to only its `const` methods than they relinquish part of their access. In both cases, it means you can't mutate an instance, which means the defensive programmer proactively avoid iterator, reference and pointer invalidation. Attempting to call a member that one does not have access to results in an error being provided by the compiler instead of by some external tool.

This is also related to borrow checking. Borrow checking is more about aliasing and restricting ownership to just 1. Shadowing allows programmers to reduce their aliases, actually accessible reference count, to 1 just like borrow checking and in some cases even farther to just 0.

Shadowing is of benefit of certain analyzers. Analyzer processing time is usually directly proportional to some number of instances or relationships between instances. For some analyzers, denaming instances could be used to reduces number of concerned instances. Shadowing in these cases would essentially be a analyzer hint.

## Summary

All of these requests are mutually exclusive, the C++ community, could adopt any combination of them. This proposal brings all these requests together because they are related.

The advantages of adopting said proposal are as follows:

1. It better allows programmers to use the compiler to avoid, debug and identify iterator, reference and pointer invalidation issues.
1. This proposal aids in const correctness which is good for immutability and thread safety.
1. More shadowing promotes simple and succinct code.

## Frequently Asked Questions

### Why dename instead of unname?

1. dename [^dename]: To remove the name from
2. unname [^unname]: To cease to name; to deprive (someone or something) of their name

While both names would work, dename seems simpler to me than unname. Personally, I think using `void` or slightly less so `auto` would be the more intuitive `C++` name.

## References

<!--dename-->
[^dename]: <https://en.wiktionary.org/wiki/dename>
<!--unname-->
[^unname]: <https://en.wiktionary.org/wiki/unname>
<!--Principle of least privilege-->
[^Principle_of_least_privilege]: <https://en.wikipedia.org/wiki/Principle_of_least_privilege>
<!--Pattern matching using is and as-->
[^as]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2392r1.pdf>
<!---->
<!--
[^]: <>
-->

