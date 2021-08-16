//https://github.com/TartanLlama/function_ref/blob/master/include/tl/function_ref.hpp
template <class R, class... Args> class function_ref<R(Args...)> {
// ...
private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};