#include "ref_ptr.h"
#include <cstdio>
// #include <initializer_list>
using namespace ref;

template <typename T>
class test{
private:
    T *a;
public:
    test() : a(nullptr) { }
    test(T b) { a = new T(b);}
    T& get() {return *a;}
    ~test() {delete a;}

};

typedef struct {
    int a;
    char b;
} s;

void basic_test() {
    ref_ptr<int> i1(new int(1));
    if (*i1 == 1)
        printf("basic test <int> constructor pass\n");
    ref_ptr<int> i2(i1);
    if (*i2 == *i1)
        printf("basic test <int> copy constructor pass\n");
    ref_ptr<char> c2 = new char(2);
    if (*c2 == 2)
        printf("basic test <char> copy assignment pass\n");
    ref_ptr<test<double>> td(new test<double>(1.1));
    if (td->get() == 1.1)
        printf("basic test <template class> constructor pass\n");
    auto tdm = std::move(td);
    if (tdm->get() == 1.1 && !td) {
        printf("basic test <template class> move assignemnt pass\n");
    }
}

void array_test() {
    ref_ptr<int[]> i1(new int[2]{1,2});
    if (i1[0] == 1 && i1[1] == 2)
        printf("array test <int[]> constructor pass\n");
    ref_ptr<int[]> i2(i1);
    if (i2[0] == i1[0] && i2[1] == i1[1])
        printf("array test <int[]> copy constructor pass\n");
    ref_ptr<char[]> c2 = new char[1]{2};
    if (c2[0] == 2)
        printf("array test <char> copy assignment pass\n");
    ref_ptr<test<double>[]> td(new test<double>[2]{1.1, 2.2});
    if (td[0].get() == 1.1 && td[1].get() == 2.2)
        printf("array test <template class> constructor pass\n");
    auto tdm = std::move(td);
    if (tdm[0].get() == 1.1 && tdm[1].get() == 2.2 && !td) {
        printf("array test <template class> move assignemnt pass\n");
    }

}


int main() {
    basic_test();
    array_test();

    return 0;
}