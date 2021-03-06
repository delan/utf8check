#include "utf8check.h"

void utf8check_putchar(struct utf8check_state *state, uint32_t c) {
	if (state->mode != UTF8CHECK_VALIDATE) {
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
}

void utf8check_error(struct utf8check_state *state, int errno) {
	fflush(stdout);
	switch (state->mode) {
	case UTF8CHECK_VALIDATE:
		fprintf(stderr, "%zu: %s\n", state->offset,
			utf8check_errors[errno]);
		break;
	case UTF8CHECK_INLINE:
		fprintf(stderr, "%s", utf8check_minierrs[errno]);
		break;
	case UTF8CHECK_SANITISE:
		utf8check_putchar(state, 0xfffd);
		break;
	}
	state->needed = 0;
	state->offset++;
}

void utf8check_parse(
	struct utf8check_state *state,
	uint8_t *buf,
	size_t len
) {
	int *needed_start = &state->needed_start;
	uint32_t *cp = &state->cp;
	size_t i;
	for (i = 0; i < len; i++) {
		if (utf8check_type[buf[i]] == 7) {
			utf8check_error(state, 0);
			continue;
		}
		switch (state->needed) {
		case 0:
			if (utf8check_type[buf[i]] == 1) {
				utf8check_error(state, 1);
				continue;
			} else if (utf8check_type[buf[i]] != 0) {
				state->needed = utf8check_type[buf[i]] - 1;
				*needed_start = state->needed;
				*cp = utf8check_initial[buf[i]];
			} else if (utf8check_type[buf[i]] == 0) {
				*cp = buf[i];
				utf8check_putchar(state, *cp);
			}
			break;
		case 1:
			if (utf8check_type[buf[i]] == 0) {
				utf8check_error(state, 2);
				*cp = buf[i];
				utf8check_putchar(state, *cp);
				continue;
			} else if (utf8check_type[buf[i]] > 1) {
				utf8check_error(state, 3);
				state->needed = utf8check_type[buf[i]] - 1;
				*needed_start = state->needed;
				*cp = utf8check_initial[buf[i]];
				continue;
			} else {
				*cp |= buf[i] & 0x3f;
				state->needed = 0;
				if (
					(*cp > 0x10FFFF) ||
					((*cp >= 0xfdd0) && (*cp <= 0xfdef)) ||
					((*cp & 0xfffe) == 0xfffe) ||
					((*cp & 0xfffff800) == 0xd800)
				) {
					utf8check_error(state, 4);
					continue;
				}
				if (*cp < utf8check_min[*needed_start]) {
					utf8check_error(state, 5);
					continue;
				}
				utf8check_putchar(state, *cp);
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			if (utf8check_type[buf[i]] == 0) {
				utf8check_error(state, 2);
				*cp = buf[i];
				utf8check_putchar(state, *cp);
				continue;
			} else if (utf8check_type[buf[i]] > 1) {
				utf8check_error(state, 3);
				state->needed = utf8check_type[buf[i]] - 1;
				*needed_start = state->needed;
				*cp = utf8check_initial[buf[i]];
				continue;
			} else {
				*cp |= (buf[i] & 0x3f) <<
					(--(state->needed) * 6);
			}
			break;
		}
		state->offset++;
	}
}

int main(int argc, char **argv) {
	size_t bufread;
	unsigned char buf[4096];
	struct utf8check_state state = { 0 };
	if (argc == 2) {
		if (!strcmp(argv[1], "-i"))
			state.mode = UTF8CHECK_INLINE;
		else if (!strcmp(argv[1], "-s"))
			state.mode = UTF8CHECK_SANITISE;
	}
	while ((bufread = fread(buf, 1, 4096, stdin)))
		utf8check_parse(&state, buf, bufread);
	if (state.needed)
		utf8check_error(&state, 6);
	return 0;
}
