#include <iostream>

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
    bool b = true;
    int i = 7;
    float f = 3.14;
    fully_typed_f3(b, &b, b, i, &i, i, f, &f, f);
    strongfs fp1 = fully_typed_f3;
    fp1(b, &b, b, i, &i, i, f, &f, f);
    // error: address of overloaded function 'fully_typed_f' does not match required type 'void (bool, void *, void *, int, void *, void *, float, void *, void *)'
    //weakfs fp2 = fully_typed_f3;// desired behavior
    // workaround
    weakfs fp2 = (weakfs)((void*)fully_typed_f3);
    fp2(b, &b, &b, i, &i, &i, f, &f, &f);// definable undefined behavior
    // if lvalue reference could also be implicitly converted to void pointer
    //fp2(b, &b, b, i, &i, i, f, &f, f);// definable undefined behavior
}