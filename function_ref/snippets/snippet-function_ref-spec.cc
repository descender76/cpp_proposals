// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0792r5.html
namespace std
{
    template <typename Signature>
    class function_ref
    {
        void* erased_object; // exposition only

        R(*erased_function)(Args...); // exposition only
        // `R`, and `Args...` are the return type, and the parameter-type-list,
        // of the function type `Signature`, respectively.

    public:
        function_ref(const function_ref&) noexcept = default;
// ...