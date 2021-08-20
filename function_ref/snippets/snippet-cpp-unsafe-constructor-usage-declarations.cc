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

void walking_cat(cat*)
{
    std::cout << "a cat walks\n";
}

void walking_dog(dog*)
{
    std::cout << "a dog walks\n";
}

void run(human*)
{
    std::cout << "a human runs\n";
}

void fly(airplane*)
{
    std::cout << "an airplane flies\n";
}

void walking_cat()
{
    std::cout << "a cat walks\n";
}

void walking_dog()
{
    std::cout << "a dog walks\n";
}

void run()
{
    std::cout << "a human runs\n";
}

void fly()
{
    std::cout << "an airplane flies\n";
}