struct delegateTest0
{
    long operator()(int i, long l, char c) const
    {
        return 0;
    }
    long operator()(int i, long l, char c)
    {
        return 0;
    }
};

struct delegateTest
{
    int state = 0;
    long operator()(int i, long l, char c) const
    {
        return state + i + l;
    }
    long operator()(int i, long l, char c)
    {
        state += i;
        return state + l;
    }
};

int main()
{
    delegateTest0 dt0;
    delegateTest dt;
    function_ref_prime<long(int, long, char)> frp = call_operator<long(int, long, char)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    frp = call_operator<long(int, long, char)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = call_operator<long(int, long, char)>::make_function_ref(dt0);
    std::cout << frp(7800, 91, 'l') << std::endl;// 0
    auto l = [&dt, &dt0](int i, long l, char c){
        return dt(i, l, c) + dt0(i, l, c);
    };
    frp = call_operator<long(int, long, char)>::make_function_ref(l);
    std::cout << frp(7800, 91, 'l') << std::endl;// 23491
}