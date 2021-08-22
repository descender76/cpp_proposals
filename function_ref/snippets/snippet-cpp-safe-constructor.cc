template <class F> class function_ref;

/// Specialization for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  function_ref(void *obj_, R (*callback_)(void *, Args...))
  {
	  this->obj_ = obj_;
	  this->callback_ = callback_;
  }
  // safe constructor
  // requires #1 constructor template parameters
  // requires #2 being able to type erase function pointers
  // by casting one function pointer into another
  // function pointer provided only pointers have
  // changed into void pointers
  template<class T>
  function_ref(T *obj_, R (*callback_)(T *, Args...))
  {
	  this->obj_ = obj_;
	  this->callback_ = (R (*)(void*, Args...))callback_;
  }
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};