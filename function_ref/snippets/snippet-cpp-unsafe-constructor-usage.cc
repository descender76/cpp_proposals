int main()
{
    cat c;
    dog d;
    human h;
    airplane p;
    // member function with type erasure usecase
    function_ref_prime<void(void)> frp = {&c, [](void* user_data){((cat*)user_data)->walk();}};
    frp();
    frp = {&d, [](void* user_data){((dog*)user_data)->walk();}};
    frp();
    frp = {&h, [](void* user_data){((human*)user_data)->run();}};
    frp();
    frp = {&p, [](void* user_data){((airplane*)user_data)->fly();}};
    frp();
    // function with type erasure usecase
    //
    //
    frp = {&c, [](void* user_data){walking_cat((cat*)user_data);}};
    frp();
    //
    frp = {&d, [](void* user_data){walking_dog((dog*)user_data);}};
    frp();
    //
    frp = {&h, [](void* user_data){run((human*)user_data);}};
    frp();
    //
    frp = {&p, [](void* user_data){fly((airplane*)user_data);}};
    frp();
    // function without type erasure usecase
    //
    //
    frp = {nullptr, [](void* user_data){walking_cat();}};
    frp();
    //
    frp = {nullptr, [](void* user_data){walking_dog();}};
    frp();
    //
    frp = {nullptr, [](void* user_data){run();}};
    frp();
    //
    frp = {nullptr, [](void* user_data){fly();}};
    frp();
    // member function without type erasure usecase
    // not really polymorphic like above but it is what the current function_ref is doing
    function_ref_prime<void(cat*)> frp1 = {nullptr, [](void* user_data, cat* c){c->walk();}};
    frp1(&c);
    function_ref_prime<void(dog*)> frp2 = {nullptr, [](void* user_data, dog* d){d->walk();}};
    frp2(&d);
    function_ref_prime<void(human*)> frp3 = {nullptr, [](void* user_data, human* h){h->run();}};
    frp3(&h);
    function_ref_prime<void(airplane*)> frp4 = {nullptr, [](void* user_data, airplane* p){p->fly();}};
    frp4(&p);
}