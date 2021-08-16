template <class F> class function_ref;

/// Specialization for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  function_ref(void *obj_, R (*callback_)(void *, Args...))
  {
	  this->obj_ = obj_;
	  this->callback_ = callback_;
  }
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};