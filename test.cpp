#include "ref_ptr.h"
#include <iostream>
using namespace ref;

template <typename T>
class test{
private:
    T *a;
public:
    test() : a(nullptr) { }
    test(T b) { a = new int(b);}
    T& get() {return *a;}
    ~test() {delete a;}

};

typedef struct {
    int a;
    char b;
} s;


int main() {
    ref_ptr<int[]> pa = new int [2];
    ref_ptr<int[]> pb = std::move(ref_ptr<int[]>((new int [2])));;
    ref_ptr<int[]> pc(pa);
    pb[0] = pa[0] = 0;
    pb[1] = pa[1] = 1;
    std::cout << pb[1] <<std::endl;

    ref_ptr<int> a(new int (12));
    ref_ptr<int> b;
    if (a && !b) {
        printf("bool test ok\n");
    }
    printf("%d\n", *a);
    s sa;
    sa.a = 1;
    sa.b = 'a';
    ref_ptr<s> ps(new s(sa));
    printf("%d %c\n", ps->a, ps->b);

    int *aa = a.get();
    printf("%d\n", *aa);
    return 0;
}