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
    delegateTest dt;
    function_ref_prime<long(delegateTest*, int, long, char)> frp = this_as_pointer<&delegateTest::test>::make_function_ref();
    std::cout << frp(&dt, 7800, 91, 'l') << std::endl;// 7891
    frp = this_as_pointer<&delegateTest::current>::make_function_ref();
    std::cout << frp(&dt, 7800, 91, 'l') << std::endl;// 15691
    function_ref_prime<long(delegateTest&, int, long, char)> frp1 = this_as_ref<&delegateTest::test>::make_function_ref();
    std::cout << frp1(dt, 7800, 91, 'l') << std::endl;// 15691
    frp1 = this_as_ref<&delegateTest::current>::make_function_ref();
    std::cout << frp1(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(delegateTest, int, long, char)> frp2 = this_as_value<&delegateTest::test>::make_function_ref();
    std::cout << frp2(dt, 7800, 91, 'l') << std::endl;// 23491
    frp2 = this_as_value<&delegateTest::current>::make_function_ref();
    std::cout << frp2(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest*, int, long, char)> frp3 = this_as_cpointer<&delegateTest::current>::make_function_ref();
    std::cout << frp3(&dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest&, int, long, char)> frp4 = this_as_cref<&delegateTest::current>::make_function_ref();
    std::cout << frp4(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest, int, long, char)> frp5 = this_as_cvalue<&delegateTest::current>::make_function_ref();
    std::cout << frp5(dt, 7800, 91, 'l') << std::endl;// 23491
}
