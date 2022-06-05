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
  - [Motivating examples](#motivating-examples)
  - [Implementation Experience](#implementation-experience)
  - [References](#references)

## Abstract

There is more to member function pointers than calling a member function virtually.

## Motivating Examples

`"Direct" vs "virtual" call to a virtual function` [^directvsvirtual]

```cpp
SomeObject obj;
SomeObject *pobj = &obj;
SomeObject &robj = obj;

obj.some_virtual_function(); // Direct call
pobj->some_virtual_function(); // Virtual call in general case
robj.some_virtual_function(); // Virtual call in general case

obj.SomeObject::some_virtual_function(); // Direct call
pobj->SomeObject::some_virtual_function(); // Direct call
robj.SomeObject::some_virtual_function(); // Direct call
```

`Working Draft, Standard for Programming Language C++` [^n4910]

"*11.7.3 Virtual functions [class.virtual]"*

"*16 Explicit qualifcation with the scope operator (7.5.4.3) suppresses the virtual call mechanism."*

"*[Example 10:"*

```cpp
class B { public: virtual void f(); };
class D : public B { public: void f(); };

void D::f() { /* ... */ B::f(); }
```

*"Here, the function call in D::f really does call B::f and not D::f. — end example]"*

## Implementation Experience

GCC has support acquiring the function pointer from a member function pointer via its `bound member function` [^boundmemberfunctions] feature. While their implementation supports the functionality at both runtime and at compile time, this proposal is solely concerned about compile time. Also while GCC uses a casting mechanism, this proposal is asking for a intrinsic function to better support `auto` for deduction.

## References

<!--"Direct" vs "virtual" call to a virtual function-->
[^directvsvirtual]: <https://stackoverflow.com/questions/36584670/direct-vs-virtual-call-to-a-virtual-function>
<!--Working Draft, Standard for Programming Language C++-->
[^n4910]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4910.pdf>
<!--Extracting the Function Pointer from a Bound Pointer to Member Function-->
[^boundmemberfunctions]: <https://gcc.gnu.org/onlinedocs/gcc/Bound-member-functions.html>
