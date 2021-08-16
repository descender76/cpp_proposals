struct cat {
    void walk(){}
};

struct dog {
    void walk(){}
};

struct human {
    void run(){}
};

struct airplane {
    void fly(){}
};

cat c;
function_ref<void(void)> f = {c, &cat::walk};
f();
dog d;
f = {d, &dog::walk};
f();
human h;
f = {h, &human::run};
f();
airplane p;
f = {p, &airplane::fly};
f();