template <class F> class function_ref;

/// Specialization for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  template<class I, auto MFP>
  function_ref(I& instance, internal_member_function_traits<decltype(MFP), MFP>) noexcept
    : obj_(&instance), callback_(internal_member_function_traits<decltype(MFP), MFP>::type_erase_this) {
  }
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};