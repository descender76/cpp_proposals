int main()
{
    cat c;
    dog d;
    human h;
    airplane p;
    // member function with type erasure usecase
    function_ref_prime<void(void)> frp = member_function_pointer<&cat::walk>::make_function_ref(c);
    frp();
    frp = member_function_pointer<&dog::walk>::make_function_ref(d);
    frp();
    frp = member_function_pointer<&human::run>::make_function_ref(h);
    frp();
    frp = member_function_pointer<&airplane::fly>::make_function_ref(p);
    frp();
    // function with type erasure usecase
    // static_cast needed only because functions are overloaded
    //frp = type_erase_first<walking_cat>::make_function_ref(&c);
    frp = type_erase_first<static_cast<void(*)(cat*)>(walking_cat)>::make_function_ref(&c);
    frp();
    //frp = type_erase_first<walking_dog>::make_function_ref(&d);
    frp = type_erase_first<static_cast<void(*)(dog*)>(walking_dog)>::make_function_ref(&d);
    frp();
    //frp = type_erase_first<run>::make_function_ref(&h);
    frp = type_erase_first<static_cast<void(*)(human*)>(run)>::make_function_ref(&h);
    frp();
    //frp = type_erase_first<fly>::make_function_ref(&p);
    frp = type_erase_first<static_cast<void(*)(airplane*)>(fly)>::make_function_ref(&p);
    frp();
    // function without type erasure usecase
    // static_cast needed only because functions are overloaded
    //frp = prepend_void_pointer<(walking_cat>::make_function_ref();
    frp = prepend_void_pointer<static_cast<void(*)()>(walking_cat)>::make_function_ref();
    frp();
    //frp = prepend_void_pointer<walking_dog>::make_function_ref();
    frp = prepend_void_pointer<static_cast<void(*)()>(walking_dog)>::make_function_ref();
    frp();
    //frp = prepend_void_pointer<run>::make_function_ref();
    frp = prepend_void_pointer<static_cast<void(*)()>(run)>::make_function_ref();
    frp();
    //frp = prepend_void_pointer<fly>::make_function_ref();
    frp = prepend_void_pointer<static_cast<void(*)()>(fly)>::make_function_ref();
    frp();
    // member function without type erasure usecase
    // not really polymorphic like above but it is what the current function_ref is doing
    function_ref_prime<void(cat*)> frp1 = this_as_pointer<&cat::walk>::make_function_ref();
    frp1(&c);
    function_ref_prime<void(dog*)> frp2 = this_as_pointer<&dog::walk>::make_function_ref();
    frp2(&d);
    function_ref_prime<void(human*)> frp3 = this_as_pointer<&human::run>::make_function_ref();
    frp3(&h);
    function_ref_prime<void(airplane*)> frp4 = this_as_pointer<&airplane::fly>::make_function_ref();
    frp4(&p);
}
