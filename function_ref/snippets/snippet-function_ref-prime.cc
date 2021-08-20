#include <iostream>

using namespace std;

template <typename TSignature>
class function_ref_prime;

template <typename TReturn, typename... TArgs>
class function_ref_prime<TReturn(TArgs...)> final
{
private:
    using signature_type = TReturn(void*, TArgs...);

    void* _ptr;
    TReturn (*_erased_fn)(void*, TArgs...);

public:
    function_ref_prime(void* __ptr, TReturn (*__erased_fn)(void*, TArgs...)) noexcept : _ptr{__ptr}, _erased_fn{__erased_fn}
    {
    }

    decltype(auto) operator()(TArgs... xs) const
        noexcept(noexcept(_erased_fn(_ptr, std::forward<TArgs>(xs)...)))
    {
        return _erased_fn(_ptr, std::forward<TArgs>(xs)...);
    }
};
