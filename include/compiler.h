#pragma once

#include <include/compiler_types.h>
#include <include/assert.h>

#ifndef barrier
/* The "volatile" is due to gcc bugs */
# define barrier() __asm__ __volatile__("": : :"memory")
#endif

# define __same_type(a, b) __builtin_types_compatible_p(__typeof__ (a), __typeof__ (b))

#define __scalar_type_to_expr_cases(type) \
    unsigned type:	(unsigned type)0, \
    signed type:	(signed type)0

#define __unqual_scalar_typeof(x) __typeof__( \
    _Generic((x), \
        char:	(char)0, \
        __scalar_type_to_expr_cases(char),		\
        __scalar_type_to_expr_cases(short),		\
        __scalar_type_to_expr_cases(int),		\
        __scalar_type_to_expr_cases(long),		\
        __scalar_type_to_expr_cases(long long),	\
        default: (x)))

#define __READ_ONCE(x) (*(const volatile __unqual_scalar_typeof(x) *)&(x))

#define READ_ONCE(x) \
({ \
    compiletime_assert_rwonce_type(x); \
    __READ_ONCE(x); \
})

#define __WRITE_ONCE(x, val) \
do { \
    *(volatile __typeof__ (x) *)&(x) = (val); \
} while (0)

#define WRITE_ONCE(x, val) \
do { \
    compiletime_assert_rwonce_type(x); \
    __WRITE_ONCE(x, val); \
} while (0)
