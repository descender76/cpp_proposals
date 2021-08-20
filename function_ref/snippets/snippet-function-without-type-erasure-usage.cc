long one(int i, long l, char c)
{
    return 1;
}

int main()
{
    function_ref_prime<long(int, long, char)> frp = prepend_void_pointer<&one>::make_function_ref();
    std::cout << frp(7800, 91, 'l') << std::endl;// 1
}
