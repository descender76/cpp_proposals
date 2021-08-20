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

long tef(delegateTest& dt, int i, long l, char c)
{
    return dt.test(i, l, c);
}

long tef(const delegateTest& dt, int i, long l, char c)
{
    return dt.current(i, l, c);
}

long unique_name(delegateTest& dt, int i, long l, char c)
{
    return dt.test(i, l, c);
}

long unique_name_const(const delegateTest& dt, int i, long l, char c)
{
    return dt.current(i, l, c);
}

int main()
{
    delegateTest dt;
    // explicit cast because function name is overloaded
    function_ref_prime<long(int, long, char)> frp = type_erase_first<static_cast<long(*)(delegateTest& dt, int i, long l, char c)>(tef)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    // explicit cast because function name is overloaded
    frp = type_erase_first<static_cast<long(*)(const delegateTest& dt, int i, long l, char c)>(tef)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = type_erase_first<unique_name>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = type_erase_first<unique_name_const>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 23491
}