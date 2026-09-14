#ifndef MATHS_H
#define MATHS_H

#include <stddef.h>

size_t maths_size_t_max(size_t a, size_t b);

#define maths_max(a, b) _Generic((a), size_t: maths_size_t_max)(a, b)

size_t maths_size_t_min(size_t a, size_t b);

#define maths_min(a, b) _Generic((a), size_t: maths_size_t_min)(a, b)

#endif /* MATHS_H */
