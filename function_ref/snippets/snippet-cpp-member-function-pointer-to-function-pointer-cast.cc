struct delegateTest
{
    int state = 0;
    long run(int i, long l, char c) const
    {
        return state + i + l;
    }
    long run(int i, long l, char c)
    {
        state += i;
        return state + l;
    }
};

int main()
{
    long (delegateTest::*cp1)(int i, long l, char c) const = &delegateTest::run;
    long (delegateTest::*p1)(int i, long l, char c) = &delegateTest::run;
    auto cp2 = static_cast<long (delegateTest::*)(int i, long l, char c) const>(&delegateTest::run);
    auto p2 = static_cast<long (delegateTest::*)(int i, long l, char c)>(&delegateTest::run);
    // The following is proposed to be supported in C++
    auto cp3 = static_cast<long (*)(const delegateTest* dt, int i, long l, char c) const>(static_cast<long (delegateTest::*)(int i, long l, char c) const>(&delegateTest::run));
    auto p3 = static_cast<long (*)(delegateTest* dt, int i, long l, char c)>(static_cast<long (delegateTest::*)(int i, long l, char c)>(&delegateTest::run));
    auto cp4 = static_cast<long (*)(const delegateTest* dt, int i, long l, char c) const>(&delegateTest::run);
    auto p4 = static_cast<long (*)(delegateTest* dt, int i, long l, char c)>(&delegateTest::run);
    // and/or
    auto cp5 = static_cast<long (*)(const delegateTest& dt, int i, long l, char c) const>(static_cast<long (delegateTest::*)(int i, long l, char c) const>(&delegateTest::run));
    auto p5 = static_cast<long (*)(delegateTest& dt, int i, long l, char c)>(static_cast<long (delegateTest::*)(int i, long l, char c)>(&delegateTest::run));
    auto cp6 = static_cast<long (*)(const delegateTest& dt, int i, long l, char c) const>(&delegateTest::run);
    auto p6 = static_cast<long (*)(delegateTest& dt, int i, long l, char c)>(&delegateTest::run);
}
