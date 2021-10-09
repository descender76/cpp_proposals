#include <iostream>

struct fully_typed {
    bool b1;
    bool* b2;
    int i1;
    int* i2;
    float f1;
    float* f2;
};

// This class is intended by the programmer to have
// the same size, alignment and layout as fully_typed
struct partially_typed {
    bool b1;
    void* b2;
    int i1;
    void* i2;
    float f1;
    void* f2;
};

template<class B, class I, class F>
struct templated_typed {
    bool b1;
    B* b2;
    int i1;
    I* i2;
    float f1;
    F* f2;
};

void fully_typed_f1(fully_typed* ft)
{
    std::cout << ft->b1 << "," << *ft->b2
        << "," << ft->i1 << "," << *ft->i2
        << "," << ft->f1 << "," << *ft->f2
        << std::endl;
}

void fully_typed_f2(templated_typed<bool, int, float>* ft)
{
    std::cout << ft->b1 << "," << *ft->b2
        << "," << ft->i1 << "," << *ft->i2
        << "," << ft->f1 << "," << *ft->f2
        << std::endl;
}

void fully_typed_f3(bool b1, bool* b2, bool& b3,
    int i1, int* i2, int& i3,
    float f1, float* f2, float& f3)
{
    std::cout << b1 << "," << *b2 << "," << b3
        << "," << i1 << "," << *i2 << "," << i3
        << "," << f1 << "," << *f2 << "," << f3
        << std::endl;
}

typedef void (*strongfs)(bool, bool*, bool&, int, int*, int&, float, float*, float&);
typedef void (*weakfs)(bool, void*, void*, int, void*, void*, float, void*, void*);

int main()
{
    void* temp = nullptr;
    bool b = true;
    int i = 7;
    float f = 3.14;
    bool& br = b;
    int& ir = i;
    float& fr = f;
    fully_typed ft{b, &b, i, &i, f, &f};
    partially_typed pt{b, &b, i, &i, f, &f};
    templated_typed<bool, int, float> tt1{b, &b, i, &i, f, &f};
    templated_typed<void, void, void> tt2{b, &b, i, &i, f, &f};
    temp = &b;// current behavior in simple void* case
    temp = &i;// current behavior in simple void* case
    temp = &f;// current behavior in simple void* case
    //error: incompatible integer to pointer conversion assigning to 'void *' from 'bool'
    //temp = br;// desired behavior
    //error: incompatible integer to pointer conversion assigning to 'void *' from 'int'
    //temp = ir;// desired behavior
    //error: assigning to 'void *' from incompatible type 'float'
    //temp = fr;// desired behavior
    // workaround
    temp = &br;// desired behavior
    temp = &ir;// desired behavior
    temp = &fr;// desired behavior
    fully_typed_f1(&ft);
    // error: static_cast from 'partially_typed *' to 'fully_typed *', which are not related by inheritance, is not allowed
    // https://en.cppreference.com/w/c/language/type
    // "Note: C++ has no concept of compatible types. A C program that declares two types that are compatible but not identical in different translation units is not a valid C++ program."
    // fully_typed is not related partially_typed, so without compatible types a C++ programmer could not expect this to work
    //fully_typed_f(static_cast<fully_typed*>(&pt));
    // workaround
    fully_typed_f1(static_cast<fully_typed*>(static_cast<void*>(&pt)));// definable undefined behavior
    fully_typed_f2(&tt1);
    // error: static_cast from 'templated_typed<void, void, void> *' to 'templated_typed<bool, int, float> *', which are not related by inheritance, is not allowed
    // https://en.cppreference.com/w/c/language/type
    // "Note: C++ has no concept of compatible types. A C program that declares two types that are compatible but not identical in different translation units is not a valid C++ program."
    // templated_typed<bool, int, float> is related templated_typed<void, void, void>, so despite not having compatible types a C++ programmer could expect this to work
    //fully_typed_f(static_cast<templated_typed<bool, int, float>*>(&tt2));
    // workaround
    fully_typed_f2(static_cast<templated_typed<bool, int, float>*>(static_cast<void*>(&tt2)));// definable undefined behavior
    fully_typed_f3(b, &b, b, i, &i, i, f, &f, f);
    strongfs fp1 = fully_typed_f3;
    fp1(b, &b, b, i, &i, i, f, &f, f);
    // error: address of overloaded function 'fully_typed_f' does not match required type 'void (bool, void *, void *, int, void *, void *, float, void *, void *)'
    // desired behavior
    //weakfs fp2 = fully_typed_f3;
    // workaround
    weakfs fp2 = (weakfs)((void*)fully_typed_f3);
    fp2(b, &b, &b, i, &i, &i, f, &f, &f);// definable undefined behavior
}