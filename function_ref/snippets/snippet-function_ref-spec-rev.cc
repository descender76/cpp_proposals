//
namespace std
{
    template <typename Signature>
    class function_ref
    {
        void* erased_object;

        R(*erased_function)(void*, Args...);
        // `R`, and `Args...` are the return type, and the parameter-type-list,
        // of the function type `Signature`, respectively.

    public:
        function_ref(void* obj_, R (*callback_)(void*,Args...)) noexcept : erased_object{obj_}, erased_function{callback_} {}
        function_ref(const function_ref&) noexcept = default;
// ...