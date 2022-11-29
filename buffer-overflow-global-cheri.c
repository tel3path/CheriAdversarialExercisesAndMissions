/*
 * SPDX-License-Identifier: BSD-2-Clause-DARPA-SSITH-ECATS-HR0011-18-C-0016
 * Copyright (c) 2020 SRI International
 */
#include <stdio.h>

char buffer[128];
//char buffer[1048577];
char c;

#pragma weak fill_buf
void
fill_buf(char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        buf[i] = 'b';
}

#include "main-asserts.inc"

int
main(void)
{
    (void)buffer;
    // Ensures that overflowing buffer by 1 will hit 'c'
    main_asserts();

    c = 'c';
    printf("c = %c\n", c);

    // so you have a 128-long string of just the letter b
    // and you overflow it by 1
    // and therefore (as main_asserts has set up) you hit c
    fill_buf(buffer, sizeof(buffer));

    // if CHERI doesn't stop it, there'll be a b in here
    printf("c = %c\n", c);

    return 0;
}
