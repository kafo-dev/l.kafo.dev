#include "maths.h"

size_t maths_size_t_max(size_t a, size_t b) {
	if (a > b) {
		return a;
	}

	return b;
}

size_t maths_size_t_min(size_t a, size_t b) {
	if (a < b) {
		return a;
	}

	return b;
}
