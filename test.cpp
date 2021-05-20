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
    ref_ptr<int[]> pa = new int [2];
    ref_ptr<int[]> pb = std::move(ref_ptr<int[]>((new int [2])));;
    ref_ptr<int[]> pc(pa);
    pb[0] = pa[0] = 0;
    pb[1] = pa[1] = 1;
    std::cout << pb[1] <<std::endl;
    return 0;
}