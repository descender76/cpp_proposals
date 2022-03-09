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
struct cat {
    void walk() {
    }
};

void leap(cat& c) {
}

void catwalk(cat& c) {
    c.walk();
}

struct callback {
    cat* c;
    void (*f)(cat&);
};

cat c;
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
callback cb = {&c, [](cat& c){c.walk();}};
// or
callback cb = {&c, catwalk};
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
auto temp = [&c](){c.walk();};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&c](){c.walk();});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<&cat::walk>, c};
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
callback cb = {&c, [](cat& c){leap(c);}};
// or
callback cb = {&c, leap};
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
auto temp = [&c](){leap(c);};
function_ref<void()> fr = temp;
// or when given directly as a function argument
some_function([&c](){leap(c);});
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void()> fr = {nontype<leap>, c}
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
  function_ref<void()> fr = [&c](){c.walk();};// immediately dangling
  // or
  function_ref<void()> fr = [&c](){leap(c);};// immediately dangling
  ```
  
  </td>
  </tr>
  <tr>
  <td>
  proposed
  </td>
  <td>
  
  ```cpp
  function_ref<void()> fr = {nontype<&cat::walk>, c};// DOES NOT DANGLE
  // or
  function_ref<void()> fr = {nontype<leap>, c}// DOES NOT DANGLE
  ```
  
  </td>
  </tr>
  </table>
  While both the original `function_ref` proposal and the proposed addendum perform the same desired task, the former dangles and the later doesn't. It is clear from the immediately dangling `string_view` example that it dangles because `sv` is a reference and `""s` is a temporary. However, it is less clear in the original `function_ref` example. While it is true and clear that `fr` is a reference and the stateful lambda is a temporary. It is not what the user of `function_ref` is intending or wanting to express. `c` is the state that the user wants to type erase and `function_ref` would not dangle if `c` was the state since it is not a temporary and has already been constructed higher up in the call stack. Further, the member function `walk` or the free function `leap` should not dangle since functions are stateless and also global. So member/free function with type erasure use cases are more like `string_view` when the referenced object is safely constructed. 

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
void (cat::*mf)() = &cat::walk;
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
auto temp = &cat::walk;
function_ref<void(cat&)> fr = temp;
// or when given directly as a function argument
some_function(&cat::walk);
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(cat&)> fr = {nontype<&cat::walk>};
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
void (*f)(cat&) = leap;
```

</td>
</tr>
<tr>
<td>
function_ref
</td>
<td>

```cpp
function_ref<void(cat&)> fr = leap;
```

</td>
</tr>
<tr>
<td>
proposed
</td>
<td>

```cpp
function_ref<void(cat&)> fr = {nontype<leap>};
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

The wording is relative to N4901.

Add new templates to 20.2.1 [utility.syn], header `<utility>` synopsis:

```cpp
namespace std {
  [...]

  template<size_t I>
    struct in_place_index_t {
      explicit in_place_index_t() = default;
    };

  template<size_t I> inline constexpr in_place_index_t<I> in_place_index{};

  // nontype argument tag
  template<auto V>
    struct nontype_t {
      explicit nontype_t() = default;
    };

  template<auto V> inline constexpr nontype_t<V> nontype{};
}
```

Add a definition to 20.14.3 [func.def]:

> […]
> 
> A target object is the callable object held by a call wrapper.
> 
> A constant target object is a target object of a structural type [temp.param].
> 
> A call wrapper type may additionally hold […]

Modify 20.14.17.4.3 [func.wrap.ref.ctor] as indicated:

```cpp
template<class... T>
  static constexpr bool is-invocable-using = see below;
```

> If noex is true, `is-invocable-using<T...>` is equal to:
> 
>   `is_nothrow_invocable_r_v<R, T..., ArgTypes...>`
> 
> Otherwise, `is-invocable-using<T...>` is equal to:
> 
>   `is_invocable_r_v<R, T..., ArgTypes...>`

```cpp
template<class VT>
  static constexpr bool is-callable-from = see below;
```

> If noex is true, `is-callable-from<VT>` is equal to:
> 
>   `is_nothrow_invocable_r_v<R, VT cv ref, ArgTypes…> &&
>   is_nothrow_invocable_r_v<R, VT inv-quals, ArgTypes…>`
> 
> Otherwise,  `is-callable-from<VT>` is equal to:
> 
>   `is-invocable-using<VT cv ref> &&
>   is-invocable-using<VT inv-quals>
>   is_invocable_r_v<R, VT cv ref, ArgTypes…> &&
>   is_invocable_r_v<R, VT inv-quals, ArgTypes…>`

```cpp
template<auto f, class T>
  static constexpr bool is-callable-as-if-from = see below;
```

> `is-callable-as-if-from<f, VT>` is equal to:
> 
>   `is-invocable-using<decltype(f), VT cv ref> &&
  is-invocable-using<decltype(f), VT inv-quals>`

```cpp
function_ref(function_ref& f) noexcept;
```

> Postconditions: The target objectstate entities of *this isare the target objectstate entities f had before construction, and f is in a valid state with an unspecified value.

```cpp
template<class F> function_ref(F& f);
```

> Let VT be `decay_t<F>`.
> 
> Constraints:
> 
> - `remove_cvref_t<F>` is not the same type as function_ref, and
> - `remove_cvref_t<F>` is not a specialization of in_place_type_t, and
> - `is-callable-from<F>` is true.
> Mandates: `is_constructible_v<VT, F>` is true.
> 
> Preconditions: VT meets the Cpp17Destructible requirements, and if `is_move_constructible_v<VT>` is true, VT meets the Cpp17MoveConstructible requirements.
> 
> Postconditions: `*this` has no target objectstate entity if any of the following hold:
> 
> - f is a null function pointer value, or
> - f is a null member pointer value, or
> - `remove_cvref_t<F>` is a specialization of the function_ref class template, and f has no target objectstate entity.
> 
> Otherwise, *this has a target object of type VT direct-non-list-initialized with `std::forward<F>(f)`.
> 
> Throws: Any exception thrown by the initialization of the target object. May throw bad_alloc unless VT is a function pointer or a specialization of reference_wrapper.

:::success
```cpp
template<auto f> function_ref(nontype_t<f>) noexcept;
```
:::

:::success
> Constraints: `is-invocable-using<decltype(f)>` is true.
> 
> Postconditions: *this has no state entity if f is a null function pointer value or a null member pointer value. Otherwise, *this has a constant target object. Such an object and f are template-argument-equivalent [temp.type].
:::


:::success
```cpp
template<auto f, class T> function_ref(nontype_t<f>, T&& x);
```
:::

> :::success
> 
> Let VT be `decay_t<T>`.
> 
> Constraints: `is-callable-as-if-from<f, VT>` is true.
> 
> Mandates: `is_constructible_v<VT, T>` is true.
> 
> Preconditions: VT meets the Cpp17Destructible requirements, and if `is_move_constructible_v<VT>` is true, VT meets the Cpp17MoveConstructible requirements.
> 
> Postconditions: `*this` has no state entity if f is a null function pointer value or a null member pointer value. Otherwise, `*this` has the following properties:
> 
> - Its constant target object and f are template-argument-equivalent [temp.type].
> - It has one bound argument entity, an object of type VT direct-non-list-initialized with `std::forward<T>(x)`.
> 
> Throws: Any exception thrown by the initialization of the bound argument. May throw bad_alloc unless
> 
> - VT is a specialization of reference_wrapper, or
> - VT is an object pointer and f is of a pointer to member.
> :::

```cpp
~function_ref();
```

> Effects: Destroys the target objectstate entities of `*this`, if any.

Modify 20.14.17.4.4 [func.wrap.ref.inv] as indicated:

```cpp
R operator()(ArgTypes... args) cv ref noexcept(noex);
```

> *Preconditions*: `*this` has a target object.
> 
> *Effects*: Equivalent to:Let f be an lvalue designating the target object of `*this` and F be the type of f.
> 
> If `*this` has a bound argument, equivalent to
> 
>   `return INVOKE<R>(f, static_cast<T inv-quals>(x), std::forward<ArgTypes>(args)...);`
> 
> where x is an lvalue designating the bound argument of type T.
> 
> Otherwise, if `*this` has a constant target object, equivalent to
> 
>   `return INVOKE<R>(f, std::forward<ArgTypes>(args)...);`
> 
> Otherwise, equivalent to
> 
>   `return INVOKE<R>(static_cast<F inv-quals>(f), std::forward<ArgTypes>(args)...);`
> 
> where f is an lvalue designating the target object of `*this` and F is the type of f.

Modify 20.14.17.4.5 [func.wrap.ref.util] as indicated:

```cpp
void swap(function_ref& other) noexcept;
```

> *Effects*: Exchanges the target objectsstate entities of `*this` and other.

```cpp
friend void swap(function_ref& f1, function_ref& f2) noexcept;
```

> *Effects*: Equivalent to f1.swap(f2).

Add new signatures to [func.wrap.ref.class] synopsis:

> […]

```cpp
  template<class R, class... ArgTypes>
  class function_ref<R(ArgTypes...) cv ref noexcept(noex)> {
  public:
    using result_type = R;

    // [func.wrap.ref.ctor], constructors, assignment, and destructor
    function_ref(function_ref&) noexcept;
    template<class F> function_ref(F&);
    template<auto f> function_ref(nontype_t<f>) noexcept;
    template<auto f, class T> function_ref(nontype_t<f>, T&);

    ~function_ref();

    // [func.wrap.ref.inv], invocation
    R operator()(ArgTypes...) cv ref noexcept(noex);

    // [func.wrap.ref.util], utility
    void swap(function_ref&) noexcept;
    friend void swap(function_ref&, function_ref&) noexcept;

  private:
    template<class... T>
      static constexpr bool is-invocable-using = see below;     // exposition only
    template<class VT>
      static constexpr bool is-callable-from = see below;       // exposition only
    template<auto f, class T>
      static constexpr bool is-callable-as-if-from = see below; // exposition only
  };
}
```

## Feature test macro

We do not need a feature macro, because we intend for this paper to modify std::function_ref before it ships.

## Other Languages
C# and the .NET family of languages provide this via `delegates` [^delegates].

```cpp
// C#
delegate void some_name();
some_name fr = leap;// the stateless free function use case
some_name fr = c.walk;// the stateful member function use case
```

Borland C++ now embarcadero provide this via `__closure` [^closure].

```cpp
// Borland C++, embarcadero __closure
void(__closure * fr)();
fr = leap;// the stateless free function use case
fr = c.walk;// the stateful member function use case
```

Since `nontype function_ref` handles all 4 stateless/stateful free/member use cases, it is more feature rich than either of the above.

## Example implementation

The most up-to-date implementation, created by Zhihao Yuan, is available on `Github` [^nontype]

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

