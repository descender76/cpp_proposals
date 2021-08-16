// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0792r5.html
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