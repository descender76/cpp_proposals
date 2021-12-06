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
