<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2657R1</td>
</tr>
<tr>
<td>Date</td>
<td>2022-11-14</td>
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

# C++ is the next C++

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

- [C++ is the next C++](#c-is-the-next-c)
  - [Changelog](#changelog)
  - [Abstract](#abstract)
  - [Motivating examples](#motivating-examples)
  - [Tooling Opportunities](#tooling-opportunities)
  - [Reserved Behavior](#reserved-behavior)
  - [Summary](#summary)
  - [Frequently Asked Questions](#frequently-asked-questions)
  - [Acknowledgments](#acknowledgments)
  - [References](#references)

## Changelog

### R1

- aggregate multiple instances of the `static_analysis` attribute into the `inclusions` and `exclusions` members of the attribute
- added `std::const_pointer_cast` and `std::reinterpret_pointer_cast` to the [No unsafe casts](#No-unsafe-casts) section
- added the [Use ranges](#Use-ranges) section
- [use_function_ref](#Use-stdfunction_ref) is a subset of `safer` instead of `modern` because it reduces void* usage
- added the [Tooling Opportunities](#Tooling-Opportunities) section
- added the [Reserved Behavior](#Reserved-Behavior) section
- removed the `How do we configure future analyzers`<!--[](#How-do-we-configure-future-analyzers)--> section from the [Frequently Asked Questions](#Frequently-Asked-Questions) as it is impossible to configure individually on the aggregate/grouped analyzers `safer` and `modern`, also consolidating using the attribute multiple times into one attribute
- added the [How does this relate to p2687r0: Design Alternatives for Type-and-Resource Safe C++?](#How-does-this-relate-to-p2687r0-Design-Alternatives-for-Type-and-Resource-Safe-C) section to the [Frequently Asked Questions](#Frequently-Asked-Questions) section
- added the [Acknowledgments](#Acknowledgments) section

## Abstract

Programmer's, Businesses and Government(s) want C++ to be safer and simpler. This has led some `C++` programmers to create new programming languages or preprocessors, which again is a new language. This paper discusses using static analysis to make the `C++` language itself safer and simpler.

## Motivating Examples

Following is a wishlist. Most are optional. While, they all would be of benefit. It all starts with a new <!--repeatable--> module level attribute that would preferably be applied once in the `primary module interface unit` and would automatically apply to it and all `module implementation unit`(s). It could also be applied to a `module implementation unit` but that would generally be less useful. However, it might aid in *gradual migration*.

```cpp
export module some_module_name [[static_analysis(inclusions{"", "", ""}, exclusions{"", "", ""})]];// primary module interface unit
// or
module some_module_name [[static_analysis(inclusions{"", "", ""}, exclusions{"", "", ""})]];// module implementation unit
```

- It would be ideal if the `inclusions` and `exclusions` members of the `static_analysis` attribute could be passed as either an environment variable and/or command line argument to compilers so it could be used by pipelines to assert the degree of conformance to the defined static analyzer without actually changing the source.
- It would be ideal if compilers could standardize the environment variable name or command line argument name in order to ease tooling.
- It would be ideal if compilers could produce a machine readable report in JSON, YAML or something else so that pipelines could more easily consume the results.
- It would be ideal if compilers could standardize the machine readable report.

The names of the `inclusions` and `exclusions` are dotted. Unscoped or names that start with `std.`, `c++.`, `cpp.`, `cxx.` or `c.` are reserved for standardization.

This proposal wants to stardardize two overarching static analyzer names; `safer` and `modern`.

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"safer"})]]
```

</td>
<td>

The `safer` analyzer is for safety, primarily memory related. It is for those businesses and programmers who must conform to safety standards. The `safer` analyzer is a subset of `modern` analyzer.

</td>
</tr>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"modern"})]]
```

</td>
<td>

The `modern` analyzer goes beyond just memory and safety concerns. It can be thought of as bleeding edge. It is for those businesses and programmers who commit to safety and higher quality modern code. That is those who want to enjoy the full benefits that C++ has to offer.

</td>
</tr>
</table>

Neither is concerned about formatting or nitpicking. Both static analyzers only produce errors. These are meant for programmers, businesses and governments in which safety takes precedence. They both represent +&infin;; an ever increasing commitment. When a new version of the standard is released and adds new sub static analyzers than everyone's code is broken, until their code is fixed. These sub static analyzers usually consist of features that have been mostly replaced with some other feature. It would be ideal if the errors produced not only say that the code is wrong but also provide a link to html page(s) maintained by the `C++` teaching group, the authors of the `C++ Core Guidelines` [^cppcg] and compiler specific errors. These pages should provide example(s) of what is being replaced and by what was it replaced. Mentioning the version of the `C++` standard would also be helpful.

All modules can be used even if they don't use the `static_analysis` attribute as this allows *gradual adoption*.

The resolved list of static analyzers that will run is derived by first taking the union of all of the included analyzers including their component analyzers and removing from that union the union of all the excluded ananlyzers and their component analyzers.

*resolved = (inclusion &cup; inclusion &cup; inclusion) - (exclusion &cup; exclusion &cup; exclusion)*

This allows the programmer to exclude a few analyzers from a larger grouping instead of having to list out almost all the ever growing number of smaller analyzers that comprise the larger ones.

```cpp
[[static_analysis(inclusions{"modern"}, exclusions{"use_ranges"})]];
```

### What are the `safer` and `modern` analyzers composed of?

These overarching static analyzers are composed of multiple static analyzers which can be used individually to allow a degree of *gradual adoption*.

#### Use lvalue references

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"use_lvalue_references"})]]
```

</td>
<td>

`use_lvalue_references` is a subset of `safer`.

</td>
</tr>
</table>

- Any declaration of a pointer is an error.
- Calling any function that has parameters that take a pointer is an error unless the pointer type are "pointer to `const` character type" or "`const` pointer to `const` character type" and their arguments were string literals.
  - string literals are always safe having static storage duration
  - `std::string` and `std::string_view` must be creatable at compile time
- Function pointers and member function pointers can still be used.
  - Function pointers and member function pointers that declare pointers to non [member] function pointers produces an error.
- Lvalue references, &, can still be used.
<!--
Using the keyword `nullptr` is an error because there are no pointers. What about function pointers = nullptr
-->

**WHY?**

- A large portion of the C++ community have been programming without pointers for years. Some can go their whole career this way. This proposal just standardise existing practice.
- Modern `C++` has been advocated to programmers in other programming languages who complain about memory issues. This allows us to show them what we have been saying for decades.
- Over half of our memory related issues gets hashed away.
- Pointers have largely been replaced with the following:

<table>
<tr>
<td>

lvalue references

</td>
<td>

1985: Cfront 1.0 [^history]

</td>
</tr>
<tr>
<td>

STL

</td>
<td>

1992 [^history]

</td>
</tr>
<tr>
<td>

`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`, `std::reference_wrapper`, `std::make_shared`

</td>
<td>

C++11

</td>
</tr>
<tr>
<td>

`std::make_unique`

</td>
<td>

C++14

</td>
</tr>
<tr>
<td>

`std::string_view`, `std::optional`, `std::any`, `std::variant`

</td>
<td>

C++17

</td>
</tr>
<tr>
<td>

`std::make_shared` support arrays, `std::span`

</td>
<td>

C++20

</td>
</tr>
</table>

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.

- `P.4: Ideally, a program should be statically type safe` [^cppcgp4]
- `P.6: What cannot be checked at compile time should be checkable at run time` [^cppcgp6]
- `P.7: Catch run-time errors early` [^cppcgp7]
- `P.8: Don’t leak any resources` [^cppcgp8]
- `P.11: Encapsulate messy constructs, rather than spreading through the code` [^cppcgp11]
- `P.12: Use supporting tools as appropriate` [^cppcgp12]
- `P.13: Use support libraries as appropriate` [^cppcgp13]
- `I.4: Make interfaces precisely and strongly typed` [^cppcgi4]
- `I.11: Never transfer ownership by a raw pointer (T*) or reference (T&)` [^cppcgi11]
- `I.12: Declare a pointer that must not be null as not_null` [^cppcgi12]
- `I.13: Do not pass an array as a single pointer` [^cppcgi13]
- `I.23: Keep the number of function arguments low` [^cppcgi23]
- `F.7: For general use, take T* or T& arguments rather than smart pointers` [^cppcgf7]
- `F.15: Prefer simple and conventional ways of passing information` [^cppcgf15]
- `F.22: Use T* or owner<T*> to designate a single object` [^cppcgf22]
- `F.23: Use a not_null<T> to indicate that “null” is not a valid value` [^cppcgf23]
- `F.25: Use a zstring or a not_null<zstring> to designate a C-style string` [^cppcgf25]
- `F.26: Use a unique_ptr<T> to transfer ownership where a pointer is needed` [^cppcgf26]
- `F.27: Use a shared_ptr<T> to share ownership` [^cppcgf27]
- `F.42: Return a T* to indicate a position (only)` [^cppcgf42]
- `F.43: Never (directly or indirectly) return a pointer or a reference to a local object` [^cppcgf43]
- `C.31: All resources acquired by a class must be released by the class’s destructor` [^cppcgc31]
- `C.32: If a class has a raw pointer (T*) or reference (T&), consider whether it might be owning` [^cppcgc32]
- `C.33: If a class has an owning pointer member, define a destructor` [^cppcgc33]
- `C.149: Use unique_ptr or shared_ptr to avoid forgetting to delete objects created using new` [^cppcgc149]
- `C.150: Use make_unique() to construct objects owned by unique_ptrs` [^cppcgc150]
- `C.151: Use make_shared() to construct objects owned by shared_ptrs` [^cppcgc151]
- `R.1: Manage resources automatically using resource handles and RAII (Resource Acquisition Is Initialization)` [^cppcgr1]
- `R.2: In interfaces, use raw pointers to denote individual objects (only)` [^cppcgr2]
- `R.3: A raw pointer (a T*) is non-owning` [^cppcgr3]
- `R.5: Prefer scoped objects, don’t heap-allocate unnecessarily` [^cppcgr5]
- `R.10: Avoid malloc() and free()` [^cppcgr10]
- `R.11: Avoid calling new and delete explicitly` [^cppcgr11]
- `R.12: Immediately give the result of an explicit resource allocation to a manager object` [^cppcgr12]
- `R.13: Perform at most one explicit resource allocation in a single expression statement` [^cppcgr13]
- `R.14: Avoid [] parameters, prefer span` [^cppcgr14]
- `R.15: Always overload matched allocation/deallocation pairs` [^cppcgr15]
- `R.20: Use unique_ptr or shared_ptr to represent ownership` [^cppcgr20]
- `R.22: Use make_shared() to make shared_ptrs` [^cppcgr22]
- `R.23: Use make_unique() to make unique_ptrs` [^cppcgr23]
- `ES.20: Always initialize an object` [^cppcges20]
- `ES.24: Use a unique_ptr<T> to hold pointers` [^cppcges24]
- `ES.42: Keep use of pointers simple and straightforward` [^cppcges42]
- `ES.47: Use nullptr rather than 0 or NULL` [^cppcges47]
- `ES.60: Avoid new and delete outside resource management functions` [^cppcges60]
- `ES.61: Delete arrays using delete[] and non-arrays using delete` [^cppcges61]
- `ES.65: Don’t dereference an invalid pointer` [^cppcges65]
- `E.13: Never throw while being the direct owner of an object` [^cppcge13]
- `CPL.1: Prefer C++ to C` [^cppcgcpl1]

**Gotchas**

**Usage of smart pointers**

This static analyzer causes programmers to use 2 extra characters when using smart pointers, `->` vs `(*).`, since the overloaded `->` operator returns a pointer.

<table>
<tr>
<td>

```cpp
smart_pointer->some_function();// bad
```

</td>
<td>

```cpp
(*smart_pointer).some_function();// good
```

</td>
</tr>
</table>

**the main function and environment variables**

A shim module is needed in order to transform main and env functions into a more C++ friendly functions. These have been asked for years.
1. `A Modern C++ Signature for main` [^modernmain]
1. `Desert Sessions: Improving hostile environment interactions` [^hostileenv]

---

#### No unsafe casts

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_unsafe_casts"})]]
```

</td>
<td>

`no_unsafe_casts` is a subset of `safer`.

</td>
</tr>
</table>

- Using `C`/core cast produces an error.
- Using `reinterpret_cast` produces an error.
- Using `const_cast` produces an error.
- Using `std::reinterpret_pointer_cast` produces an error.
- Using `std::const_pointer_cast` produces an error.

Why?

- `C`/core cast was replaced by `static_cast` and `dynamic_cast`.
- The `reinterpret_cast` is needed more for library authors than their users. For library users it usually just causes problems and questions. It is rarely used in daily `C++` when coding at a higher level.
- The `const_cast` is needed more for library authors than their users. It is a means for the programmer to lie to oneself. For library users it usually just causes problems and questions. It is rarely used in daily `C++` when coding at a higher level.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `C.146: Use dynamic_cast where class hierarchy navigation is unavoidable` [^cppcgc146]
- `ES.48: Avoid casts` [^cppcges48]
- `ES.49: If you must use a cast, use a named cast` [^cppcges49]
- `ES.50: Don’t cast away const` [^cppcges50]

---

#### No unions

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_union"})]]
```

</td>
<td>

`no_union` is a subset of `safer`.

</td>
</tr>
</table>

- Using the `union` keyword produces an error.

It was replaced by `std::variant`, which is safer.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `C.181: Avoid “naked” unions` [^cppcgc181]

---

#### No mutable

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_mutable"})]]
```

</td>
<td>

`no_mutable` is a subset of `safer`.

</td>
</tr>
</table>

- Using the `mutable` keyword produces an error.

The programmer shall not lie to oneself. The `mutable` keyword violates the safety of `const` and is rarely used at a high level.

---

#### No new or delete

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_new_delete"})]]
```

</td>
<td>

`no_new_delete` is a subset of `safer`.

</td>
</tr>
</table>

- Using the `new` and `delete` keywords to allocate and deallocate memory produces an error.

It was replaced by `std::make_unique` and `std::make_shared`, which are safer.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `F.26: Use a unique_ptr<T> to transfer ownership where a pointer is needed` [^cppcgf26]
- `F.27: Use a shared_ptr<T> to share ownership` [^cppcgf27]
- `C.149: Use unique_ptr or shared_ptr to avoid forgetting to delete objects created using new` [^cppcgc149]
- `C.150: Use make_unique() to construct objects owned by unique_ptrs` [^cppcgc150]
- `C.151: Use make_shared() to construct objects owned by shared_ptrs` [^cppcgc151]
- `R.11: Avoid calling new and delete explicitly` [^cppcgr11]
- `R.20: Use unique_ptr or shared_ptr to represent ownership` [^cppcgr20]
- `R.22: Use make_shared() to make shared_ptrs` [^cppcgr22]
- `R.23: Use make_unique() to make unique_ptrs` [^cppcgr23]
- `ES.60: Avoid new and delete outside resource management functions` [^cppcges60]
- `ES.61: Delete arrays using delete[] and non-arrays using delete` [^cppcges61]

---

#### No volatile

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_volatile"})]]
```

</td>
<td>

`no_volatile` is a subset of `safer`.

</td>
</tr>
</table>

- Using the `volatile` keyword produces an error.

The `volatile` keyword has nothing to do with concurrency. Use `std::atomic` or `std::mutex` instead.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `CP.8: Don’t try to use volatile for synchronization` [^cppcgcp8]

---

#### No `C` style variadic functions

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_c_style_variadic_functions"})]]
```

</td>
<td>

`no_c_style_variadic_functions` is a subset of `safer`.

</td>
</tr>
</table>

- Declaring a `C` style variadic function produces an error.
- Calling a `C` style variadic function produces an error.
- Using the `va_start`, `va_arg`, `va_copy`, `va_end` or `va_list` functions produces errors.

`C` style variadic functions has been replaced by overloading, templates and variadic template functions.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `F.55: Don’t use va_arg arguments` [^cppcgf55]
- `ES.34: Don’t define a (C-style) variadic function` [^cppcges34]

---

#### No deprecated

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_deprecated"})]]
```

</td>
<td>

`no_deprecated` is a subset of `modern`.

</td>
</tr>
</table>

- Using anything that has the deprecated attribute on it produces an error.

Deprecated functionality is not modern.

---

#### Use `std::array`

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"use_std_array"})]]
```

</td>
<td>

`use_std_array` is a subset of `modern`.

</td>
</tr>
</table>

- Declaring a `C` style/core `C++` array variable, whether locally or in a class, produces an error.
- It is okay to use array literals when initializing `std::array` and other collections.

Use `std::array` instead of `C` style/core `C++` array.

---

#### Use ranges

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"use_ranges"})]]
```

</td>
<td>

`use_ranges` is a subset of `modern`.

</td>
</tr>
</table>

Using any iterator based algorithm that has been replaced with a range based algorithm produces an error informing the programmer to use the range based algorithm instead.

- Using `std::all_of` produces an error.
- Using `std::any_of` produces an error.
- Using `std::none_of` produces an error.
- Using `std::for_each` produces an error.
- Using `std::for_each_n` produces an error.
- Using `std::count` produces an error.
- Using `std::count_if` produces an error.
- Using `std::mismatch` produces an error.
- Using `std::find` produces an error.
- Using `std::find_if` produces an error.
- Using `std::find_if_not` produces an error.
- Using `std::find_end` produces an error.
- Using `std::find_first_of` produces an error.
- Using `std::adjacent_find` produces an error.
- Using `std::search` produces an error.
- Using `std::search_n` produces an error.
- Using `std::copy` produces an error.
- Using `std::copy_if` produces an error.
- Using `std::copy_n` produces an error.
- Using `std::copy_backward` produces an error.
- Using `std::move` produces an error.
- Using `std::move_backward` produces an error.
- Using `std::fill` produces an error.
- Using `std::fill_n` produces an error.
- Using `std::transform` produces an error.
- Using `std::generate` produces an error.
- Using `std::generate_n` produces an error.
- Using `std::remove` produces an error.
- Using `std::remove_if` produces an error.
- Using `std::remove_copy` produces an error.
- Using `std::remove_copy_if` produces an error.
- Using `std::replace` produces an error.
- Using `std::replace_if` produces an error.
- Using `std::replace_copy` produces an error.
- Using `std::replace_copy_if` produces an error.
- Using `std::swap_ranges` produces an error.
- Using `std::reverse` produces an error.
- Using `std::reverse_copy` produces an error.
- Using `std::rotate` produces an error.
- Using `std::rotate_copy` produces an error.
- Using `std::shift_left` produces an error.
- Using `std::shift_right` produces an error.
- Using `std::shuffle` produces an error.
- Using `std::unique` produces an error.
- Using `std::unique_copy` produces an error.
- Using `std::is_partitioned` produces an error.
- Using `std::partition` produces an error.
- Using `std::partition_copy` produces an error.
- Using `std::stable_partition` produces an error.
- Using `std::partition_point` produces an error.
- Using `std::is_sorted` produces an error.
- Using `std::is_sorted_until` produces an error.
- Using `std::sort` produces an error.
- Using `std::partial_sort` produces an error.
- Using `std::partial_sort_copy` produces an error.
- Using `std::stable_sort` produces an error.
- Using `std::nth_element` produces an error.
- Using `std::lower_bound` produces an error.
- Using `std::upper_bound` produces an error.
- Using `std::binary_search` produces an error.
- Using `std::equal_range` produces an error.
- Using `std::merge` produces an error.
- Using `std::includes` produces an error.
- Using `std::set_difference` produces an error.
- Using `std::set_intersection` produces an error.
- Using `std::set_symmetri_difference` produces an error.
- Using `std::set_union` produces an error.
- Using `std::is_heap` produces an error.
- Using `std::is_heap_until` produces an error.
- Using `std::make_heap` produces an error.
- Using `std::push_heap` produces an error.
- Using `std::pop_heap` produces an error.
- Using `std::sort_heap` produces an error.
- Using `std::max` produces an error.
- Using `std::max_element` produces an error.
- Using `std::min` produces an error.
- Using `std::min_element` produces an error.
- Using `std::minmax` produces an error.
- Using `std::minmax_element` produces an error.
- Using `std::clamp` produces an error.
- Using `std::equal` produces an error.
- Using `std::lexicographical_compare` produces an error.
- Using `std::is_permutation` produces an error.
- Using `std::next_permutation` produces an error.
- Using `std::prev_permutation` produces an error.
- Using `std::iota` produces an error.
- Using `std::uninitialized_copy` produces an error.
- Using `std::uninitialized_copy_n` produces an error.
- Using `std::uninitialized_fill` produces an error.
- Using `std::uninitialized_fill_n` produces an error.
- Using `std::uninitialized_move` produces an error.
- Using `std::uninitialized_move_n` produces an error.
- Using `std::uninitialized_default_construct` produces an error.
- Using `std::uninitialized_default_construct_n` produces an error.
- Using `std::uninitialized_value_construct` produces an error.
- Using `std::uninitialized_value_construct_n` produces an error.
- Using `std::destroy` produces an error.
- Using `std::destroy_n` produces an error.
- Using `std::destroy_at` produces an error.
- Using `std::construct_at` produces an error.
<!--
WHY?

The iterator based algorithms have been replaced by the range based algorithms.
-->
### What may `safer` and `modern` analyzers be composed of in the future?

#### No include

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_include"})]]
```

</td>
<td>

`no_include` is a subset of `modern`.

</td>
</tr>
</table>

The preprocessor directive `#include` has been replaced with `import`.
Don't add the static analyzer until `#embed` is added.

<span style="color:red">**NOTE**: This may be impossible to implement as preprocessing occurs before compilation.</span>

---

#### No goto

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"no_goto"})]]
```

</td>
<td>

`no_goto` is a subset of `modern`.

</td>
</tr>
</table>

- Using the `goto` keyword produces an error.

Don't add until `break` and `continue` to a label is added. Also a really easy to use finite state machine library may be needed.

The `C++ Core Guidelines` [^cppcg] identifies issues that this feature helps to mitigate.
- `ES.76: Avoid goto` [^cppcges76]

---

#### Use `std::function_ref`

<table>
<tr>
<td>

```cpp
[[static_analysis(inclusions{"use_function_ref"})]]
```

</td>
<td>

`use_function_ref` is a subset of `safer`.

</td>
</tr>
</table>

- Declaring a `C` style/core `C++` function pointer, whether locally or in a class, produces an error.
- Declaring a `C` style/core `C++` member function pointer, whether locally or in a class, produces an error.
- It is okay to use [member] function pointer literals when initializing `std::function_ref` and others.

Use `std::function_ref` instead of `C` style/core `C++` [member] function pointers. `std::function_ref` can bind to stateful and stateless, free and member functions. It saves programmers from having to include a `void*` state parameter in their function pointer types and it also saves from having to include `void*` state parameter along side the function pointer type in each function where the function pointer type is used in function declarations. Neither of which could be performed with the `"use_lvalue_references"` static analyzer.

**NOTE:**
- This can't be performed until `nontype_t` [^morefunctional] `std::function_ref` [^functionref] gets standardized.

---

## Tooling Opportunities

### Automated Code Reviews

In the [Motivating Examples](#Motivating-Examples) section there were two specific wishlist items.

- *It would be ideal if compilers could produce a machine readable report in JSON, YAML or something else so that pipelines could more easily consume the results.*
- *It would be ideal if compilers could standardize the machine readable report.*

With these capabilities, a report could be created during a distributed version control system's pull/merge request. The report could be compared to the report of the destination of the request. If the changed code is not better than the existing code than the request can be automatically rejected. This would result in an adaptation of the boy scout rule.

*Leave the code cleaner than you found it.*

Consequently, this results in the creation of a programmer incline. With each checkin, the code gets better. The incline can even be adjusted by requiring how much better one must leave the code.

## Reserved Behavior

The `static_analysis` attribute can only, for now, be used on either `primary module interface unit` or `module implementation unit` but not both at the same time. Enabling it in both would require a discussion of how these analyzers should combine. Which one would take precedence? It would need to be part of a larger discussion of whether the `static_analysis` attribute could be applied at the namespace, class, function or control block levels. This proposal is focused on the module level as current static analyzers on the market is more focused on the translation unit level rather than on a per line basis. As such, this proposal could be adopted faster, yet, leaving room for improvements, once static analyzers improve in their precision.

## Summary

By adding static analysis to the `C++` language we can make the language safer and easier to teach because we can restrict how much of the language we use. Human readable errors and references turns the compiler into a teacher freeing human teachers to focus on what the compiler doesn't handle.

## Frequently Asked Questions

### Shouldn't these be warnings instead of errors?

NO, otherwise we'll be stuck with what we just have. `C++` compilers produces plenty of warnings. `C++` static analyzers produces plenty of warnings. However, when some one talks about creating a new language, then old language syntax becomes invalid i.e. errors. This is for programmers. Programmers and businesses rarely upgrade their code unless they are forced to. Businesses and Government(s) want errors, as well, in order to ensure code quality and the assurance that bad code doesn't exist anywhere in the module. This is also important from a language standpoint because we are essentially pruning; somewhat. Keep in mind that all of these pruned features still have use now. In the future, more constructs will be built upon these pruned features. This is why they need to be part of the language, just not a part of everyday usage of the language.

### Why at the module level? Why not safe and unsafe blocks?

Programmers and businesses rarely upgrade their code unless they are forced to. New programmers need training wheels and some of us older programmers like them too. Due to the proliferation of government regulations and oversight, businesses have acquired `software composition analysis` services and tools. These services map security errors to specific versions of modules; specifically programming artifacts such as executables and libraries. As such, businesses want to know if a module is reasonably safe.
<!--
### How do we configure future analyzers?

Any arguments provided after the name of the analyzer can be forwarded onto the analyzer.

```cpp
[[static_analysis("some_future_analyzer", 1, true, 0.5, "Hello World")]]
```

In this case, `1, true, 0.5, "Hello World"` would all be forwarded to the static analyzer "some_future_analyzer". None of the current analyzers use this functionality so this just illustrates distant future work where we can define these analyzers in a standard fashion but that can't happen until we have a code DOM. As such, how these arguments are forwarded are currently compiler specific.
-->
### You must really hate pointers?

Actually, I love `C`, `C++` and pointers.

- I recognize that most of the time, when I code, that I don't need them.
- I recognize that **past** fundamental `C++` libraries use pointers but the users of those libraries don't need them.
- I recognize that **present** fundamental libraries such `function_ref` uses `void*` for type erasure but the users of `function_ref`, most of the time, won't need it.
- I recognize that **future** fundamental libraries such as dynamic polymorphic traits also need pointers for type erasure but they don't expect their users to fidget with raw pointers.
- I also recognize that 1 programmer writes a library but hundreds use the library without needing the same parts of C++ used in its creation.
- Pointers are simple and easy for memory mapped hardware but many C++ programmers don't operate at this level.
- A few will create an OS [driver] but thousands will use it.

The fact is pointers, unsafe casts, `union`, `mutable` and `goto` are the engine of C++ change. As such it would be foolish to remove them but it is also unrealistic for users/drivers of a vehicle to have to drive with nothing between them and the engine, without listening to them clamor for interior finishing.

### C++ can't standardize specific static analyzers

- Can't `C++` provide the `static_analysis` attribute so that static analyzers can be called?
- Can't `C++` reserve unscoped or names that start with `std.`, `c++.`, `cpp.`, `cxx.` or `c.` are for future standardization?
- Can't `C++` reserve the names of static analyzers in the reserved `C++` static analyzer namespace?
- Can't `C++` **recommend** these reserved static analyzers and leave it to the compiler writers to appease their users that clamor for them?

### Do you fear that this could create a "subset of C++" that "could split the user community and cause acrimony"? [^bsfaqsubset]

First of all, let's consider the quotes of Bjarne Stroustrup that this question are based upon.

<table>
<tr>
<td>

*"being defined by an 'industry consortium.' I am not in favor of language subsets or dialects. I am especially not fond of subsets that cannot support the standard library so that the users of that subset must invent their own incompatible foundation libraries. I fear that a defined subset of C++ could split the user community and cause acrimony"* [^bsfaqsubset]

</td>
</tr>
</table>

Does this paper create a subset? YES. Like it or not `C++` already have a couple of subsets; some positive, some quasi. `Freestanding` is a subset for low level programming. This proposal primarily focus on high level programming but there is nothing preventing the creation of `[[static_analysis(inclusions{"freestanding"})]]` which enforces `freestanding`. The `C++` value categories has to some degree fractured the community into a clergy class that thoroughly understand its intracacies and a leity class that gleefully uses it.

Does this paper split the user community? YES and NO. It splits code into safer vs. less safe, high level vs. low level. However, this is performed at the module level, allowing the same programmer to decide what falls on either side of the fence. This would not be performed by an industry consortium but rather the standard. Safer modules can be used by less safe modules. Less safe modules can partly be used by safer modules, such as with the standard module. This latter impact is already minimalized because the standard frequently write their library code in `C++` fashion instead of a `C` fashion.

---

<table>
<tr>
<td>

***"Are there any features you'd like to remove from C++?"*** [^bsfaqremovefeature]

*Not really. People who ask this kind of question usually think of one of the major features such as multiple inheritance, exceptions, templates, or run-time type identification. C++ would be incomplete without those. I have reviewed their design over the years, and together with the standards committee I have improved some of their details, but none could be removed without doing damage.* [^bsfaqremovefeature]

*Most of the features I dislike from a language-design perspective (e.g., the declarator syntax and array decay) are part of the C subset of C++ and couldn't be removed without doing harm to programmers working under real-world conditions. C++'s C compatibility was a key language design decision rather than a marketing gimmick. Compatibility has been difficult to achieve and maintain, but real benefits to real programmers resulted, and still result today. By now, C++ has features that allow a programmer to refrain from using the most troublesome C features. For example, standard library containers such as vector, list, map, and string can be used to avoid most tricky low-level pointer manipulation.* [^bsfaqremovefeature]

</td>
</tr>
</table>

The beauty of this proposal is it does not and it does remove features from C++. Like the standard library, it allows programmers to refrain from using the most troublesome `C` and `C++` features.

---

<table>
<tr>
<td>

"Within C++, there is a much smaller and cleaner language struggling to get out" [^bsq]

</td>
</tr>
</table>

Both making things smaller and cleaner requires removing something. When creating a new language, removing things happens extensively at the beginning but, frequently, features have to be added back in, when programmers clamor for them. This paper cleans up a programmers use of the `C++` language, meaning less `C++` has to be taught immediately, thus making things simpler. As a programmer matures, features can be gradually added to their repertoire, just as it was added to ours. After all, isn't `C++` larger now, than when we started programming in `C++`.

### How does this relate to p2687r0: Design Alternatives for Type-and-Resource Safe C++?

This proposal and the "Design Alternatives for Type-and-Resource Safe C++" [^p2687r0] proposal both recommend that static analysis be used and brought into the language instead of inventing a whole new language. Both tackles problems in its own way. Either proposal could be enhanced to do what the other proposal does. The question is what are these differences and should these be given some attention.

#### Different audiences

This proposal might appeal more to non voting, newer programmers working on smaller, newer code bases. The `p2687r0` proposal appeals more to voting, older programmers working on larger, older code bases.
There are also differences in the sizes of these two audiences. This proposal would have the larger audience as it appeals to those who want a subset of language and library features. There are also differences in the level of coding. This proposal favors high level, abstraction heavy coding. The `p2687r0` proposal appeals more to lower level, closer to hardware coding. Again both proposals fixes safety issues and either audience just wants more safety, **sooner, rather than later**.

Are there any elements of this proposal that would still appeal to lower level coders? New code does get developed in older code bases. The question is do you want programmers to keep writing their code the old way for the sake of a foolish consistency! So this proposal is of use to lower level programmers. With the `p2687r0` proposal, a lot of time is spent analyzing and documenting with attributes the intention of pointers at each point of use in the code. No rewrite is being performed and more information is being provided to resolve ambiguity for the benefit of the static analyzer. The cost of this programmer analysis and attributing can be most of the cost of a rewrite, so why not just rewrite it incrementally in `safer` `modern` `C++`! This proposal helps even with this. From my experience with lower level code, I tend to have a few files of the majority that does the work with memory mapped files or that call `C` API's but once I have my wrappers, the remainer of my code is very high level and abstract. So in this regard, this proposal is of benefit. `C++20` modules are still `new` even to existing code bases especially since tool chains are still being developed and their are still many unanswered questions. Since there will need to be incremental refactoring to use modules in older code bases, why not take advantage of this proposal's module level attribute to take advantage of more refactoring.

#### Different scopes

This proposal has fewer features than `p2687r0`. For instance, it currently only works at the module level. This is similar to where many static analyzers run, at the translation unit level. While this proposal has far fewer features, it is smaller, simpler and easier to implement. This could mean the difference in getting some subset of safety in the `C++26/C++29` timeframe instead of the `C++29/C++32` timeframe. The additional features could be added incrementally.

#### Different solutions

The `p2687r0` proposal tackles problems head on. Bravo! This proposal is about avoiding problems, all together, by using existing language and library features, that we have had for years, if not decades, but just needed the option of enforcement. Both, I know, have merit. Some problems are left by this proposal deliberately to other proposals.

For instance, on the subject of dangling, it is best to fix more of this in the language instead of the analyzer. With the following two proposals, the dangling mountain could be shrunk to a mole-hill or ant-hill.

- `implicit constant initialization` [^p2623r2]
- `temporary storage class specifiers` [^p2658r0]

Further adding the paper that those two were based on would further shrink dangling to a few grains of sand on a sea shore of code. 

- `Bind Returned/Initialized Objects to Lifetime of Parameters` [^p0936r0]
- `[RFC] Lifetime annotations for C++` [^clangla]

On the subject of type safety, this paper agrees with the `p2687r0` proposal on the usage of SELL, Semantically Enhanced Language
Libraries. Currently, there is work ongoing in the standardization process to provide a standard units library which would go a long ways for type safety. Currently, there is work ongoing in the standardization process to provide a standard graph library which would go a long ways for the more extreme memory safety. Still unproposed but still needed are strongly typed alias library or language features for safer `int`(s). Enhancements to existing fundamental types in `C++` could include validation and tag classes in order to make those types safer.

In short, this proposal is, in some ways, a subset of the `p2687r0` proposal. Combined with other proposals, they beat the most notorious safety problems into an acceptable level of safety to many.

## Acknowledgments

Thanks to Vladimir Smirnov<!--mapron1@gmail.com--> for providing very valuable feedback on this proposal.

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
[^cppcg]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>


<!--C++ Core Guidelines - P.4: Ideally, a program should be statically type safe-->
[^cppcgp4]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p4-ideally-a-program-should-be-statically-type-safe>
<!--C++ Core Guidelines - P.6: What cannot be checked at compile time should be checkable at run time-->
[^cppcgp6]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p6-what-cannot-be-checked-at-compile-time-should-be-checkable-at-run-time>
<!--C++ Core Guidelines - P.7: Catch run-time errors early-->
[^cppcgp7]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p7-catch-run-time-errors-early>
<!--C++ Core Guidelines - P.8: Don’t leak any resources-->
[^cppcgp8]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p8-dont-leak-any-resources>
<!--C++ Core Guidelines - P.11: Encapsulate messy constructs, rather than spreading through the code-->
[^cppcgp11]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p11-encapsulate-messy-constructs-rather-than-spreading-through-the-code>
<!--C++ Core Guidelines - P.12: Use supporting tools as appropriate-->
[^cppcgp12]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p12-use-supporting-tools-as-appropriate>
<!--C++ Core Guidelines - P.13: Use support libraries as appropriate-->
[^cppcgp13]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p13-use-support-libraries-as-appropriate>
<!--C++ Core Guidelines - I.4: Make interfaces precisely and strongly typed-->
[^cppcgi4]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i4-make-interfaces-precisely-and-strongly-typed>
<!--C++ Core Guidelines - I.11: Never transfer ownership by a raw pointer (T*) or reference (T&)-->
[^cppcgi11]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i11-never-transfer-ownership-by-a-raw-pointer-t-or-reference-t>
<!--C++ Core Guidelines - I.12: Declare a pointer that must not be null as not_null-->
[^cppcgi12]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i12-declare-a-pointer-that-must-not-be-null-as-not_null>
<!--C++ Core Guidelines - I.13: Do not pass an array as a single pointer-->
[^cppcgi13]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i13-do-not-pass-an-array-as-a-single-pointer>
<!--C++ Core Guidelines - I.23: Keep the number of function arguments low-->
[^cppcgi23]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i23-keep-the-number-of-function-arguments-low>
<!--C++ Core Guidelines - F.7: For general use, take T* or T& arguments rather than smart pointers-->
[^cppcgf7]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f7-for-general-use-take-t-or-t-arguments-rather-than-smart-pointers>
<!--C++ Core Guidelines - F.15: Prefer simple and conventional ways of passing information-->
[^cppcgf15]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f15-prefer-simple-and-conventional-ways-of-passing-information>
<!--C++ Core Guidelines - F.22: Use T* or owner<T*> to designate a single object-->
[^cppcgf22]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f22-use-t-or-ownert-to-designate-a-single-object>
<!--C++ Core Guidelines - F.23: Use a not_null<T> to indicate that “null” is not a valid value-->
[^cppcgf23]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f23-use-a-not_nullt-to-indicate-that-null-is-not-a-valid-value>
<!--C++ Core Guidelines - F.25: Use a zstring or a not_null<zstring> to designate a C-style string-->
[^cppcgf25]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f25-use-a-zstring-or-a-not_nullzstring-to-designate-a-c-style-string>
<!--C++ Core Guidelines - F.26: Use a unique_ptr<T> to transfer ownership where a pointer is needed-->
[^cppcgf26]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f26-use-a-unique_ptrt-to-transfer-ownership-where-a-pointer-is-needed>
<!--C++ Core Guidelines - F.27: Use a shared_ptr<T> to share ownership-->
[^cppcgf27]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f27-use-a-shared_ptrt-to-share-ownership>
<!--C++ Core Guidelines - F.42: Return a T* to indicate a position (only)-->
[^cppcgf42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f42-return-a-t-to-indicate-a-position-only>
<!--C++ Core Guidelines - F.43: Never (directly or indirectly) return a pointer or a reference to a local object-->
[^cppcgf43]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f43-never-directly-or-indirectly-return-a-pointer-or-a-reference-to-a-local-object>
<!--C++ Core Guidelines - F.55: Don’t use va_arg arguments-->
[^cppcgf55]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f55-dont-use-va_arg-arguments>
<!--C++ Core Guidelines - C.31: All resources acquired by a class must be released by the class’s destructor-->
[^cppcgc31]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c31-all-resources-acquired-by-a-class-must-be-released-by-the-classs-destructor>
<!--C++ Core Guidelines - C.32: If a class has a raw pointer (T*) or reference (T&), consider whether it might be owning-->
[^cppcgc32]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c32-if-a-class-has-a-raw-pointer-t-or-reference-t-consider-whether-it-might-be-owning>
<!--C++ Core Guidelines - C.33: If a class has an owning pointer member, define a destructor-->
[^cppcgc33]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c33-if-a-class-has-an-owning-pointer-member-define-a-destructor>
<!--C++ Core Guidelines - C.146: Use dynamic_cast where class hierarchy navigation is unavoidable-->
[^cppcgc146]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c146-use-dynamic_cast-where-class-hierarchy-navigation-is-unavoidable>
<!--C++ Core Guidelines - C.149: Use unique_ptr or shared_ptr to avoid forgetting to delete objects created using new-->
[^cppcgc149]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c149-use-unique_ptr-or-shared_ptr-to-avoid-forgetting-to-delete-objects-created-using-new>
<!--C++ Core Guidelines - C.150: Use make_unique() to construct objects owned by unique_ptrs-->
[^cppcgc150]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c150-use-make_unique-to-construct-objects-owned-by-unique_ptrs>
<!--C++ Core Guidelines - C.151: Use make_shared() to construct objects owned by shared_ptrs-->
[^cppcgc151]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c151-use-make_shared-to-construct-objects-owned-by-shared_ptrs>
<!--C++ Core Guidelines - C.181: Avoid “naked” unions-->
[^cppcgc181]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c181-avoid-naked-unions>
<!--C++ Core Guidelines - R.1: Manage resources automatically using resource handles and RAII (Resource Acquisition Is Initialization)-->
[^cppcgr1]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r1-manage-resources-automatically-using-resource-handles-and-raii-resource-acquisition-is-initialization>
<!--C++ Core Guidelines - R.2: In interfaces, use raw pointers to denote individual objects (only)-->
[^cppcgr2]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r2-in-interfaces-use-raw-pointers-to-denote-individual-objects-only>
<!--C++ Core Guidelines - R.3: A raw pointer (a T*) is non-owning-->
[^cppcgr3]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r3-a-raw-pointer-a-t-is-non-owning>
<!--C++ Core Guidelines - R.5: Prefer scoped objects, don’t heap-allocate unnecessarily-->
[^cppcgr5]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r5-prefer-scoped-objects-dont-heap-allocate-unnecessarily>
<!--C++ Core Guidelines - R.10: Avoid malloc() and free()-->
[^cppcgr10]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r10-avoid-malloc-and-free>
<!--C++ Core Guidelines - R.11: Avoid calling new and delete explicitly-->
[^cppcgr11]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r11-avoid-calling-new-and-delete-explicitly>
<!--C++ Core Guidelines - R.12: Immediately give the result of an explicit resource allocation to a manager object-->
[^cppcgr12]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r12-immediately-give-the-result-of-an-explicit-resource-allocation-to-a-manager-object>
<!--C++ Core Guidelines - R.13: Perform at most one explicit resource allocation in a single expression statement-->
[^cppcgr13]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r13-perform-at-most-one-explicit-resource-allocation-in-a-single-expression-statement>
<!--C++ Core Guidelines - R.14: Avoid [] parameters, prefer span-->
[^cppcgr14]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r14-avoid--parameters-prefer-span>
<!--C++ Core Guidelines - R.15: Always overload matched allocation/deallocation pairs-->
[^cppcgr15]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r15-always-overload-matched-allocationdeallocation-pairs>
<!--C++ Core Guidelines - R.20: Use unique_ptr or shared_ptr to represent ownership-->
[^cppcgr20]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r20-use-unique_ptr-or-shared_ptr-to-represent-ownership>
<!--C++ Core Guidelines - R.22: Use make_shared() to make shared_ptrs-->
[^cppcgr22]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r22-use-make_shared-to-make-shared_ptrs>
<!--C++ Core Guidelines - R.23: Use make_unique() to make unique_ptrs-->
[^cppcgr23]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r23-use-make_unique-to-make-unique_ptrs>
<!--C++ Core Guidelines - ES.20: Always initialize an object-->
[^cppcges20]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es20-always-initialize-an-object>
<!--C++ Core Guidelines - ES.24: Use a unique_ptr<T> to hold pointers-->
[^cppcges24]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es24-use-a-unique_ptrt-to-hold-pointers>
<!--C++ Core Guidelines - ES.34: Don’t define a (C-style) variadic function-->
[^cppcges34]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#-es34-dont-define-a-c-style-variadic-function>
<!--C++ Core Guidelines - ES.42: Keep use of pointers simple and straightforward-->
[^cppcges42]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es42-keep-use-of-pointers-simple-and-straightforward>
<!--C++ Core Guidelines - ES.47: Use nullptr rather than 0 or NULL-->
[^cppcges47]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es47-use-nullptr-rather-than-0-or-null>
<!--C++ Core Guidelines - ES.48: Avoid casts-->
[^cppcges48]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es48-avoid-casts>
<!--C++ Core Guidelines - ES.49: If you must use a cast, use a named cast-->
[^cppcges49]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es49-if-you-must-use-a-cast-use-a-named-cast>
<!--C++ Core Guidelines - ES.50: Don’t cast away const-->
[^cppcges50]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es50-dont-cast-away-const>
<!--C++ Core Guidelines - ES.60: Avoid new and delete outside resource management functions-->
[^cppcges60]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es60-avoid-new-and-delete-outside-resource-management-functions>
<!--C++ Core Guidelines - ES.61: Delete arrays using delete[] and non-arrays using delete-->
[^cppcges61]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es61-delete-arrays-using-delete-and-non-arrays-using-delete>
<!--C++ Core Guidelines - ES.65: Don’t dereference an invalid pointer-->
[^cppcges65]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es65-dont-dereference-an-invalid-pointer>
<!--C++ Core Guidelines - ES.76: Avoid goto-->
[^cppcges76]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es76-avoid-goto>
<!--C++ Core Guidelines - ES.84: Don’t try to declare a local variable with no name-->
[^cppcges84]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es84-dont-try-to-declare-a-local-variable-with-no-name>
<!--C++ Core Guidelines - CP.8: Don’t try to use volatile for synchronization-->
[^cppcgcp8]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cp8-dont-try-to-use-volatile-for-synchronization>
<!--C++ Core Guidelines - E.13: Never throw while being the direct owner of an object-->
[^cppcge13]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#e13-never-throw-while-being-the-direct-owner-of-an-object>
<!--C++ Core Guidelines - CPL.1: Prefer C++ to C-->
[^cppcgcpl1]: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#cpl1-prefer-c-to-c>
<!--A Modern C++ Signature for main-->
[^modernmain]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0781r0.html>
<!--Desert Sessions: Improving hostile environment interactions-->
[^hostileenv]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1275r0.html>
<!--History of C++-->
[^history]: <https://en.cppreference.com/w/cpp/language/history>
<!--Bjarne Stroustrup's FAQ-->
[^bsfaq]: <https://www.stroustrup.com/bs_faq.html>
<!--Bjarne Stroustrup's FAQ-Are there any features you'd like to remove from C++?-->
[^bsfaqremovefeature]: <https://www.stroustrup.com/bs_faq.html#remove-from-C++>
<!--Bjarne Stroustrup's FAQ-What do you think of EC++?-->
[^bsfaqsubset]: <https://www.stroustrup.com/bs_faq.html#EC++>
<!--Bjarne Stroustrup Quotes-->
[^bsq]: <https://www.stroustrup.com/quotes.html>
<!--Leaving no room for a lower-level language: A C++ Subset-->
[^freestanding]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1105r1.html>
<!--function_ref: a type-erased callable reference-->
[^functionref]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0792r11.html>
<!--make function_ref more functional-->
[^morefunctional]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2472r3.html>
<!--Design Alternatives for Type-and-Resource Safe C++-->
[^p2687r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2687r0.pdf>
<!--temporary storage class specifiers-->
[^p2658r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2658r0.html>
<!--implicit constant initialization-->
[^p2623r2]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2623r2.html>
<!--Bind Returned/Initialized Objects to Lifetime of Parameters-->
[^p0936r0]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0936r0.pdf>
<!--[RFC] Lifetime annotations for C++-->
[^clangla]: <https://discourse.llvm.org/t/rfc-lifetime-annotations-for-c/61377>
