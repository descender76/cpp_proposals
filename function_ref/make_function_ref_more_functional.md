<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2472R2</td>
</tr>
<tr>
<td>Date</td>
<td>2022-03-08</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo <<descender76 at gmail dot com>>

Zhihao Yuan <<zy at miator dot net>>

</td>
</tr>
<tr>
<td>Audience</td>
<td>Library Evolution Working Group (LEWG)</td>
</tr>
</table>

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
  - [Changelog](#changelog)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [Wording](#wording)
  - [Feature test macro](#feature-test-macro)
  - [Other Languages](#other-languages)
  - [Example implementation](#example-implementation)
  - [Acknowledgments](#acknowledgments)
  - [References](#references)

## Changelog

### R1

- Moved from my make_function_ref implementation to Zhihao Yuan's nontype implementation
  - Constructors were always my ideal over factory function but I was unable to make it work.
  - nontype has better type deduction
  - Most of the changes were syntactical and reasoning based on feedback as well as making examples even more concise

### R2

- Added this changelog
- Transformed solution into the wording
- Added a third constructor which takes a pointer instead of a reference for convenience
- Revised example from cat to rule/test
- Added deduction guide

## Abstract

This document proposes adding additional constructors to `function_ref` [^p0792r6] in order to make it easier to use, more efficient and safer to use with common use cases.

Currently, a `function_ref`, can be constructed from a lambda/functor, a free function pointer and a member function pointer. While the lambda/functor use case does supports type erasing the `this` pointer, its free/member function pointer constructors does NOT allow type erasing any arguments, even though these two use cases are common.

<table>
<tr>
<td colspan="3">

`function_ref`

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
struct rule {
    void when() {
    }
};
// rule could just as easily be test which also have when and then functions

void then(rule& r) {
}

void rule_when(rule& r) {
    r.when();
}

struct callback {
    rule* r;
    void (*f)(rule&);
};

rule localTaxRule;
```

#### member/free function with type erasure

Since typed erased free and member functions are not currently supported, the current `function_ref` proposal forces its users to create a unnecessary temporary functor likely with std::bind_front or a `stateful`/capturing lambda which the developer hopes the optimizer elides.

##### member function with type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
callback cb = {&localTaxRule, [](rule& r){r.when();}};
// or
callback cb = {&localTaxRule, rule_when};
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
auto temp = [&localTaxRule](){localTaxRule.when();};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&localTaxRule](){localTaxRule.when();});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<&rule::when>, localTaxRule};
```

</td>
</tr>
</table>

##### free function with type erasure

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
callback cb = {&localTaxRule, [](rule& r){then(r);}};
// or
callback cb = {&localTaxRule, then};
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
auto temp = [&localTaxRule](){then(localTaxRule);};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&localTaxRule](){then(localTaxRule);});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<then>, localTaxRule}
```

</td>
</tr>
</table>

This has numerous disadvantages when compared to what can currently be performed in the C/C++ core language. It is not easy to use, it is inefficient and at times unsafe to use.

##### Not easy to use

* Unlike the consistent direct initialization of the C/C++ core language example, the initialization of `function_ref` is `bifurcated` among passing it directly as a function argument or using 2 step initialization by first creating a named temporary. Direct initialization of function_ref to a variable would be welcomed if the same function_ref is passed to multiple arguments on either the same but more likely different functions. This leads to immediate dangling which must be resolved with an additional line of code. Getting in the habit of only passing `function_ref` as a argument to a function results in users duplicating lambdas when needed to be used more than once, again needless increasing the volume of code. In both the C/C++ core language and the proposed revision, initialization is consistantly the same and safe regardless of whether `function_ref` is first assigned to a variable.
* Why should a `stateful` lambda even be needed when the signature is compatible?
  * The C/C++ core language example works with a `stateless` lambda. By requiring a `stateful` lambda, a new anonymous type gets created which has state. That state's lifetime also needs to be managed by the user.
  * When using a lambda, the user must manually forward the function arguments. In the C/C++ core language example, the syntax can get even simpler when the function already exists because in that case the function pointer is passed to the `callback` struct because the function signature is compatible.

##### Inefficient

* By requiring a `stateful` lambda, a new type is created with a lifetime that must be managed. `function_ref` is a reference to a `stateful` lambda which also is a reference. Even if the optimizer can remove the cost, the user is still left with the burden in the code.
* In the proposed, `nontype` does not take a pointer but rather a [member] function pointer initialization statement thus giving the compiler more information to resolve the function selection at compile time rather than run time, granted it is more likely with free functions instead of member functions.

##### Unsafe

* Direct initialization of function_ref outside of a function argument immediately dangles. Some would say that this is no different than other standardized types such as string_view. However, this is not true.
  <table>
  <tr>
  <td>
  C/C++ core language
  </td>
  <td>
  
  ```cpp
  std::string_view sv = "hello world"s;// immediately dangling
  auto c = "hello world"s;
  std::string_view sv = c;// DOES NOT DANGLE
  ```
  
  </td>
  </tr>
  <tr>
  <td>
  function_ref
  </td>
  <td>
  
  ```cpp
  function_ref<void()> fr = [&localTaxRule](){localTaxRule.when();};// immediately dangling
  // or
  function_ref<void()> fr = [&localTaxRule](){then(localTaxRule);};// immediately dangling
  ```
  
  </td>
  </tr>
  <tr>
  <td>
  proposed
  </td>
  <td>
  
  ```cpp
  function_ref<void()> fr = {nontype<&rule::when>, localTaxRule};// DOES NOT DANGLE
  // or
  function_ref<void()> fr = {nontype<then>, localTaxRule}// DOES NOT DANGLE
  ```
  
  </td>
  </tr>
  </table>
  While both the original `function_ref` proposal and the proposed addendum perform the same desired task, the former dangles and the later doesn't. It is clear from the immediately dangling `string_view` example that it dangles because `sv` is a reference and `""s` is a temporary. However, it is less clear in the original `function_ref` example. While it is true and clear that `fr` is a reference and the stateful lambda is a temporary. It is not what the user of `function_ref` is intending or wanting to express. `localTaxRule` is the state that the user wants to type erase and `function_ref` would not dangle if `localTaxRule` was the state since it is not a temporary and has already been constructed higher up in the call stack. Further, the member function `when` or the free function `then` should not dangle since functions are stateless and also global. So member/free function with type erasure use cases are more like `string_view` when the referenced object is safely constructed. 

|                         | easier to use | more efficient | safer to use |
|-------------------------|---------------|----------------|--------------|
| **C/C++ core language** | &#128504;     | &#128504;      | &#128504;    |
| **function_ref**        | &#10007;      | &#10007;       | &#10007;     |
| **proposed**            | &#128504;     | &#128504;      | &#128504;    |

What is more, member/free function with type erasure are common use cases! `member function with type erasure` is used by delegates/events in object oriented programming languages and `free function with type erasure` are common with callbacks in procedural/functional programming languages.

#### member function without type erasure

The fact that users do not expect `stateless` things to dangle becomes even more apparent with the member function without type erasure use case.  

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```
void (rule::*mf)() = &rule::when;
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
auto temp = &rule::when;
function_ref<void(rule&)> fr = temp;
// or when given directly as a function argument
some_function(&rule::when);
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(rule&)> fr = {nontype<&rule::when>};
```

</td>
</tr>
</table>

Current `function_ref` implementations store a reference to the member function pointer as the state inside `function_ref`. A trampoline function is required regardless. However the user `expected` behavior is for `function_ref` referenced state to be unused/`nullptr`, as `ALL` of the arguments has to be forwarded since `NONE` are being type erased. As such dangling is `NEVER` expected and yet the current `function_ref` [proposal/implementation] does.
Similarly, this use case suffers, just as the previous two did, with respect to ease of of use, efficiency and safety due to the superfluous lambda/functor and two step initialization. Further if the size of `function_ref` is increased beyond 2 pointers to just make the original proposal work for member function pointers, when it is not needed since only a reference to state and a pointer to a trampoline function is needed, then all use cases are pessimistically increased in size.

|                         | easier to use | more efficient | safer to use |
|-------------------------|---------------|----------------|--------------|
| **C/C++ core language** | &#128504;     | &#128504;      | &#128504;    |
| **function_ref**        | &#10007;      | &#10007;       | &#10007;     |
| **proposed**            | &#128504;     | &#128504;      | &#128504;    |

#### free function without type erasure

The C/C++ core language, `function_ref` and the proposed examples are approximately equal with respect to ease of use, efficiency and safety for the free function without type erasure use case. While the proposed `nontype` example is slightly more wordy because of using the template `nontype`, it is more consistent with the other three use cases, making it more teachable and usable since the user does not have to know when to do one versus the other i.e. less bifurcation. Also the expectation of unused state and the function being selected at compile time still applies here, as it does for member function without type erasure use case. 

<table>
<tr>
<td>
C/C++ core language
</td>
<td>

```cpp
void (*f)(rule&) = then;
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
function_ref<void(rule&)> fr = then;
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(rule&)> fr = {nontype<then>};
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

With the overlap in functionality with the free function without type erasure use case, should the existing free function constructor be removed? NO. Initializing a `function_ref` from a function pointer instead of function pointer initialization statement is still usable in the most runtime of libraries such as runtime dynamic library linking where a function pointer is looked up by a string or some other identifier. It is just not the general case, where users work with declarations found in header files and modules.

## Wording

The wording is relative to P0792R8.

Add new templates to 20.2.1 [utility.syn], header `<utility>` synopsis after `in_place_index_t` and `in_place_index`:

```cpp
namespace std {
  [...]

  // nontype argument tag
  template<auto V>
    struct nontype_t {
      explicit nontype_t() = default;
    };

  template<auto V> inline constexpr nontype_t<V> nontype{};
}
```

Add a definition to 20.14.3 [func.def]:

```cpp
template<auto f> constexpr function_ref(nontype_t<f>) noexcept;
```

> *Constraints:* `is-invocable-using<decltype(f)>` is `true`.
> 
> *Effects:* Initializes `bound-entity` with `nullptr`, and `thunk-ptr` to address of a function such that `thunk-ptr(bound-entity, call-args...)` is expression equivalent to `invoke_r<R>(f, nullptr, call-args...)`.

```cpp
template<auto f, class T> function_ref(nontype_t<f>, T& state) noexcept;
```

> 
> *Constraints:* `is-invocable-using<decltype(f), cv T&>` is true.
> 
> *Effects:* Initializes `bound-entity` with `addressof(state)`, and `thunk-ptr` to address of a function such that `thunk-ptr(bound-entity, call-args...)` is expression equivalent to `invoke_r<R>(f, static_cast<T cv&>(bound-entity), call-args...)`.

```cpp
template<auto f, class T> function_ref(nontype_t<f>, cv T* state) noexcept;
```

> 
> *Constraints:* `is-invocable-using<decltype(f), cv T*>` is `true`.
> 
> *Effects:* Initializes `bound-entity` with `state`, and `thunk-ptr` to address of a function such that `thunk-ptr(bound-entity, call-args...)` is expression equivalent to `invoke_r<R>(f, static_cast<cv T*>(bound-entity), call-args...)`.

```cpp
  template<class R, class... ArgTypes>
  class function_ref<R(ArgTypes...) cv ref noexcept(noex)> {
  public:
    // [func.wrap.ref.ctor], constructors ...
    ...
    template<auto f> constexpr function_ref(nontype_t<f>) noexcept;
    template<auto f, class T> function_ref(nontype_t<f>, T&) noexcept;
    template<auto f, class T> function_ref(nontype_t<f>, cv T*) noexcept;

    ...
  };

  // [func.wrap.ref.deduct], deduction guides
  template<auto f>
    function_ref(nontype_t<f>) -> function_ref<f>;
  template<auto f, class T>
    function_ref(nontype_t<f>, T&) -> function_ref<f>;
  template<auto f, class T>
    function_ref(nontype_t<f>, cv T*) -> function_ref<f>;
}
```

> **Deduction guides**
>
> [func.wrap.ref.deduct]

```cpp
template<auto f>
  function_ref(nontype_t<f>) -> function_ref<f>;
template<auto f, class T>
  function_ref(nontype_t<f>, T&) -> function_ref<f>;
template<auto f, class T>
  function_ref(nontype_t<f>, cv T*) -> function_ref<f>;
```
> *Constraints:* `is_function_v<f>` is `true`.

## Feature test macro

We do not need a feature macro, because we intend for this paper to modify `std::function_ref` before it ships.

## Other Languages
C# and the .NET family of languages provide this via `delegates` [^delegates].

```cpp
// C#
delegate void some_name();
some_name fr = then;// the stateless free function use case
some_name fr = localTaxRule.when;// the stateful member function use case
```

Borland C++ now embarcadero provide this via `__closure` [^closure].

```cpp
// Borland C++, embarcadero __closure
void(__closure * fr)();
fr = then;// the stateless free function use case
fr = localTaxRule.when;// the stateful member function use case
```

Since `nontype` `function_ref` handles all 4 stateless/stateful free/member use cases, it is more feature rich than either of the above. Further, these other language implementations require strict matching of the signatures.

## Example implementation

The most up-to-date implementation, created by Zhihao Yuan, is available on GitHub.[^nontype]

## Acknowledgments

Thanks to Arthur O'Dwyer, Tomasz Kamiński, Corentin Jabot and Zhihao Yuan for providing very valuable feedback on this proposal.

## References

[^p0792r6]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0792r6.html>
[^functionref]: <https://github.com/TartanLlama/function_ref>
[^functionrefprime]: <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>
[^nontype]: <https://github.com/zhihaoy/nontype_functional/blob/main/include/std23/function_ref.h>
[^delegates]: <https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/using-delegates>
[^closure]: <http://docwiki.embarcadero.com/RADStudio/Sydney/en/Closure>
[^fastest]: <https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible>
[^impossiblyc]: <https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates>
[^impossiblyd]: <https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11>
[^dyno]: <https://github.com/ldionne/dyno>
[^boostextte]: <https://github.com/boost-ext/te>

