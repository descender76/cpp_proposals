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
        // factory function to be used by make_function_ref and other user defined construction methods
        static function_ref<R(Args...)> construct_from_type_erased(void* obj_, R (*callback_)(void*,Args...)) noexcept;

        function_ref(const function_ref&) noexcept = default;

        template <typename F>
        function_ref(F&&);

        function_ref& operator=(const function_ref&) noexcept = default;

        template <typename F>
        function_ref& operator=(F&&);

        void swap(function_ref&) noexcept;

        R operator()(Args...) const noexcept(see below);
        // `R` and `Args...` are the return type and the parameter-type-list
        // of the function type `Signature`, respectively.
    };

    template <typename Signature>
    void swap(function_ref<Signature>&, function_ref<Signature>&) noexcept;
}