#pragma once

#include "cstdef.h"

// Fixed width unsigned integers
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// Fixed width signed integers
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

// Fixed width binary floats
typedef float float32_t;
typedef double float64_t;
typedef long double float128_t; // Varies depending on compiler; may be 80-bit or 128-bit

// Fixed width decimal floats
#if (C_STANDARD >= 2023)
	typedef _Decimal32 dec32_t;
	typedef _Decimal64 dec64_t;
	typedef _Decimal128 dec128_t;
#endif

// Fixed width unsigned integer limits
#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 0xffffffffU /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */

// Fixed width signed integer limits
#define INT8_MIN  (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)

#define INT8_MAX  (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807LL)