|                 |                                                           |
|-----------------|-----------------------------------------------------------|
| Document number | P2472R2                                                   |
| Date            | 2022-01-02                                                |
| Reply-to        | Jarrad J. Waterloo <<descender76@gmail.com>>              |
| Audience        | Library Evolution Working Group (LEWG)                    |
| Project         | ISO JTC1/SC22/WG21: Programming Language C++              |

# make `function_ref` more functional

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

- [make `function_ref` more functional](#make-functionref-more-functional)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [Solution](#solution)
  - [Feature test macro](#feature-test-macro)
  - [Other Languages](#other-languages)
  - [Example implementation](#example-implementation)
  - [Acknowledgments](#acknowledgments)
  - [References](#references)

## Abstract

I propose adding two/four constructors to `function_ref` [^p0792r6] in order to make it easier to use, more efficient and safer to use with common use cases.

Currently, a `function_ref` [^p0792r6], can be constructed from a lambda/functor, a free function pointer and a member function pointer. While the lambda/functor use case does supports type erasing the `this` pointer, its free/member function pointer constructors does NOT allow type erasing any arguments even though these two use cases are among the most common and valuable in the programming world.

<table>
<tr>
<td colspan="3">

`function_ref` [^p0792r6]

</td>
</tr>
<tr>
<td></td>
<td>Stateless</td>
<td>Stateful</td>
</tr>
<tr>
<td>Lambda/Functor</td>
<td>&#128504;</td>
<td>&#128504;</td>
</tr>
<tr>
<td>Free Function</td>
<td>&#128504;</td>
<td>&#10007;</td>
</tr>
<tr>
<td>Member Function</td>
<td>&#128504;</td>
<td>&#10007;</td>
</tr>
</table>

## Motivating Examples

### Given

```cpp
struct bar {
    void baz() {
    }
};

void foo(bar& b) {
}

struct callback {
    bar* b;
    void (*f)(bar&);
};

bar b;
```

Since typed erased free and member functions are not currently supported, the current `function_ref` [^p0792r6] proposal forces its users to create a unnecessary temporary `stateful` lambda which the developer hopes the optimizer removes.

#### member function with type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
callback cb = {&b, [](bar& b){b.baz();}};
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
// separate temp needed to prevent dangling
// when temp is passed to multiple arguments
auto temp = [&b](){b.baz();};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&b](){b.baz();});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<&bar::baz>, b};
```

</td>
</tr>
</table>

#### free function with type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
callback cb = {&b, [](bar& b){foo(b);}};
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
// separate temp needed to prevent dangling
// when temp is passed to multiple arguments
auto temp = [&b](){foo(b);};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&b](){foo(b);});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<foo>, b}
// or
function_ref<void()> fr = {nontype<&foo>, b}
```

</td>
</tr>
</table>

This has numerous disadvantages when compared to what can currently be performed in the C/C++ core language. It is not easy to use, it is inefficient and at times unsafe to use.

##### Not easy to use

* Unlike the consistent direct initialization of the C/C++ core language example, the initialization of function_ref is `bifurcated` among passing it directly as a function argument or using 2 step initialization by first creating a named temporary. Direct initialization of function_ref leads to immediate dangling.
* Why should a `stateful` lambda even be needed?
  * The C/C++ core language example works with a `stateless` lambda. By requiring a `stateful` lambda, a new anonymous type gets created which has state. That state's lifetime also needs to be managed by the user.
  * By requiring lambda, the user must manually forward the function arguments. In the C/C++ core language example, the syntax can get even simpler when the function already exists because in that case the function pointer is passed to the `callback` struct because the function signature is compatible.

##### Inefficient

* By requiring a `stateful` lambda, a new type is created with a lifetime that must be managed. `function_ref` is a reference to a `stateful` lambda which also is a reference.
* In the case of member functions, function_ref stores a reference to member function pointer which means calling the member function means going through the fatness of the member function pointer. That could include going through a v-table and multiple inheritance. In the proposed, `nontype` does not take a pointer but rather a [member] function pointer initialization statement thus giving the compiler more information to resolve the function selection at compile time rather than run time.

##### Unsafe

* Direct initialization of function_ref outside of a function argument immediately dangles. Some would say that this is no different than other standardized types such as string_view.
  ```cpp
  std::string_view sv = "hello world"s;  // immediately dangling
  ```
  Unfortunate there is a big difference. While with string_view it is apparent that dangling occurs but with `function_ref` of existing instances it is not `expected` to dangle. In this string_view example, it is clear that sv is a reference and ""s is state. It is totally unclear in the case of function_ref initialization. Dangling is not what the end user is intending on doing when a function is assigned whether free or member. Functions are stateless and also global, so they don't go out of scope. So the state that is `expected` to be referenced by `function_ref` should be the type erased argument such as `this` rather than a temporary lambda/functor itself.

|                         | easier to use | more efficient | safer to use |
|-------------------------|---------------|----------------|--------------|
| **C/C++ core language** | &#128504;     | &#128504;      | &#128504;    |
| **function_ref**        | &#10007;      | &#10007;       | &#10007;     |
| **proposed**            | &#128504;     | &#128504;      | &#128504;    |

What is more member/free function with type erasure are common use cases! `member function with type erasure` is used in delegates in object oriented programming languages and `free function with type erasure` are common with callbacks in procedural/functional programming languages. Why should the call operator be treated more important than any other member function with the same signature?

The fact that users do not expect `stateless` things to dangle becomes even more apparent with the member function without type erasure use case.  

#### member function without type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```
void (bar::*mf)() = &bar::baz;
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
// separate temp needed to prevent dangling
// when temp is passed to multiple arguments
auto temp = &bar::baz;
function_ref<void(bar&)> fr = temp;
// or when given directly as a function argument
some_function(&bar::baz);
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(bar&)> fr = {nontype<&bar::baz>};
```

</td>
</tr>
</table>

Current `function_ref` implementations store a reference to the member function pointer as the state inside `function_ref`. A trampoline function is required regardless. However the user `expected` behavior is for `function_ref` referenced state to be unused/`nullptr` as all the arguments has to be forwarded as none are being type erased. As such dangling is `NEVER` expected and yet the current `function_ref` [proposal/implementation] does.
Similarly this use case similarly suffers as the previous two with respect to ease of of use, efficiency and safety due to the superfluous lambda/functor and two step initialization.

|                         | easier to use | more efficient | safer to use |
|-------------------------|---------------|----------------|--------------|
| **C/C++ core language** | &#128504;     | &#128504;      | &#128504;    |
| **function_ref**        | &#10007;      | &#10007;       | &#10007;     |
| **proposed**            | &#128504;     | &#128504;      | &#128504;    |

The C/C++ core language, `function_ref` and the proposed examples are approximately equal with respect to ease of use, efficiency and safety for the free function without type erasure use case. While the proposed `nontype` example is slightly more wordy because of using the template `nontype`, it is more consistent with the other three use cases making it more teachable and usable not having to know when to do one versus the other. Also the expectation of unused state and the function being selected at compile time still applies here as it does for member function without type erasure use case. 

#### free function without type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
void (*f)(bar&) = foo;
// or
void (*f)(bar&) = &foo;
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
function_ref<void(bar&)> fr = foo;
// or
function_ref<void(bar&)> fr = &foo;
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(bar&)> fr = {nontype<foo>};
// or
function_ref<void(bar&)> fr = {nontype<&foo>};
```

</td>
</tr>
</table>

|                         | easier to use | more efficient | safer to use |
|-------------------------|---------------|----------------|--------------|
| **C/C++ core language** | &#128504;     | &#128504;      | &#128504;    |
| **function_ref**        | &#128504;     | &#128504;      | &#128504;    |
| **proposed**            | &#128504;     | &#128504;      | &#128504;    |

#### Remove existing free function constructor?

With the overlap in functionality with the free function without type erasure use case, should the existing free function constructor be removed? NO. Initializing a `function_ref` from a function pointer instead of function pointer initialization statement is still usable in the most runtime of libraries such as runtime dynamic library linking where a function pointer is looked by a string. It is just not the general case when working with type declarations found in header files and modules.

## Solution

```cpp
template<class R, class... ArgTypes> class function_ref<R(ArgTypes...) cv noexcept(noex)>
public:
  template<class MFP, class I> function_ref(nontype<MFP>, I*) noexcept;
  // MFP is a member function pointer
  // I is an instance of the type that house the member function pointed to by MFP
  template<class MFP> function_ref(nontype<MFP>) noexcept;
  template<class FP, class FST> function_ref(nontype<FP>, FST*) noexcept;
  // FP is a free function pointer
  // FST is the type of the first parameter of the free function pointed to by FP
  template<class FP> function_ref(nontype<FP>) noexcept;
};
```

## Feature test macro

We do not need a feature macro, because we intend for this paper to modify std::function_ref before it ships.

## Other Languages
C# and the .NET family of languages provide this via `delegates` [^delegates].

```cpp
// C#
delegate void some_name();
some_name fr = foo;// the stateless free function use case
some_name fr = b.baz;// the stateful member function use case
```

Borland C++ now embarcadero provide this via `__closure` [^closure].

```cpp
// Borland C++, embarcadero __closure
void(__closure * fr)();
fr = foo;// the stateless free function use case
fr = b.baz;// the stateful member function use case
```

Since make_function_ref handles all 4 statess/stateful free/member use cases, it is more feature rich than either of the above.

## Example implementation

The most up-to-date implementation, created by Zhihao Yuan, is available on `Github` [^nontype]

## Acknowledgments

Thanks to Arthur O'Dwyer, Tomasz Kamiński, Corentin Jabot and Zhihao Yuan for providing very valuable feedback on this proposal.

## References

[^p0792r6]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0792r6.html>
[^functionref]: <https://github.com/TartanLlama/function_ref>
[^functionrefprime]: <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>
[^nontype]: <https://github.com/zhihaoy/function_ref_nontype/blob/main/include/function_ref.h>
[^delegates]: <https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/using-delegates>
[^closure]: <http://docwiki.embarcadero.com/RADStudio/Sydney/en/Closure>
[^fastest]: <https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible>
[^impossiblyc]: <https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates>
[^impossiblyd]: <https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11>
[^dyno]: <https://github.com/ldionne/dyno>
[^boostextte]: <https://github.com/boost-ext/te>

