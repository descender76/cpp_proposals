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