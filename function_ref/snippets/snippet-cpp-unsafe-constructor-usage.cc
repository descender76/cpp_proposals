#include <iostream>

using namespace std;

template <typename TSignature>
class function_ref_prime;

template <typename TReturn, typename... TArgs>
class function_ref_prime<TReturn(TArgs...)> final
{
private:
    using signature_type = TReturn(void*, TArgs...);

    void* _ptr;
    TReturn (*_erased_fn)(void*, TArgs...);

public:
    function_ref_prime(void* __ptr, TReturn (*__erased_fn)(void*, TArgs...)) noexcept : _ptr{__ptr}, _erased_fn{__erased_fn}
    {
    }

    decltype(auto) operator()(TArgs... xs) const
        noexcept(noexcept(_erased_fn(_ptr, std::forward<TArgs>(xs)...)))
    {
        return _erased_fn(_ptr, std::forward<TArgs>(xs)...);
    }
};

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

int main()
{
    cat c;
    dog d;
    human h;
    airplane p;

    function_ref_prime<void(void)> frp = {&c, [](void* user_data){((cat*)user_data)->walk();}};// member function with type erasure usecase
    frp();
    frp = {&d, [](void* user_data){((dog*)user_data)->walk();}};// member function with type erasure usecase
    frp();
    frp = {&h, [](void* user_data){((human*)user_data)->run();}};// member function with type erasure usecase
    frp();
    frp = {&p, [](void* user_data){((airplane*)user_data)->fly();}};// member function with type erasure usecase
    frp();

    frp = {&c, [](void* user_data){walking_cat((cat*)user_data);}};// function with type erasure usecase
    frp();
    frp = {&d, [](void* user_data){walking_dog((dog*)user_data);}};// function with type erasure usecase
    frp();
    frp = {&h, [](void* user_data){run((human*)user_data);}};// function with type erasure usecase
    frp();
    frp = {&p, [](void* user_data){fly((airplane*)user_data);}};// function with type erasure usecase
    frp();

    frp = {&c, [](void* user_data){walking_cat();}};// function without type erasure usecase
    frp();
    frp = {&d, [](void* user_data){walking_dog();}};// function without type erasure usecase
    frp();
    frp = {&h, [](void* user_data){run();}};// function without type erasure usecase
    frp();
    frp = {&p, [](void* user_data){fly();}};// function without type erasure usecase
    frp();

    // member function without type erasure usecase
    // not really polymorphic like above but it is what the current function_ref is doing
    function_ref_prime<void(cat*)> frp1 = {&c, [](void* user_data, cat* c){c->walk();}};
    frp1(&c);
    function_ref_prime<void(dog*)> frp2 = {&d, [](void* user_data, dog* d){d->walk();}};
    frp2(&d);
    function_ref_prime<void(human*)> frp3 = {&h, [](void* user_data, human* h){h->run();}};
    frp3(&h);
    function_ref_prime<void(airplane*)> frp4 = {&p, [](void* user_data, airplane* p){p->fly();}};
    frp4(&p);
}