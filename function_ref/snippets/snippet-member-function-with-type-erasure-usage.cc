struct delegateTest0
{
    long current(int i, long l, char c) const
    {
        return 0;
    }
    long test(int i, long l, char c)
    {
        return 0;
    }
};

struct delegateTest
{
    int state = 0;
    long current(int i, long l, char c) const
    {
        return state + i + l;
    }
    long test(int i, long l, char c)
    {
        state += i;
        return state + l;
    }
};

int main()
{
    delegateTest0 dt0;
    delegateTest dt;
    function_ref_prime<long(int, long, char)> frp = member_function_pointer<&delegateTest::test>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    frp = member_function_pointer<&delegateTest::current>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = member_function_pointer<&delegateTest0::current>::make_function_ref(dt0);
    std::cout << frp(7800, 91, 'l') << std::endl;// 0
}
