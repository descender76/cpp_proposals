<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2951R0</td>
</tr>
<tr>
<td>Date</td>
<td>2023-7-14</td>
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
  - [Abstract](#Abstract)
  - [Motivational Examples](#Motivational-Examples)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

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

using namespace std;

int main()
{
  vector<string> vs{"1", "2", "3"};
  for (auto &s : vs) {
    // allow programmer to only call
    // non container changing methods
    const vector<string>& vs = vs;// error
  }
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
  for (auto &s : vs) {
    // allow programmer to only call
    // non container changing methods
    const vector<string>& vs1 = vs;
    dename vs;
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

## Summary

The advantages of adopting said proposal are as follows:

1. It better allows programmers to use the compiler to avoid, debug and identify iterator, reference and pointer invalidation issues.
1. This proposal aids in const correctness which is good for immutability and thread safety.
1. More shadowing promotes simple and succinct code.

## Frequently Asked Questions

### Why dename instead of unname?

1. dename [^dename]: To remove the name from
2. unname [^unname]: To cease to name; to deprive (someone or something) of their name

While both names would work, dename seems simpler to me than unname. Personally, I think using `void` would be the more intuitive `C++` name.

## References

<!--dename-->
[^dename]: <https://en.wiktionary.org/wiki/dename>
<!--unname-->
[^unname]: <https://en.wiktionary.org/wiki/unname>
<!---->
<!--
[^]: <>
-->
