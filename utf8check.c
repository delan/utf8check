#include <stdio.h>
#include <stdint.h>

#define ERROR(error) do {\
		fprintf(stderr, "%zu: %s\n", state->offset, errors[error]);\
		state->needed = 0;\
		continue;\
	} while (0)

struct parser_state {
	size_t offset;
	int needed;
};

char *errors[] = {
	"invalid byte (0xfe or 0xff)",
	"unexpected continuation byte; ASCII or start byte expected",
	"unexpected ASCII byte; continuation byte expected",
	"unexpected start byte; continuation byte expected",
	"invalid codepoint",
	"overlong sequence",
	"unexpected EOF while waiting for a continuation byte",
};

int byte_type[256] = {
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

uint32_t initial_cp[256] = {
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

uint32_t min_cp[6] = { 0, 0x80, 0x800, 0x10000, 0x200000, 0x4000000 };

void putchar8(uint32_t c) {
	if (c < 0x80) {
		putchar(c);
	} else if (c < 0x800) {
		putchar((c >> 6) | 0xc0);
		putchar((c & 0x3f) | 0x80);
	} else if (c < 0x10000) {
		putchar((c >> 12) | 0xe0);
		putchar((c >> 6 & 0x3f) | 0x80);
		putchar((c & 0x3f) | 0x80);
	} else {
		putchar((c >> 18) | 0xf0);
		putchar((c >> 12 & 0x3f) | 0x80);
		putchar((c >> 6 & 0x3f) | 0x80);
		putchar((c & 0x3f) | 0x80);
	}
}

void parse_block(struct parser_state *state, unsigned char *buf,
	size_t len) {
	int needed_start;
	uint32_t cp;
	size_t i;
	for (i = 0; i < len; i++) {
		if (byte_type[buf[i]] == 7)
			ERROR(0);
		switch (state->needed) {
		case 0:
			if (byte_type[buf[i]] == 1)
				ERROR(1);
			else if (byte_type[buf[i]] != 0) {
				state->needed = byte_type[buf[i]] - 1;
				needed_start = state->needed;
				cp = initial_cp[buf[i]];
			}
			break;
		case 1:
			if (byte_type[buf[i]] == 0)
				ERROR(2);
			else if (byte_type[buf[i]] > 1)
				ERROR(3);
			else {
				cp |= buf[i] & 0x3f;
				state->needed = 0;
				if (
					(cp > 0x110000) ||
					((cp >= 0xfdd0) && (cp <= 0xfdef)) ||
					((cp & 0xfffe) == 0xfffe) ||
					((cp & 0xfffff800) == 0xd800)
				)
					ERROR(4);
				if (cp < min_cp[needed_start])
					ERROR(5);
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			if (byte_type[buf[i]] == 0)
				ERROR(2);
			else if (byte_type[buf[i]] > 1)
				ERROR(3);
			else
				cp |= (buf[i] & 0x3f) <<
					(--(state->needed) * 6);
			break;
		}
		state->offset++;
	}
}

int main(void) {
	size_t bufread;
	unsigned char buf[4096];
	struct parser_state state = { 0 };
	while ((bufread = fread(buf, 1, 4096, stdin)))
		parse_block(&state, buf, bufread);
	if (state.needed)
		fprintf(stderr, "%zu: %s\n", state.offset, errors[6]);
	return 0;
}
