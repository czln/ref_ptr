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



int main() {
    ref_ptr<test<int>> p(new test<int>(1) );
    ref_ptr<test<int>[]> pa = new test<int> [2];
    ref_ptr<uint8_t> u = make_ref(new uint8_t(1));
    // ref_ptr<uint8_t []> ar
    *u = 1;
    // ++*u;
    std::cout<<p->get()<<std::endl;
    // auto a = pa[0].get();
    printf("%d\n", *u);
    return 0;
}