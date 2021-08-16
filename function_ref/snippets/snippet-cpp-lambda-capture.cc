#include <iostream>

struct cat {
    void walk()
    {
        std::cout << "a cat walks\n";
    }
};

struct dog {
    void walk()
    {
        std::cout << "a dog walks\n";
    }
};

struct human {
    void run()
    {
        std::cout << "a human runs\n";
    }
};

struct airplane {
    void fly()
    {
        std::cout << "an airplane flies\n";
    }
};

int main()
{
    cat c;
    auto f1 = [&c]() {c.walk();};
    f1();
    dog d;
    auto f2 = [&d]() {d.walk();};
    f2();
    human h;
    auto f3 = [&h]() {h.run();};
    f3();
    airplane p;
    auto f4 = [&p]() {p.fly();};
    f4();
}