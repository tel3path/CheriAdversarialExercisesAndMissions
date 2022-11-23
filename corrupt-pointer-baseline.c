/*
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2022 Microsoft Corporation
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __CHERI_PURE_CAPABILITY__
#define PRINTF_PTR "#p"
#else
#define PRINTF_PTR "p"
#endif

// Single-line comments are added by me, for my own
// comprehension - HO, 23/11/2022

int
main(void)
{
    // 511-character string
    char buf[0x1FF];

    // a char pointer
    // or a char array with as many items as there are bytes in a char
    volatile union {
        char *ptr;
        char bytes[sizeof(char*)];
    } p;

    // go through the 511-character array and number each one 
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = i;
    }
    // address of item 271:
    // assign to the char-pointer member of the union
    p.ptr = &buf[0x10F];

    // Nobody could accuse this hash notation of being well documented
    // The pointer dereferenced value is going to be the same as its address
    // So we have the address of the start of the buffer
    // And we have the address of the volatile union
	// Vanilla:
	// buffer starts at 0x7ffeeebeb4b0
	// volatile union is at 0x7ffeeebeb4a0
	// The union starts 16 bytes before the buffer
	// CHERI:
	// buffer is at 0xfffffff7fd6c [rwRW,0xfffffff7fd6c-0xfffffff7ff6b]
	// volatile union is at 0xfffffff7fd50 [rwRW,0xfffffff7fd50-0xfffffff7fd60]
	// buffer is 511 wide, union is 16 wide
	// The union starts 28 before the buffer
	// The union ends 12 before the buffer
	// So they're not overlapping
    printf("buf=%" PRINTF_PTR " &p=%" PRINTF_PTR "\n", buf, &p);
    // Then you get the address of the char pointer member of the volatile union.
    // Then you subtract the address of the buffer start from it
    // (showing how far it is into the buffer).
    // And then you get the value of the volatile union char pointer member,
    // which previously you set to be the address of the buffer start,
    // and you display the most significant bits of that address.
    // Vanilla:
    // address of the char pointer member of the volatile union (p.ptr) = 0x7ffeeebeb5bf
    // minus the address of the buffer start (p.ptr - buf) = 10f (271, the previously chosen index of buf)
    // (these should be the same, since you set the latter to the former previously)
    // and then show the most significant byte (0f = 15) of the value of the former (which, bamboozlingly, is an address)
    // CHERI: 
    // p.ptr = 0xfffffff7fe7b [rwRW,0xfffffff7fd6c-0xfffffff7ff6b] 
    // (511 wide as expected and p.ptr is at position 271 within that)
    // p.ptr - buf in %zx = 10f (271, the chosen one from buf)
    // In other words, the pointer capability and the buffer have the same
    // starting address
    // *p.ptr in %02x = 0f (15) 
    // so what just happened - we originally gave p.ptr the address of index 271 of buf
    // and then we subtract the address of where the buffer starts,
    // from the address of index 271
    // and in CHERI we find that the buffer and the pointer capability have the same start address
    // and then we show the most significant byte of the value of p.ptr (index 271) which is its address
    printf("p.ptr=%" PRINTF_PTR " (0x%zx into buf) *p.ptr=%02x\n",
        p.ptr, p.ptr - buf, *p.ptr);

    /* One way to align the address down */
    // Take the one's complement of 255 (always (-n+1) so -256)
    // Bitwise-and it with address of the volatile union's char pointer member,
    // which, when you left it, would be the address of index 271 of the buffer,
    // and which you have cast to a uintptr_t
    // (there's no mnemonic to guess the result of a bitwise-and??)
    // and you cast the result to a char*
    // and you give its value^h^h^h^h^h address to a new char *q
    char *q = (char*)(((uintptr_t)p.ptr) & ~0xFF);
    // Vanilla:
    // Address of q = 0x7ffeeebeb500 
    // Address of p.ptr = 0x7ffeeebeb5bf (so q is 191 before p.ptr)
    // BUT I can't trust my numbers, they keep coming out differently
    // Address of q - start address of buf = 0x50 (80) 
    // This adds up, don't overthink it
    // so q is 80 after index 271 of buffer IFF that hasn't changed?
    // CHERI:
    // Address of q = 0xfffffff7fe00 
    // Address of p.ptr = 0xfffffff7fe7b (so q is 123 before p.ptr)
    // Address of q - start address of buf = 0x94 (148) 
    // This also adds up, don't overthink it
    // so q is 148 after index 271 of buffer IFF that hasn't changed?
    printf("q=%" PRINTF_PTR " (0x%zx into buf)\n", q, q - buf);
	// as before - vanilla: 0x50, cheri: 0x94
    printf("*q=%02x\n", *q);

    /* Maybe another, assuming a little-endian machine. */
    // so we take the char array member of the volatile union, 
    // and we give its first index a zero value
    // (but in the tutorial it's set to 5?)
    p.bytes[0] = 0;
    // and then we give the address of p.ptr to a new pointer
    char *r = p.ptr;
	// address of r, address of r minus start address of buf
	// Vanilla:
	// 0x7ffeeebeb500, 0x50 (same address as q... wait what?!?)
	// because here: char *q = (char*)(((uintptr_t)p.ptr) & ~0xFF);
	// we gave its address cast to a char*, not its value
	// I misread the notation
	// CHERI:
	// 0xfffffff7fe00 [rwRW,0xfffffff7fd6c-0xfffffff7ff6b] (invalid), 0x94
	// also exactly the same as q
	// Invalid, huh?
	// Surely we are little-endian? readelf says yes
    printf("r=%" PRINTF_PTR " (0x%zx)\n", r, r - buf);
    // as before - vanilla: 0x50, cheri: 0x94
    printf("*r=%02x\n", *r);

    return 0;
}
