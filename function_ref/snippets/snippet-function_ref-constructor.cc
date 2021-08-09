namespace std
{
    template <typename Signature>
    class function_ref
    {
        // ...

    public:
        function_ref(const function_ref&) noexcept = default;

        template <typename F>
        function_ref(F&&);

        // ...
    };

    // ...
}