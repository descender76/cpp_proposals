// http://docwiki.embarcadero.com/RADStudio/Sydney/en/Closure

class something {
public:
    int func(int x) {
        return 0;
    };
};

int main(int argc, char * argv[]) {
    something s;
    void(__closure * c)(int);

    c = s.func;
    c(3);

    return 0;
}