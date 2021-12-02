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

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref(T& obj);

template<auto mf, typename T> requires std::is_member_function_pointer<decltype(mf)>::value
auto make_function_ref(const T& obj);

template<typename testType>
struct is_function_pointer
{
    static const bool value =
        std::is_pointer<testType>::value ?
        std::is_function<typename std::remove_pointer<testType>::type>::value :
        false;
};

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(T& obj);

template<auto f, typename T> requires is_function_pointer<decltype(f)>::value
auto make_function_ref(const T& obj);