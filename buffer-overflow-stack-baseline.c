/*
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2022 Microsoft Corporation
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

// HO 28/11/2022 all single-line comments are my own annotations made for
// my own reference.
//
// Explanation of weak symbols here: https://en.wikipedia.org/wiki/Weak_symbol
// param: *buf, a char pointer
// param: idx, an int representing the index at which to insert a char
#pragma weak write_buf
void
write_buf(char *buf, size_t ix)
{
    // Set memory contents to 'b' at location (buf+ix)
    buf[ix] = 'b';
}

int
main(void)
{
    // 16-item char arrays
    char upper[0x10];
    char lower[0x10];

	// the address of the start of the upper array
	// the address of the start of the lower array
	// and the distance between the two
    printf("upper = %p, lower = %p, diff = %zx\n",
        upper, lower, (size_t)(upper - lower));

    /* Assert that these get placed how we expect */
    // And what we expect is the upper array to start immediately after the lower
    //assert((ptraddr_t)upper == (ptraddr_t)&lower[sizeof(lower)]);
    assert((size_t)upper == (size_t)&lower[sizeof(lower)]);
	// Ooh sneaky
    upper[0] = 'a';
    printf("upper[0] = %c\n", upper[0]);

    // what I think will happen here is that the 'a' will be replaced with a 'b'
    // and it will get away with it
    write_buf(lower, sizeof(lower));

    printf("upper[0] = %c\n", upper[0]);

    return 0;
}
