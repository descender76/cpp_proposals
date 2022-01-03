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
  - [Motivating overview](#motivating-overview)
  - [Solution](#solution)
  - [Motivating examples](#motivating-examples)
  - [Feature test macro](#feature-test-macro)
  - [Other Languages](#other-languages)
  - [Example implementation](#example-implementation)
  - [Acknowledgments](#acknowledgments)
  - [Annex: alternative design decisions](#annex-alternative-design-decisions)
    - [auto](#auto)
    - [General Purpose Callable Library](#general-purpose-callable-library)
  - [References](#references)

## Abstract

I propose to make `function_ref` [^p0792r5] easier and safer to use with pointer-to-member-functions by adding a std::make_function_ref factory function. Also the pointer-to-free-function and pointer-to-member-function cases should be expanded to easily and safely include type erased state just like functors and capturing lambdas has.

## Motivating overview

* std::function_ref has an api similar to that of std::function. However, its constructors are hard to use correctly with pointer-to-member-function and other common use cases.
* std::function_ref currently has no interface to allow it to store the this pointer. This is an intentional design choice of P0792 (which tries to be consistent with the language-syntax of pointer-to-member-function). However I found that interface to be surprising, and often leading to access to dangling objects.
* Because function_ref is expected to be a reference to a callable, we can support proper pointer-to-member-function usage without increasing the size of the std::function_ref (no copy needs to be made).

## Solution

### Proposal #1 - The minimal proposal

The following are the minimal changes needed to support make_function_ref and other author's proposals in the future. It consists of 2 changes to the current function_ref proposal.

1. Tie down the specification to a double pointer implementation; a void* of state and a free function pointer that takes that state as its first perameter. In short, remove the exposition only from the class definition in the current function_ref proposal.
2. Add a static factory function, named "construct_from_type_erased", that trivially initialize the two aforementioned pointers.

Not making this change in C++23 would constitute a breaking change because it would require implementations that did not use the two pointer solution to have to change.

```cpp
template <class F> class function_ref;

/// Specializations for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void* erased_object;
  R(*erased_function)(void*, Args...);
  // NOTE:
  // 1) The following has been removed: // exposition only
  // 2) "void*, " was added to erased_function
};

// NOTE: noexcept version was added
template <class R, class... Args> class function_ref<R(Args...) noexcept> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void* erased_object;
  R(*erased_function)(void*, Args...);
};
```

### Proposal #2 - The make_function_ref proposal

The make_function_ref proposal is a superset of the minimal proposal. It utilizes the "construct_from_type_erased" plumbing function to create the type safe "make_function_ref" factory functions. These functions DO NOT replace the current constructor. While, the current function_ref proposal is focused on callables, callable objects, such as functors and lambda functions, make_function_ref is focused on functions in general. While the current function_ref constructor handles implicit conversion of parameters, make_function_ref handles explicit simple assignment of ~exact signatures. This focus of make_function_ref makes it easier in general and safer when it comes to dangling. Other than when these two construction methodologies overlap with the stateless free and member function pointers, they are largely complimentary.

```cpp
template <class F> class function_ref;

/// Specializations for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void* erased_object;
  R(*erased_function)(void*, Args...);
  // NOTE:
  // 1) The following has been removed: // exposition only
  // 2) "void*, " was added to erased_function
};

// NOTE: noexcept version was added
template <class R, class... Args> class function_ref<R(Args...) noexcept> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void* erased_object;
  R(*erased_function)(Args...);
};

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref(T& obj);

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref(const T& obj);

template<auto mf> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref();

template<typename testType>
struct is_function_pointer
{
    static const bool value = std::is_function_v<std::remove_pointer_t<testType>>;
};

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(T& obj);

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(const T& obj);

template<auto f> requires is_function_pointer<decltype(f)>::value
auto make_function_ref();
```

### Proposal #2++ - The make_function_ref++ proposal

"function_ref" and "make_function_ref" overlap when it comes to [stateless] assigning of a free or member function.

|                 |    make_function_ref | vs | function_ref                         |
|-----------------|-----------|----------|----|-------------|------------------------|
|                 | Stateless | Stateful |    | Stateless   | Stateful               |
| Free Function   | ✓         | ✓        |    | ✓ different | ✗ use capturing lambda |
| Member Function | ✓         | ✓        |    | ✗ dangle    | ✗ use capturing lambda |

The questions are should either free or member function assignment be removed from function_ref or make_function_ref!

#### Should the make_function_ref proposal or the function_ref proposal drop initialization from a free function?

The make_function_ref implementation is not the same as the function_ref implementation. In make_function_ref, a free function is passed as a template parameter making it more compile time. The internal state pointer goes unused and likely initialized to nullptr. The function_ref implementation is more runtime by storing the function pointer in the internal state pointer member of function_ref. In this case, the function_ref proposal is easier in the sense of a constructor over a factory function. It is also more flexible in the sense a user can wait latter in their code to bind the function pointer to the function_ref. make_function_ref is more consistent with the other three use case where it is clearly better. Also just like strongly typed primitive types such as email alias type vs the string it is based upon, code quality improves by using function_ref sooner rather than latter. Despite these differences, the gains of having both out weigh the small overlap in functionality.

#### Should the make_function_ref proposal drop initialization from a member function?

Perish the thought. "make_function_ref" does not dangle, the current function_ref proposal can. "make_function_ref" is consistent with its other three use cases.

#### Should the function_ref proposal drop initialization from a member function?

That depends. Should the standardization process go with just the minimal interface i.e. without make_function_ref, than it should remain. Having it is just clunky and users can just learn the multiple right ways of doing things and be on guard against the easy wrong way of doing things that lead to dangling. However, if the standardization does go with the make_function_ref factory functions then MAYBE. Removing it would just be a matter of adding something like " requires !std::is_member_function_pointer::value" to the constructor in the current function_ref proposal. However, for consistency sake, since function_ref can take a free function pointer it could be argued to keep it in. In either case, it doesn't negate the need of the make_function_ref factory functions to address the four free and member, stateless and stateful function pointer use cases.

### Proposal #3 - The scorched Earth/nuclear option

For all the advantages mentioned in the current function_ref proposal, function_ref is an important vocabulary type that like v-table interfaces or traits live and breath on the API/module boundary. If there is any hint of function_ref, with or without make_function_ref, NOT being stanardized in C++23 than I would hope the following is considered instead. Don't have the make_function_ref factory functions. Don't have the current constructor in the current function_ref proposal. Rather, just have the explicit double pointer implementation and the construct_from_type_erased factory function. There is still value in a trivial function_ref even while its construction methodologies gets straightened out in C++26.

```cpp
namespace std
{
    template <typename Signature>
    class function_ref
    {
        void* erased_object;

        R(*erased_function)(void*, Args...);
        // `R`, and `Args...` are the return type, and the parameter-type-list,
        // of the function type `Signature`, respectively.

    public:
        // factory function to be used by make_function_ref and other user defined construction methods
        static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;

        function_ref(const function_ref&) noexcept = default;

        template <typename F>
        function_ref(F&&);

        function_ref& operator=(const function_ref&) noexcept = default;

        template <typename F>
        function_ref& operator=(F&&);

        void swap(function_ref&) noexcept;

        R operator()(Args...) const noexcept(see below);
        // `R` and `Args...` are the return type and the parameter-type-list
        // of the function type `Signature`, respectively.
    };

    template <typename Signature>
    void swap(function_ref<Signature>&, function_ref<Signature>&) noexcept;
}
```

## Motivating examples

In order to be able to identify such areas of improvement, we need to look at the `reference implementation` [^functionref] for function_ref. This also requires that we, momentarily, add one constructor that provides direct access to its two pointers. This constructor serves the same purpose as the proposed static factory function named "construct_from_type_erased" in the previous solution. A constructor is instead used here to reduce the additional syntax in order to ensure we are making a fair comparison between the varying construction methods.

### The given

```cpp
template <class F> class function_ref;

/// Specializations for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  function_ref(void* obj_, R (*callback_)(void*,Args...)) noexcept : obj_{obj_}, callback_{callback_} {}
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};

template <class R, class... Args> class function_ref<R(Args...) noexcept> {
public:
  function_ref(void* obj_, R (*callback_)(void*,Args...)) noexcept : obj_{obj_}, callback_{callback_} {}
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) noexcept = nullptr;
};
```

All examples unless stated otherwise were compiled on [godbolt.org](https://www.godbolt.org/) using "x86-64 clang (trunk)" with "-std=c++20 -O2" in order to identify dangling.

In the following examples, function_ref is compared to std::function in order to identify it behaving unexpectedly. "function_ref" is also compared with itself to highlight inconsistencies in its own interface.

```cpp
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

int main() {
    bar b;
    tl::function_ref<void(bar)> fr1 = &bar::baz;// immediately dangle
    tl::function_ref<void(bar)> fr2 = foo;// does not dangle
    tl::function_ref<void(bar)> fr3 = &foo;// immediately dangle
    // the following behaves as expected
    std::function<void(bar)> fr4 = &bar::baz;
    std::function<void(bar)> fr5 = foo;
    std::function<void(bar)> fr6 = &foo;
    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);
    return 42;
}
```

There are two things to note from the previous example. First, std::function works as expected in all three scenarios. The comparable function_ref has immediate dangling in two of the three. Second, when you compare function_ref to itself, there is a scenario where free function works but member function doesn't. This treats object oriented programming with less support than functional programming. Its been said that this is the incorrect usage of function_ref, in effect blaming the user instead of the specification and implementation. Those that believe such may give the following example as proof as it is ok to standardize something that can immediately dangle.

```cpp
std::string_view sv = "hello world"s;  // immediately dangling
```

While I normally would agree with such sentiment, in this scenario I whole heartedly disagree. Dangling occurs when you still have a reference to state that has gone out of scope, no longer exists. In this string_view example, it is clear that sv is a reference and ""s is state. It is totally unclear in the case of function_ref initialization. Dangling is not what the end user is intending on doing when a function is assigned whether free or member. Functions are stateless and also global, so they don't go out of scope. This implementation stores a free function pointer as the void* state of function_ref and because a member function pointer is larger than void* it is treated just like a capturing lambda or functor of size greater than a void*. In essence, implementation details are bleeding through function_ref's public interface and effecting how users are to use function_ref. The current function_ref specification is currently silent on how implementations should behave and how end users should consume in light of its differences with std::function.

Another example given by the current proponents of function_ref is how the users of function_ref should fix the dangling.

```cpp
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

void correct_usage_of_current_function_ref(tl::function_ref<void(bar)> fr1, tl::function_ref<void(bar)> fr2) {
    bar b;
    fr1(b);
    fr2(b);    
}

int main() {
    auto temp = &bar::baz;// WORKAROUND
    tl::function_ref<void(bar)> fr1 = temp;
    auto temp = &foo;// WORKAROUND
    tl::function_ref<void(bar)> fr2 = temp;
    bar b;
    fr1(b);
    fr2(b);    

    correct_usage_of_current_function_ref(&foo, &bar::baz);
    return 42;
}
```

Some proponents of the current function_ref proposal advocate for not even messing with the WORKAROUND which is sort of how one currently deals with dangling today and that the correct usage is to just initialize the function_ref directly in the function parameter in question. I would argue that we should still be able to initialize first because I may provide the same function_ref instantiation to multiple parameters across multiple function calls. Otherwise, duplicating an exact same lambda, in more complicated scenarios, would increase verbosity and violating in programming the one definition rule or don't repeat yourself. This usage is also plagued with the superfluous workaround line. Why initialize with 2 lines of code when it should just be one! Again, it is not like std::function or how we initialize in C++. Why force end users to work around dangling needlessly when the library implementation is more than capable of handling this? By fixing these trivial dangling noise, we leave the remaining dangling cases to be as clear as the std::string_view example. So, how do we fix some of this dangling in the implementation!

Given the new constructor mentioned in the [Overview](#overview) we can construct an example that does not dangle.

```cpp
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

int main() {
    tl::function_ref<void(bar)> fr1 = &bar::baz;// immediately dangle
    tl::function_ref<void(bar)> fr2 = &foo;// immediately dangle
    // no dangling, this proposal via new constructor
    tl::function_ref<void(bar)> fr3 = {nullptr, [](void*, bar b){b.baz();}};
    tl::function_ref<void(bar)> fr4 = {nullptr, [](void*, bar b){foo(b);}};
    // no dangling, original proposal via stateless lambda
    tl::function_ref<void(bar)> fr5 = [](bar b){b.baz();};
    tl::function_ref<void(bar)> fr6 = [](bar b){foo(b);};

    bar b;

    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);

    return 42;
}
```

This set of examples shows how free function and member function without type erasure use cases can be handled without any dangling and without the syntax of having to put function_ref initialization parameter on a seperate temporary line. The pair of examples that uses the new constructor is clear that their is no dangling because their is no state, both by the nullptr and the lambda used doesn't capture i.e. stateless. Similarly, the lambda examples that uses the current function_ref constructor is similarly non capturing, stateless. The problem with these two solutions is the lack of clarity because we can't simply assign a function with a known compatible signature. This means we have [], (), forwarded aparmeters and possibly return statement. The new constructor examples are also not type safe, though in a stateless scenario, that is a little less of an issue. However, by nailing down the specification and providing the new constructor, we can do much better. Using the new constructor, "make_function_ref" factory functions can be created that type safely creates function_ref from member or free functions without any dangling.

```cpp
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar&) {
}

// side by side examples
void examples() {
    tl::function_ref<void(bar&)> fr1 = {nullptr, [](void*, bar& b){b.baz();}};
    tl::function_ref<void(bar&)> fr2 = {nullptr, [](void*, bar& b){foo(b);}};
    tl::function_ref<void(bar&)> fr3 = [](bar& b){b.baz();};
    tl::function_ref<void(bar&)> fr4 = [](bar& b){foo(b);};
    tl::function_ref<void(bar&)> fr5 = tl::make_function_ref<foo>();
    tl::function_ref<void(bar&)> fr6 = tl::make_function_ref<&foo>();
    tl::function_ref<void(bar&)> fr7 = tl::make_function_ref<&bar::baz>();

    bar b;

    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);
    fr7(b);
}

int main() {
    return 42;
}
```

These make_function_ref examples provides the best of both worlds. No dangling for member and free functions without type erasure and a reasonably concise syntax that explicitly states what the user intends to do.

Adding the new constructor, mentioned in the [Overview](#overview), makes the implementation of function_ref capable of so much more functionality then is currently available. The current function_ref implentation only type erases state for callables such as capturing lambdas but what if your functions or member functions could benefit from type erasure. Instead of making these latter two use cases easy, current users have to use a lambda even if there were no changes in their callable parameters. This too can be improved. Using the new constructor, "make_function_ref" factory functions can be created that type safely creates function_ref from member or free functions with type erased state, as is common in both functional and object oriented programming. Since the usage of these factory functions are intended for objects that already exist, than in this scenario too there is significantly less dangling that a user has to deal with.

```cpp
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
    void operator()(){}
};

void foo(bar&) {
}

// side by side examples
void examples() {
    bar b;
    tl::function_ref<void()> fr1 = [&b](){b.baz();};
    tl::function_ref<void()> fr2 = [&b](){foo(b);};
    // using the new constructor
    tl::function_ref<void()> fr3 = {&b, [](void* obj){static_cast<bar*>(obj)->baz();}};
    tl::function_ref<void()> fr4 = {&b, [](void* obj){foo(*static_cast<bar*>(obj));}};
    // using make_function_ref which uses new constructor
    tl::function_ref<void()> fr5 = tl::make_function_ref<foo>(b);
    tl::function_ref<void()> fr6 = tl::make_function_ref<&foo>(b);
    tl::function_ref<void()> fr7 = tl::make_function_ref<&bar::baz>(b);
    tl::function_ref<void()> fr8 = tl::make_function_ref<&bar::operator()>(b);
    // C#
    // delegate void some_name();
    // some_name fr = foo;
    // some_name fr = b.baz;
    ///////////////////////////////
    // Borland C++, embarcadero __closure
    // void(__closure * fr)();
    // fr = foo
    // fr = b.baz

    fr1();
    fr2();
    fr3();
    fr4();
    fr5();
    fr6();
    fr7();
    fr8();
}

int main() {
    return 42;
}
```

Here, the type erased free and member function use cases are consistent with the free and member use cases that do not have type erasure. It is concise. Conveys the intent of the user and provides a feature found in other proramming languages that C++ users have been clamoring for decades. There is an additional reason that goes to ease of use. When you use a stateful lambda to needlessly tie the type erased state to a method, you are in essence creating a new anonymous type and instance which has its own lifetime that must be managed by the user. Superflous code is being added which the optimizer has to remove. Usage of function_ref becomes harder to reason about. You in essense have a function_ref which is a reference, refering to a lambda which is a reference that references the actual object in question. This makes it harder to reason about any remaining dangling. A reference to a reference is not a reference. It is rather a pointer which has multiple levels of indirection. A reference was always a collapsable single level of indirection.

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

The most up-to-date implementation, created by Jarrad Waterloo, is available on `Github` [^functionrefprime]

## Acknowledgments

Thanks to Arthur O'Dwyer, Tomasz Kamiński and Corentin Jabot for providing very valuable feedback on this proposal.

## Annex: alternative design decisions

### Why not just use `[&b](auto... args){b.baz(args...);}`?

While succinct, this does have some undesirable consequences.

* It is more complex; at least cognitively speaking. The only pieces the end user really cares about is either "=baz" or "=b.baz". The capture, the parameter propagation even with auto and the occassional return are all superfluous.
* If this was a C function pointer would the end user be happy with that versus just assigning with "= some_function". In this case the public wrapper interface is more complicated than the internals. Shouldn't that be reversed i.e. abstraction.
* Using a functor or stateful lambda does require creating a object and type even if anonymous that has a lifetime that must be managed making things more complicated.
* Construction which should be one step has now been broken into two which is more cumbersome; see the next decision.

### Why not just use bind_front and turn the remaining make_function_ref implementations into a general purpose functors library?

I actually agree that C++ could use a separate function transformation library for the general cases but I don't think that should negate function_ref supporting compatible signatures; i.e. first class support. Such a general purpose library would turn what should be a one step construction process into a two step process; first create a functor and then assign it to the function_ref. Personally I prefer one step and here is why. function_ref is to function pointer, in C, as std::array is to C/C++ array. It is more intuitive/familiar to the end user. a function pointer is simply assigned for compatible/comparable signatures and the end user must create a new function for incompatible signatures in order to do conversions or reordering of parameters and in the case of functors, adding more parameters. This two phase construction is more verbose. make_function_ref is less so and the same name regardless of what is being assigned. It is also similar to make_pair, make_shared_ptr and the other single step make functions; again familiar. I would hesitate to say that end users expect a simple construction methodology based on the prevalence of simpler solutions in other languages and there is no good reason why we should make things deliberately harder especially for common cases when it is not needed to be so.

It should also be noted that their are current C++ limitations that restrict our implementation choices.

* C++ template constructors do not support explicit template parameters meaning having to resort to tag classes which make things more verbose.
* Template functions do not support partial specialization meaning such things that should simply be performed by functions has to be performed by whole partially specialized classes making things more verbose.
* Making heavily templatized make_function_ref functions into FRIEND functions of function_ref is borderline impossible, if not so. This is why I still need my public double pointer constructor. I am still for having this constructor anyway for advance end users to prototype other compatible signatures for operators and other function like constructs in C++ that currently don't have a natural binding.

At present, my current make_function_ref solution seems to provide a very concise single step construction that enhances function_ref to support better the 4 use cases: [non] type erased [member/free] functions.

## References

[^p0792r5]: <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0792r5.html>
[^functionref]: <https://github.com/TartanLlama/function_ref>
[^functionrefprime]: <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>
[^delegates]: <https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/using-delegates>
[^closure]: <http://docwiki.embarcadero.com/RADStudio/Sydney/en/Closure>
[^fastest]: <https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible>
[^impossiblyc]: <https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates>
[^impossiblyd]: <https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11>
[^dyno]: <https://github.com/ldionne/dyno>
[^boostextte]: <https://github.com/boost-ext/te>

