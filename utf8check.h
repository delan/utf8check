#ifndef UTF8CHECK_H
#define UTF8CHECK_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct utf8check_state {
	size_t offset;
	int needed;
	int needed_start;
	uint32_t cp;
	enum {
		UTF8CHECK_VALIDATE,
		UTF8CHECK_INLINE,
		UTF8CHECK_SANITISE
	} mode;
};

const char *const utf8check_errors[] = {
	"invalid byte (0xFE or 0xFF)",
	"unexpected continuation byte; ASCII or start byte expected",
	"unexpected ASCII byte; continuation byte expected",
	"unexpected start byte; continuation byte expected",
	"invalid codepoint",
	"overlong sequence",
	"unexpected EOF while waiting for a continuation byte",
};

const char *const utf8check_minierrs[] = {
	"\033[31mI\033[m",
	"\033[31mC\033[m",
	"\033[31mA\033[m",
	"\033[31mS\033[m",
	"\033[31mP\033[m",
	"\033[31mO\033[m",
	"\033[31mE\033[m",
};

/*
	Classification of octet values:

	0	single-byte ASCII character
	1	continuation byte
	2	start of 2-byte sequence
	3	start of 3-byte sequence
	4	start of 4-byte sequence
	5	start of 5-byte sequence
	6	start of 6-byte sequence
	7	invalid byte (0xFE or 0xFF)
*/

const int utf8check_type[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 7
};

/* Base codepoints for starting bytes */

const uint32_t utf8check_initial[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,	0x40,	0x80,	0xc0,	0x100,	0x140,	0x180,	0x1c0,
	0x200,	0x240,	0x280,	0x2c0,	0x300,	0x340,	0x380,	0x3c0,
	0x400,	0x440,	0x480,	0x4c0,	0x500,	0x540,	0x580,	0x5c0,
	0x600,	0x640,	0x680,	0x6c0,	0x700,	0x740,	0x780,	0x7c0,
	0,	0x1000,	0x2000,	0x3000,	0x4000,	0x5000,	0x6000,	0x7000,
	0x8000,	0x9000,	0xa000,	0xb000,	0xc000,	0xd000,	0xe000,	0xf000,
	0,		0x40000,	0x80000,	0xc0000,
	0x100000,	0x140000,	0x180000,	0x1c0000,
	0,		0x1000000,	0x2000000,	0x3000000,
	0,		0x40000000,	0,		0
};

/* Minimum codepoints by sequence length for overlong sequence check */

const uint32_t utf8check_min[] = {
	0, 0x80, 0x800, 0x10000, 0x200000, 0x4000000
};

#endif
