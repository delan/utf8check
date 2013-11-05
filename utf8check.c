#include "utf8check.h"

#define ERROR(error) do {\
		fprintf(stderr, "%zu: %s\n", state->offset, errors[error]);\
		state->needed = 0;\
		continue;\
	} while (0)

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
