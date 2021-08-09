// https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/using-delegates

delegate int int_return_int(int);

class something {
public:
    int func(int x) {
        return 0;
    };
};

int main(int argc, char * argv[]) {
    something s;

    int_return_int c = s.func;
    c(3);

    return 0;
}