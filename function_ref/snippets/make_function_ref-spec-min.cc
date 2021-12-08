template <class F> class function_ref;

/// Specializations for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void *obj_ = nullptr;// NOT exposition only
  R (*callback_)(void *, Args...) = nullptr;// NOT exposition only
};

template <class R, class... Args> class function_ref<R(Args...) noexcept> {
public:
  // factory function to be used by make_function_ref and other user defined construction methods
  static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;
private:
  void *obj_ = nullptr;// NOT exposition only
  R (*callback_)(void *, Args...) noexcept = nullptr;// NOT exposition only
};