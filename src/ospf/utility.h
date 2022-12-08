#pragma once


/*
 * XORP code uses typedefs (e.g. uint32_t, int32_t) rather than using
 * the base types, because the 'C' language allows a compiler to use a
 * natural size for base type. The XORP code is internally consistent
 * in this usage, one problem arises with format strings in printf
 * style functions.
 *
 * In order to print a size_t or uint32_t with "%u" it is necessary to
 * cast to an unsigned int. On Mac OS X a size_t is not an unsigned
 * int. On windows uint32_t is not an unsigned int.
 *
 * In order to print a int32_t with a "%d" it is necessary to cast to
 * a signed int. On windows int32_t is not a signed int.
 *
 * The two convenience macros are provided to perform the cast.
 */
#ifdef __cplusplus
#define XORP_UINT_CAST(x)   static_cast<unsigned int>(x)
#define XORP_INT_CAST(x)    static_cast<int>(x)
#else
#define XORP_UINT_CAST(x)   (unsigned int)(x)
#define XORP_INT_CAST(x)    (int)(x)
#endif


