#pragma once

#ifdef ZIMG_X86

#ifndef ZIMG_X86UTIL_H_
#define ZIMG_X86UTIL_H_

#include <cstdint>
#include "ccdep.h"

namespace zimg {

extern const uint8_t xmm_mask_table_l alignas(16)[17][16];
extern const uint8_t xmm_mask_table_r alignas(16)[17][16];

extern const uint8_t ymm_mask_table_l alignas(32)[33][32];
extern const uint8_t ymm_mask_table_r alignas(32)[33][32];

#ifdef HAVE_CPU_SSE

// Store the right-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm_store_left_ps(float *dst, __m128 x, unsigned count)
{
	__m128 orig = _mm_load_ps(dst);
	__m128 mask = _mm_load_ps((const float *)(&xmm_mask_table_l[count]));

	orig = _mm_andnot_ps(mask, orig);
	x = _mm_and_ps(mask, x);
	x = _mm_or_ps(x, orig);

	_mm_store_ps(dst, x);
}

// Store the left-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm_store_right_ps(float *dst, __m128 x, unsigned count)
{
	__m128 orig = _mm_load_ps(dst);
	__m128 mask = _mm_load_ps((const float *)(&xmm_mask_table_r[count]));

	orig = _mm_andnot_ps(mask, orig);
	x = _mm_and_ps(mask, x);
	x = _mm_or_ps(x, orig);

	_mm_store_ps(dst, x);
}

#endif // HAVE_CPU_SSE

#ifdef HAVE_CPU_SSE2

// Store the right-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm_store_left_si128(__m128i *dst, __m128i x, unsigned count)
{
	__m128i orig = _mm_load_si128(dst);
	__m128i mask = _mm_load_si128((const __m128i *)(&xmm_mask_table_l[count]));

	orig = _mm_andnot_si128(mask, orig);
	x = _mm_and_si128(mask, x);
	x = _mm_or_si128(x, orig);

	_mm_store_si128(dst, x);
}

// Store the left-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm_store_right_si128(__m128i *dst, __m128i x, unsigned count)
{
	__m128i orig = _mm_load_si128(dst);
	__m128i mask = _mm_load_si128((const __m128i *)(&xmm_mask_table_r[count]));

	orig = _mm_andnot_si128(mask, orig);
	x = _mm_and_si128(mask, x);
	x = _mm_or_si128(x, orig);

	_mm_store_si128(dst, x);
}

// Stores the elements of [x] into [dst0]-[dst7].
static inline FORCE_INLINE void mm_scatter_epi16(uint16_t *dst0, uint16_t *dst1, uint16_t *dst2, uint16_t *dst3,
                                                 uint16_t *dst4, uint16_t *dst5, uint16_t *dst6, uint16_t *dst7, __m128i x)
{
	*dst0 = _mm_extract_epi16(x, 0);
	*dst1 = _mm_extract_epi16(x, 1);
	*dst2 = _mm_extract_epi16(x, 2);
	*dst3 = _mm_extract_epi16(x, 3);
	*dst4 = _mm_extract_epi16(x, 4);
	*dst5 = _mm_extract_epi16(x, 5);
	*dst6 = _mm_extract_epi16(x, 6);
	*dst7 = _mm_extract_epi16(x, 7);
}

// Transpose in-place the 8x8 matrix stored in [x0]-[x7].
static inline FORCE_INLINE void mm_transpose8_epi16(__m128i &x0, __m128i &x1, __m128i &x2, __m128i &x3, __m128i &x4, __m128i &x5, __m128i &x6, __m128i &x7)
{
	__m128i t0, t1, t2, t3, t4, t5, t6, t7;
	__m128i tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7;

	t0 = _mm_unpacklo_epi16(x0, x1);
	t1 = _mm_unpacklo_epi16(x2, x3);
	t2 = _mm_unpacklo_epi16(x4, x5);
	t3 = _mm_unpacklo_epi16(x6, x7);
	t4 = _mm_unpackhi_epi16(x0, x1);
	t5 = _mm_unpackhi_epi16(x2, x3);
	t6 = _mm_unpackhi_epi16(x4, x5);
	t7 = _mm_unpackhi_epi16(x6, x7);

	tt0 = _mm_unpacklo_epi32(t0, t1);
	tt1 = _mm_unpackhi_epi32(t0, t1);
	tt2 = _mm_unpacklo_epi32(t2, t3);
	tt3 = _mm_unpackhi_epi32(t2, t3);
	tt4 = _mm_unpacklo_epi32(t4, t5);
	tt5 = _mm_unpackhi_epi32(t4, t5);
	tt6 = _mm_unpacklo_epi32(t6, t7);
	tt7 = _mm_unpackhi_epi32(t6, t7);

	x0 = _mm_unpacklo_epi64(tt0, tt2);
	x1 = _mm_unpackhi_epi64(tt0, tt2);
	x2 = _mm_unpacklo_epi64(tt1, tt3);
	x3 = _mm_unpackhi_epi64(tt1, tt3);
	x4 = _mm_unpacklo_epi64(tt4, tt6);
	x5 = _mm_unpackhi_epi64(tt4, tt6);
	x6 = _mm_unpacklo_epi64(tt5, tt7);
	x7 = _mm_unpackhi_epi64(tt5, tt7);
}

// Saturated convert signed 32-bit to unsigned 16-bit, biased by INT16_MIN.
static inline FORCE_INLINE __m128i mm_packus_epi32_bias(__m128i a, __m128i b)
{
	const __m128i i16_min_epi32 = _mm_set1_epi32(INT16_MIN);

	a = _mm_add_epi32(a, i16_min_epi32);
	b = _mm_add_epi32(b, i16_min_epi32);

	a = _mm_packs_epi32(a, b);

	return a;
}

// Saturated convert signed 32-bit to unsigned 16-bit.
static inline FORCE_INLINE __m128i mm_packus_epi32(__m128i a, __m128i b)
{
	const __m128i i16_min_epi16 = _mm_set1_epi16(INT16_MIN);

	a = mm_packus_epi32_bias(a, b);
	a = _mm_sub_epi16(a, i16_min_epi16);

	return a;
}

#endif // HAVE_CPU_SSE2

#ifdef HAVE_CPU_AVX

// Store the right-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm256_store_left_ps(float *dst, __m256 x, unsigned count)
{
	__m256 orig = _mm256_load_ps(dst);
	__m256 mask = _mm256_load_ps((const float *)(&ymm_mask_table_l[count]));

	orig = _mm256_andnot_ps(mask, orig);
	x = _mm256_and_ps(mask, x);
	x = _mm256_or_ps(x, orig);

	_mm256_store_ps(dst, x);
}

// Store the left-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm256_store_right_ps(float *dst, __m256 x, unsigned count)
{
	__m256 orig = _mm256_load_ps(dst);
	__m256 mask = _mm256_load_ps((const float *)(&ymm_mask_table_r[count]));

	orig = _mm256_andnot_ps(mask, orig);
	x = _mm256_and_ps(mask, x);
	x = _mm256_or_ps(x, orig);

	_mm256_store_ps(dst, x);
}

// Stores the elements of [x] into [dst0]-[dst7].
static inline FORCE_INLINE void mm256_scatter_ps(float *dst0, float *dst1, float *dst2, float *dst3,
                                                 float *dst4, float *dst5, float *dst6, float *dst7, __m256 y)
{
	__m128 x;

	x = _mm256_castps256_ps128(y);

	_MM_EXTRACT_FLOAT(*dst0, x, 0);
	_MM_EXTRACT_FLOAT(*dst1, x, 1);
	_MM_EXTRACT_FLOAT(*dst2, x, 2);
	_MM_EXTRACT_FLOAT(*dst3, x, 3);

	x = _mm256_castps256_ps128(_mm256_permute2f128_ps(y, y, 0x01));

	_MM_EXTRACT_FLOAT(*dst4, x, 0);
	_MM_EXTRACT_FLOAT(*dst5, x, 1);
	_MM_EXTRACT_FLOAT(*dst6, x, 2);
	_MM_EXTRACT_FLOAT(*dst7, x, 3);
}

// Transpose in-place the 8x8 matrix stored in [x0]-[x7]
static inline FORCE_INLINE void mm256_transpose8_ps(__m256 &row0, __m256 &row1, __m256 &row2, __m256 &row3, __m256 &row4, __m256 &row5, __m256 &row6, __m256 &row7)
{
	__m256 t0, t1, t2, t3, t4, t5, t6, t7;
	__m256 tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7;
	t0 = _mm256_unpacklo_ps(row0, row1);
	t1 = _mm256_unpackhi_ps(row0, row1);
	t2 = _mm256_unpacklo_ps(row2, row3);
	t3 = _mm256_unpackhi_ps(row2, row3);
	t4 = _mm256_unpacklo_ps(row4, row5);
	t5 = _mm256_unpackhi_ps(row4, row5);
	t6 = _mm256_unpacklo_ps(row6, row7);
	t7 = _mm256_unpackhi_ps(row6, row7);
	tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
	tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
	tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
	tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
	tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
	tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
	tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
	tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));
	row0 = _mm256_permute2f128_ps(tt0, tt4, 0x20);
	row1 = _mm256_permute2f128_ps(tt1, tt5, 0x20);
	row2 = _mm256_permute2f128_ps(tt2, tt6, 0x20);
	row3 = _mm256_permute2f128_ps(tt3, tt7, 0x20);
	row4 = _mm256_permute2f128_ps(tt0, tt4, 0x31);
	row5 = _mm256_permute2f128_ps(tt1, tt5, 0x31);
	row6 = _mm256_permute2f128_ps(tt2, tt6, 0x31);
	row7 = _mm256_permute2f128_ps(tt3, tt7, 0x31);
}

#endif // HAVE_CPU_AVX

#ifdef HAVE_CPU_AVX2

// Store the right-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm256_store_left_si256(__m256i *dst, __m256i x, unsigned count)
{
	__m256i orig = _mm256_load_si256(dst);
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table_l[count]));

	orig = _mm256_andnot_si256(mask, orig);
	x = _mm256_and_si256(mask, x);
	x = _mm256_or_si256(x, orig);

	_mm256_store_si256(dst, x);
}

// Store the left-most [count] bytes from [x] into [dst].
static inline FORCE_INLINE void mm256_store_right_si256(__m256i *dst, __m256i x, unsigned count)
{
	__m256i orig = _mm256_load_si256(dst);
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table_r[count]));

	orig = _mm256_andnot_si256(mask, orig);
	x = _mm256_and_si256(mask, x);
	x = _mm256_or_si256(x, orig);

	_mm256_store_si256(dst, x);
}

#endif // HAVE_CPU_AVX2

} // namespace zimg

#endif // ZIMG_X86UTIL_H_

#endif // ZIMG_X86
