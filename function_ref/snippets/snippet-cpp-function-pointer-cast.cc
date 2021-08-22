#include <iostream>

using namespace std;

struct void_pointers
{
    void* first;
    void* second;
};

struct int_pointers
{
    int* first;
    int* second;
};

double work(int* to_be_typed_erased, bool b, int i, float f, double d)
{
    return *to_be_typed_erased + i + f + d;
}

typedef double (*workptr)(int* to_be_typed_erased, bool b, int i, float f, double d);
typedef double (*type_erased_work)(void* to_be_typed_erased, bool b, int i, float f, double d);

// undefined behavior
type_erased_work convert(workptr wp) {
    // function pointer can be explicitly cast to void* 
    void* temp = (void*)wp;
    return (type_erased_work)temp;
}

int main()
{
    int i = 0;
    int* ip = &i;
    // ANY non const pointer implicitly castd to void pointer
    void* vp = ip;
    const int* cip = &i;
    // const pointers are not implicitly cast to void pointer
    //vp = cip;
    // const pointers can be explicitly cast to void pointer
    vp = (void*)cip;
    int& ir = i;
    // references are not implicitly cast to void pointer
    //vp = ir;
    // references can not be explicitly cast to void pointer
    //vp = (void*)ir;
    // references can be explicitly cast to void pointer via &
    vp = &ir;
    const int& cir = i;
    // const references are not implicitly cast to void pointer
    //vp = cir;
    // const references are not implicitly cast to void pointer
    //vp = (void*)cir;
    // const references can be explicitly cast to void pointer via & and const_cast
    vp = &const_cast<int&>(cir);
    int_pointers ips{&i, &i};
    void_pointers vps{&i, &i};
    // aggregates of pointers can't be cast to aggregates of void pointers
    // even though they both can be initialized by the same set of values
    //vps = ips;
    std::cout << work(&i, true, 1, 2, 3) << std::endl;// 6
    workptr wp = work;
    // function pointers that contain pointers
    // can't be cast to function pointers with void pointers
    //type_erased_work tewp = wp;
    // function pointers that contain pointers can be
    // explicitly converted to function pointers with void pointers
    // though this is undefined behavior that could be easily defined
    type_erased_work tewp = convert(wp);
    std::cout << wp(&i, true, 1, 2, 3) << std::endl;// 6
    std::cout << convert(work)(&i, true, 1, 2, 3) << std::endl;// 6
    std::cout << convert(wp)(&i, true, 1, 2, 3) << std::endl;// 6
    // this works in GCC and clang
}
