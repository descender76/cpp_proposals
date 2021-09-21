#include <iostream>
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
//#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
// function_ref - A low-overhead non-owning function
// Written in 2017 by Simon Brand (@TartanLlama)
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
///

#ifndef TL_FUNCTION_REF_HPP
#define TL_FUNCTION_REF_HPP

#define TL_FUNCTION_REF_VERSION_MAJOR 1
#define TL_FUNCTION_REF_VERSION_MINOR 0
#define TL_FUNCTION_REF_VERSION_PATCH 0

#if (defined(_MSC_VER) && _MSC_VER == 1900)
/// \exclude
#define TL_FUNCTION_REF_MSVC2015
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
/// \exclude
#define TL_FUNCTION_REF_GCC49
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 4 &&              \
     !defined(__clang__))
/// \exclude
#define TL_FUNCTION_REF_GCC54
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
// GCC < 5 doesn't support overloading on const&& for member functions
/// \exclude
#define TL_FUNCTION_REF_NO_CONSTRR
#endif

#if __cplusplus > 201103L
/// \exclude
#define TL_FUNCTION_REF_CXX14
#endif

// constexpr implies const in C++11, not C++14
#if (__cplusplus == 201103L || defined(TL_FUNCTION_REF_MSVC2015) ||            \
     defined(TL_FUNCTION_REF_GCC49)) &&                                        \
    !defined(TL_FUNCTION_REF_GCC54)
/// \exclude
#define TL_FUNCTION_REF_11_CONSTEXPR
#else
/// \exclude
#define TL_FUNCTION_REF_11_CONSTEXPR constexpr
#endif

#include <functional>
#include <utility>

namespace tl {
namespace detail {
namespace fnref {
// C++14-style aliases for brevity
template <class T> using remove_const_t = typename std::remove_const<T>::type;
template <class T>
using remove_reference_t = typename std::remove_reference<T>::type;
template <class T> using decay_t = typename std::decay<T>::type;
template <bool E, class T = void>
using enable_if_t = typename std::enable_if<E, T>::type;
template <bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

// std::invoke from C++17
// https://stackoverflow.com/questions/38288042/c11-14-invoke-workaround
template <typename Fn, typename... Args,
          typename = enable_if_t<std::is_member_pointer<decay_t<Fn>>::value>,
          int = 0>
constexpr auto invoke(Fn &&f, Args &&... args) noexcept(
    noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
    -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
  return std::mem_fn(f)(std::forward<Args>(args)...);
}

template <typename Fn, typename... Args,
          typename = enable_if_t<!std::is_member_pointer<decay_t<Fn>>{}>>
constexpr auto invoke(Fn &&f, Args &&... args) noexcept(
    noexcept(std::forward<Fn>(f)(std::forward<Args>(args)...)))
    -> decltype(std::forward<Fn>(f)(std::forward<Args>(args)...)) {
  return std::forward<Fn>(f)(std::forward<Args>(args)...);
}

// std::invoke_result from C++17
template <class F, class, class... Us> struct invoke_result_impl;

template <class F, class... Us>
struct invoke_result_impl<
    F, decltype(tl::detail::fnref::invoke(std::declval<F>(), std::declval<Us>()...), void()),
    Us...> {
  using type = decltype(tl::detail::fnref::invoke(std::declval<F>(), std::declval<Us>()...));
};

template <class F, class... Us>
using invoke_result = invoke_result_impl<F, void, Us...>;

template <class F, class... Us>
using invoke_result_t = typename invoke_result<F, Us...>::type;

template <class, class R, class F, class... Args>
struct is_invocable_r_impl : std::false_type {};

template <class R, class F, class... Args>
struct is_invocable_r_impl<
    typename std::is_convertible<invoke_result_t<F, Args...>, R>::type, R, F, Args...>
    : std::true_type {};

template <class R, class F, class... Args>
using is_invocable_r = is_invocable_r_impl<std::true_type, R, F, Args...>;

} // namespace detail
} // namespace fnref

template <typename T, T> struct internal_function_traits;

template <typename R, typename ...Args, R (*f)(Args...)>
struct internal_function_traits<R (*)(Args...), f>
{
	typedef R (function_signature)(Args ... args);
    static R prepend_void_pointer(void * obj, Args ... args)
    {
        return f(std::forward<Args>(args)...);
    }
};

template<auto FP>
using function_traits = internal_function_traits<decltype(FP), FP>;

template <typename T, T> struct internal_member_function_traits;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_member_function_traits<R (T::*)(Args...), mf>
{
	typedef R (function_signature)(Args ... args);
	//typedef R (*function_pointer) (Args ... args);
	//typedef R (T::*member_function_pointer)(Args ... args);
	typedef R (this_as_ref_function_signature)(T&, Args ... args);
	typedef R (this_as_pointer_function_signature)(T*, Args ... args);
	typedef R (this_as_value_function_signature)(T, Args ... args);
    static R type_erase_this(void * obj, Args ... args)
    {
        return (static_cast<T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
    static R this_as_ref(void * obj, T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static R this_as_pointer(void * obj, T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static R this_as_value(void * obj, T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_member_function_traits<R (T::*)(Args...) const, mf>
{
	typedef R (function_signature)(Args ... args);
	//typedef R (*function_pointer) (Args ... args);
	//typedef R (T::*member_function_pointer)(Args ... args) const;
	typedef R (this_as_ref_function_signature)(const T&, Args ... args);
	typedef R (this_as_pointer_function_signature)(const T*, Args ... args);
	typedef R (this_as_value_function_signature)(const T, Args ... args);
    static R type_erase_this(void * obj, Args ... args)
    {
        return (static_cast<const T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
    static R this_as_ref(void * obj, const T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static R this_as_pointer(void * obj, const T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static R this_as_value(void * obj, const T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template<auto MFP>
using member_function_traits = internal_member_function_traits<decltype(MFP), MFP>;

template <typename T, T> struct internal_type_erase_first;

template <typename R, typename FirstArg, typename ...Args, R (*f)(FirstArg&, Args...)>
struct internal_type_erase_first<R (*)(FirstArg& fa, Args...), f>
{
	typedef R (function_signature)(Args ... args);
	typedef R (*function_pointer) (Args ... args);
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(*static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
};

template <typename R, typename FirstArg, typename ...Args, R (*f)(const FirstArg&, Args...)>
struct internal_type_erase_first<R (*)(const FirstArg& fa, Args...), f>
{
	typedef R (function_signature)(Args ... args);
	typedef R (*function_pointer) (Args ... args);
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(*static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
};

template <typename R, typename FirstArg, typename ...Args, R (*f)(FirstArg*, Args...)>
struct internal_type_erase_first<R (*)(FirstArg* fa, Args...), f>
{
	typedef R (function_signature)(Args ... args);
	typedef R (*function_pointer) (Args ... args);
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
};

template <typename R, typename FirstArg, typename ...Args, R (*f)(const FirstArg*, Args...)>
struct internal_type_erase_first<R (*)(const FirstArg* fa, Args...), f>
{
	typedef R (function_signature)(Args ... args);
	typedef R (*function_pointer) (Args ... args);
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
};

template<auto FP>
using type_erase_first = internal_type_erase_first<decltype(FP), FP>;

/// A lightweight non-owning reference to a callable.
///
/// Example usage:
///
/// ```cpp
/// void foo (function_ref<int(int)> func) {
///     std::cout << "Result is " << func(21); //42
/// }
///
/// foo([](int i) { return i*2; });
template <class F> class function_ref;

/// Specialization for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  constexpr function_ref() noexcept = delete;

  /// Creates a `function_ref` which refers to the same callable as `rhs`.
    constexpr function_ref(const function_ref<R(Args...)> &rhs) noexcept = default;

  /// Constructs a `function_ref` referring to `f`.
  ///
  /// \synopsis template <typename F> constexpr function_ref(F &&f) noexcept
  template <typename F,
            detail::fnref::enable_if_t<
                !std::is_same<detail::fnref::decay_t<F>, function_ref>::value &&
                detail::fnref::is_invocable_r<R, F &&, Args...>::value> * = nullptr>
  TL_FUNCTION_REF_11_CONSTEXPR function_ref(F &&f) noexcept
      : obj_(const_cast<void*>(reinterpret_cast<const void *>(std::addressof(f)))) {
    callback_ = [](void *obj, Args... args) -> R {
      return detail::fnref::invoke(
          *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
          std::forward<Args>(args)...);
    };
  }

  /// Makes `*this` refer to the same callable as `rhs`.
  TL_FUNCTION_REF_11_CONSTEXPR function_ref<R(Args...)> &
  operator=(const function_ref<R(Args...)> &rhs) noexcept = default;

  /// Makes `*this` refer to `f`.
  ///
  /// \synopsis template <typename F> constexpr function_ref &operator=(F &&f) noexcept;
  template <typename F,
            detail::fnref::enable_if_t<detail::fnref::is_invocable_r<R, F &&, Args...>::value>
                * = nullptr>
  TL_FUNCTION_REF_11_CONSTEXPR function_ref<R(Args...)> &operator=(F &&f) noexcept {
    obj_ = reinterpret_cast<void *>(std::addressof(f));
    callback_ = [](void *obj, Args... args) {
      return detail::fnref::invoke(
          *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
          std::forward<Args>(args)...);
    };

    return *this;
  }

  /// Swaps the referred callables of `*this` and `rhs`.
  constexpr void swap(function_ref<R(Args...)> &rhs) noexcept {
    std::swap(obj_, rhs.obj_);
    std::swap(callback_, rhs.callback_);
  }

  /// Call the stored callable with the given arguments.
  R operator()(Args... args) const {
    return callback_(obj_, std::forward<Args>(args)...);
  }

  function_ref(void* obj_, R (*callback_)(void*,Args...)) noexcept : obj_{obj_}, callback_{callback_} {}
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};

/// Swaps the referred callables of `lhs` and `rhs`.
template <typename R, typename... Args>
constexpr void swap(function_ref<R(Args...)> &lhs,
                    function_ref<R(Args...)> &rhs) noexcept {
  lhs.swap(rhs);
}

#if __cplusplus >= 201703L
template <typename R, typename... Args>
function_ref(R (*)(Args...))->function_ref<R(Args...)>;

// TODO, will require some kind of callable traits
// template <typename F>
// function_ref(F) -> function_ref</* deduced if possible */>;
#endif
} // namespace tl

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref(T& obj)
{
    return tl::function_ref<typename tl::internal_member_function_traits<decltype(mf), mf>::function_signature>{&obj, tl::internal_member_function_traits<decltype(mf), mf>::type_erase_this};
}

template<auto mf> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref()
{
    return tl::function_ref<typename tl::internal_member_function_traits<decltype(mf), mf>::this_as_ref_function_signature>{nullptr, tl::internal_member_function_traits<decltype(mf), mf>::this_as_ref};
}

class ref {};
class pointer {};
class value {};

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value && std::is_same<T, ref>::value
auto make_function_ref()
{
    return tl::function_ref<typename tl::internal_member_function_traits<decltype(mf), mf>::this_as_ref_function_signature>{nullptr, tl::internal_member_function_traits<decltype(mf), mf>::this_as_ref};
}

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value && std::is_same<T, pointer>::value
auto make_function_ref()
{
    return tl::function_ref<typename tl::internal_member_function_traits<decltype(mf), mf>::this_as_pointer_function_signature>{nullptr, tl::internal_member_function_traits<decltype(mf), mf>::this_as_pointer};
}

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value && std::is_same<T, value>::value
auto make_function_ref()
{
    return tl::function_ref<typename tl::internal_member_function_traits<decltype(mf), mf>::this_as_value_function_signature>{nullptr, tl::internal_member_function_traits<decltype(mf), mf>::this_as_value};
}

template<typename testType>
struct is_function_pointer
{
    static const bool value =
        std::is_pointer<testType>::value ?
        std::is_function<typename std::remove_pointer<testType>::type>::value :
        false;
};

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(T& obj)
{
    return tl::function_ref<typename tl::internal_type_erase_first<decltype(f), f>::function_signature>{nullptr, tl::internal_type_erase_first<decltype(f), f>::type_erased_function};
}

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(const T& obj)
{
    return tl::function_ref<typename tl::internal_type_erase_first<decltype(f), f>::function_signature>{nullptr, tl::internal_type_erase_first<decltype(f), f>::type_erased_function};
}

template<auto f> requires is_function_pointer<decltype(f)>::value
auto make_function_ref()
{
    return tl::function_ref<typename tl::internal_function_traits<decltype(f), f>::function_signature>{nullptr, tl::internal_function_traits<decltype(f), f>::prepend_void_pointer};
}

void third_party_lib_function1(tl::function_ref<void(bool, int, float)> callback) {
    callback(true, 11, 3.1459f);
}

double third_party_lib_function2(tl::function_ref<double(int, float, bool)> callback) {
    return callback(11, 3.1459f, true);
}

struct bar {
    void baz(bool b, int i, float f)
    {
        std::cout << "bar::baz" << std::endl;
    }
    double buz(int i, float f, bool b)
    {
        std::cout << "bar::buz" << std::endl;
        return i + f + (b ? 1 : 0);
    }
    void caz(bool b, int i, float f)
    {
        std::cout << "bar::caz" << std::endl;
    }
    void caz(bool b, int i, float f) const
    {
        std::cout << "bar::caz const" << std::endl;
    }
};

void application_function_ref(tl::function_ref<void(bar&, bool, int, float)> callback) {
    bar b;
    callback(b, true, 11, 3.1459f);
}

void application_function_pointer(tl::function_ref<void(bar*, bool, int, float)> callback) {
    bar b;
    callback(&b, true, 11, 3.1459f);
}

void application_function_value(tl::function_ref<void(bar, bool, int, float)> callback) {
    bar b;
    callback(b, true, 11, 3.1459f);
}

void free_baz(bool b, int i, float f)
{
    std::cout << "free_baz" << std::endl;
}

void free_bar_baz1(bar&, bool b, int i, float f)
{
    std::cout << "free_bar_baz1" << std::endl;
}

void free_bar_baz2(const bar&, bool b, int i, float f)
{
    std::cout << "free_bar_baz2" << std::endl;
}

void free_bar_baz3(bar*, bool b, int i, float f)
{
    std::cout << "free_bar_baz3" << std::endl;
}

void free_bar_baz4(const bar*, bool b, int i, float f)
{
    std::cout << "free_bar_baz4" << std::endl;
}

int main()
{
    bar b;
    // member function with type erased state: b
    third_party_lib_function1([&b](bool b1, int i, float f){b.baz(b1, i, f);});// current way
    third_party_lib_function1([&b](auto... args){b.baz(args...);});// current way
    //third_party_lib_function1({b, tl::member_function_traits<&bar::baz>()});// proposed way
    third_party_lib_function2([&b](int i, float f, bool b1){return b.buz(i, f, b1);});// current way
    third_party_lib_function2([&b](auto... args){return b.buz(args...);});// current way
    //third_party_lib_function2({b, tl::member_function_traits<&bar::buz>()});// proposed way
    third_party_lib_function1([&b](bool b1, int i, float f){b.caz(b1, i, f);});// current way
    third_party_lib_function1([&b](auto... args){b.caz(args...);});// current way
    //third_party_lib_function1({b, tl::member_function_traits<static_cast<void(bar::*)(bool b, int i, float f)>(&bar::caz)>()});// proposed way
    //third_party_lib_function1({b, tl::member_function_traits<static_cast<void(bar::*)(bool b, int i, float f) const>(&bar::caz)>()});// proposed way
    //third_party_lib_function1({b, &bar::baz});// proposed way
    //auto temp = make_function_ref<void, bar, bool, int, float, &bar::baz>();
    ///*auto temp =*/ make_function_ref<&bar::baz>(b)(true, 11, 3.1459f);
    // member function with type erasure usecase
    // i.e. delegate, closure, OOP callback, event
    third_party_lib_function1(make_function_ref<&bar::baz>(b));
    // member function without type erasure usecases
    // i.e. unified function pointer
    application_function_ref(make_function_ref<&bar::baz>());
    application_function_ref(make_function_ref<&bar::baz, ref>());
    application_function_pointer(make_function_ref<&bar::baz, pointer>());
    application_function_value(make_function_ref<&bar::baz, value>());
    // function without type erasure usecase
    // i.e. C callback without user data
    third_party_lib_function1(make_function_ref<free_baz>());
    // function with type erasure usecase
    // i.e. C callback with user data
    third_party_lib_function1(make_function_ref<free_bar_baz1>(b));
    third_party_lib_function1(make_function_ref<free_bar_baz2>(b));
    third_party_lib_function1(make_function_ref<free_bar_baz3>(b));
    third_party_lib_function1(make_function_ref<free_bar_baz4>(b));
}
