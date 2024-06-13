<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3326R0</td>
</tr>
<tr>
<td>Date</td>
<td>2024-06-13</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Library Evolution Working Group (LEWG)</td>
</tr>
</table>

# favor ease of use

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

- [favor ease of use](#favor-ease-of-use)
  - [Abstract](#Abstract)
  - [Motivational Example](#Motivational-Example)
  - [Requested Changes](#Requested-Changes)
  - [`favors::safety` vs `favors::ease` usage](#favorssafety-vs-favorsease-usage)
  - [Should've, Could've, Would've](#Shouldve-Couldve-Wouldve)
  - [Summary](#Summary)
  - [References](#References)

## Abstract

`optional<&>` [^p2988r5] is a spectacularly `safe by default` type. With only small additions to both `optional<T>` and `optional<T&>` [^p2988r5], library writers will be able to make it even easier to use by allowing temporaries to be used in safe scenarios.

## Motivational Example

### p2988r5 [^p2988r5]

#### given

```cpp
optional<int&> dangler(optional<const int&> other,
                       optional<int&> left,
                       optional<int&> right)
{
    if(random_bool())
    {
        return left;
    }
    else
    {
        return right;
    }
}
```

#### usage

```cpp
int i = 42;
optional<int&> oi1{i};
optional<int&> oi2 = dangler(oi1, oi1, oi1);
optional<int&> oi3 = dangler(42/* unnecessary error */, oi1, oi1);
```

### this proposal

#### given

```cpp
// just added ", favors::ease" to the first parameter
// since 'other' nor any component of 'other' is returned
// in other words, the return is only dependent upon left and right
optional<int&> dangler(optional<const int&, favors::ease> other,
                       optional<int&> left,
                       optional<int&> right)
{
    if(random_bool())
    {
        return left;
    }
    else
    {
        return right;
    }
}
```

#### usage

```cpp
int i = 42;
optional<int&> oi1{i};
optional<int&> oi2 = dangler(oi1, oi1, oi1);
optional<int&> oi3 = dangler(42/* ok */, oi1, oi1);
```

## Requested Changes

1. Add the `favors` enumeration

```cpp
enum class favors
{
    safety,
    ease
};
```

2. Add the default value of `favors::safety` to the original `optional` template.

```cpp
template <class T, favors favor = favors::safety>
class optional {
};
```

3. Add the `favor` parameter to the `std::optional<&>` [^p2988r5] specialization.
1. Revise the `std::optional<&>` [^p2988r5] constructor by requiring `favors::safety`.
1. Add a constructor that takes a lvalue reference and requiring `favors::ease`.

```cpp
template <class T, favors favor/* = favors::safety*/>
class /*std::*/optional<T&, favor> {

// ...

        template <class U = T>
            requires(!detail::is_optional<std::decay_t<U>>::value)
        constexpr explicit(!std::is_convertible_v<U, T>) optional(U&& u) noexcept requires (favor == favors::safety)
            : value_(std::addressof(u)) {
            static_assert(
                std::is_constructible_v<std::add_lvalue_reference_t<T>, U>
                & favor == favors::safety,
                "Must be able to bind U to T&");
            static_assert(std::is_lvalue_reference<U>::value
                & favor == favors::safety,
                "U must be an lvalue");
        }

        constexpr optional(T& t) noexcept requires (favor == favors::ease)
            : value_{std::addressof(t)} {}

// ...

}
```

## `favors::safety` vs `favors::ease` usage

### favors::safety (i.e. the default)

* local variables
* return parameters just like `p2748r5` [^p2748r5]
* member variables that are dependent upon constructor arguments
* input parameters that will be returned in whole or in part

### favors::ease

* input parameters of functions that return void
* input parameters of functions that return values
* input parameters that will NOT be returned in whole or in part

##  Should've, Could've, Would've

The proposed functionality could be of benefit in other pure reference types. While that is out of scope of this proposal, it is worth mentioning the current impediments.

### span

Span is not as `safe by default` as `std::optional<&>` [^p2988r5] since it allows binding to a temporary by default.

```cpp
std::string_view sv1 = "42"s;// should be error
```

### function_ref

It can be difficult to implement as parameter packs and default parameter values both live at the end of the template parameter list. 

### reference_wrapper

`reference_wrapper` is the best existing pure reference type that could benefit from this enhancement. Its safety is comparable to that of `std::optional<&>` [^p2988r5]. `reference_wrapper` would need to be reimplemented and respecified before enhancing. Doing such would result in a safer replacement for `&` itself.

## Summary

This proposal identifies with Arthur O’Dwyer's article `Value category is not lifetime` [^value-category-is-not-lifetime]. While `safety by default` is paramount, requiring that all libraries that use `std::optional<&>` [^p2988r5] be needlessly difficult to use is detrimental to the clarity of our programs.

## References
<!--std::optional<T&>-->
[^p2988r5]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2988r5.pdf>
<!--Value category is not lifetime-->
[^value-category-is-not-lifetime]: <https://quuxplusone.github.io/blog/2019/03/11/value-category-is-not-lifetime/>
<!--Disallow Binding a Returned Glvalue to a Temporary-->
[^p2748r5]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2748r5.html>
