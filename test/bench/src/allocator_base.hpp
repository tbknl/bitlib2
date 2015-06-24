#ifndef _ALLOCATOR_BASE_HPP_
#define _ALLOCATOR_BASE_HPP_

int doSomeAllocations(int offset) {
    BitVector bv1, bv2, bv3;

    for (int i = offset; i < 1000000000; i += 8263) {
        bv1.set(i, 1);
    }

    bv2 = bv1;

    for (int i = offset + 1; i < 1000000000; i += 3997) {
        bv2.set(i, 1);
    }

    bv3 = bv2;
    bv3.bitXor(bv1);

    return bv3.count();
}

int doManyAllocations() {
    int total = 0;
    for (int r = 0; r < 20; ++r) {
        total += doSomeAllocations(2 + r);
    }
    return total;
}

#endif // _ALLOCATOR_BASE_HPP_
