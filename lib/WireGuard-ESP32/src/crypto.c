#include "crypto.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void crypto_zero(void *dest, size_t len) {
	volatile uint8_t *p = (uint8_t *)dest;
	while (len--) {
		*p++ = 0;
	}
}

bool crypto_equal(const void *a, const void *b, size_t size) {
	uint8_t neq = 0;
	while (size > 0) {
		neq |= *(uint8_t *)a ^ *(uint8_t *)b;
		a += 1;
		b += 1;
		size -= 1;
	}
	return (neq) ? false : true;
}
