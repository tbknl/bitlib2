#include <iostream>
#include "bitlib2.hpp"


using std::cout;
using std::cerr;
using std::endl;


using namespace bitlib2;

typedef BitBlock<512> BB;
typedef BitVector<BB> BV;


int main(int argc, char* argv[]) {
    BV bv1;
    for (int i = 0; i < 1000000000; i += 1000) {
        bv1.set(i, 1);
    }
    int total = 0;
    for (int r = 0; r < 100; ++r) {
        bv1.set(r * 12345, 1);
        total += bv1.count();
    }
    cout << total << endl;
    return 0;
}

