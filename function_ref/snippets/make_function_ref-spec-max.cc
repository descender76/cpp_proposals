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