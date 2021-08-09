namespace std
{
    template <typename Signature>
    class function_ref
    {
        void* erased_object; // exposition only

        R(*erased_function)(Args...); // exposition only
        // `R`, and `Args...` are the return type, and the parameter-type-list,
        // of the function type `Signature`, respectively.

        // ...
    };

    // ...
}