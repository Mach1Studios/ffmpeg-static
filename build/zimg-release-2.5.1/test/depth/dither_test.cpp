#include "common/cpuinfo.h"
#include "common/pixel.h"
#include "graph/image_filter.h"
#include "depth/depth.h"
#include "depth/dither.h"

#include "gtest/gtest.h"
#include "graph/filter_validator.h"

namespace {

void test_case(zimg::depth::DitherType type, bool fullrange, bool chroma, const char *(*expected_sha1)[3])
{
	const unsigned w = 640;
	const unsigned h = 480;

	zimg::PixelType pixel_in[] = { zimg::PixelType::BYTE, zimg::PixelType::WORD, zimg::PixelType::HALF, zimg::PixelType::FLOAT };
	zimg::PixelType pixel_out[] = { zimg::PixelType::BYTE, zimg::PixelType::WORD };
	unsigned sha1_idx = 0;

	for (zimg::PixelType pxin : pixel_in) {
		for (zimg::PixelType pxout : pixel_out) {
			SCOPED_TRACE(static_cast<int>(pxin));
			SCOPED_TRACE(static_cast<int>(pxout));

			zimg::PixelFormat fmt_in = pxin;
			fmt_in.fullrange = fullrange;
			fmt_in.chroma = chroma;

			zimg::PixelFormat fmt_out = pxout;
			fmt_out.fullrange = fullrange;
			fmt_out.chroma = chroma;

			auto dither = zimg::depth::create_dither(type, w, h, fmt_in, fmt_out, zimg::CPUClass::NONE);

			FilterValidator validator{ dither.get(), w, h, fmt_in };
			validator.set_sha1(expected_sha1[sha1_idx++]);
			validator.validate();
		}
	}
}

} // namespace


TEST(DitherTest, test_limited_luma)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "d5794ead078fee72fd10fc396aef511c96f8279c" },

		{ "6b4f540bab559ae1ef9b352a187980c32869dccb" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "99847f90502e63a05e0dd1c99dfa7757a6992d5e" },
		{ "0c8c7f18e5ee4ae4a3344a855ad31902ea4445e4" },

		{ "5e7391e61888bc96684b4fe7e08bedd677eb6233" },
		{ "1170f2c7b4ad7c7d76ae79490d97ae0ce5b4a929" },
	};

	test_case(zimg::depth::DitherType::NONE, false, false, expected_sha1);
}

TEST(DitherTest, test_limited_chroma)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "d5794ead078fee72fd10fc396aef511c96f8279c" },

		{ "6b4f540bab559ae1ef9b352a187980c32869dccb" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "060f6e8e7210c498427cef730c3602a0b5f31240" },
		{ "459af9f2faa1bc11ff8cd7e1d5995194ca7330d1" },

		{ "3c1832c2c1cfdd9907de1367b3ebdf0eca6d8b31" },
		{ "0686f300af1924d6592d6b422595616bfbc7e63e" },
	};

	test_case(zimg::depth::DitherType::NONE, false, true, expected_sha1);
}

TEST(DitherTest, test_full_luma)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "e9587ac91d9b275d2d0cd5fbf86d603d676b2b42" },

		{ "f9256d89a978d49f1966b6e6cc4d88f5981dc90f" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "1d8189624d3e4c7e41e2b5d8992cf74df34086d3" },
		{ "34cde4d903c8eb626df6c8185e207146be7c1607" },

		{ "e17c0242b746b63684213f21c4290fc2f123e595" },
		{ "b48655a46858eff58b51b266d842fa5d5600e032" },
	};

	test_case(zimg::depth::DitherType::NONE, true, false, expected_sha1);
}

TEST(DitherTest, test_full_chroma)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "5e1ed7da45389f9001c0944c9cd26d050ff28cda" },

		{ "997e708b6c0ec8861bace71884852984512494cb" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "1dedc59f9d9fd1680aa42fdba8465f4114bc50f2" },
		{ "9602dd577a97f1d1e7939b1a9e00c4db1110b8db" },

		{ "a0680c0b28fbc02561fea7746adc430354db472a" },
		{ "1bda9505b332e08f9810d75b9858cacec4e50201" },
	};

	test_case(zimg::depth::DitherType::NONE, true, true, expected_sha1);
}

TEST(DitherTest, test_bayer_dither)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "d5794ead078fee72fd10fc396aef511c96f8279c" },

		{ "c4cccb53ec3d5ada8707ba52131b8a155e77f44a" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "b1bc4729f8d7b8768137062ad08457f36f8c730d" },
		{ "c4a1ed5e7c7c2b6c7b4fd88348c19abd4fd904c5" },

		{ "1ca1d6db60eea9d6cc1c125e3add1d7951968f9e" },
		{ "31da13a60b8c73c58f808d406a0f2e6066ab8b8b" },
	};

	test_case(zimg::depth::DitherType::ORDERED, false, false, expected_sha1);
}

TEST(DitherTest, test_random_dither)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "d5794ead078fee72fd10fc396aef511c96f8279c" },

		{ "ab2d2b92cd836f28080b28474b66faab21f2f372" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "71e8c03d502ce33036e4d73478aa2932a17c7599" },
		{ "9f782faa689eeff1400880e4bc166a4a66ac229e" },

		{ "23e5c6cb1fdd7f9c4dcbbe66178f5a47a5d4ed3e" },
		{ "57ecf8fc00fc554ba932237ea830e303ed559b19" },
	};

	test_case(zimg::depth::DitherType::RANDOM, false, false, expected_sha1);
}

TEST(DitherTest, test_error_diffusion)
{
	const char *expected_sha1[][3] = {
		{ "02c0adca6d301444ac4bf717fa691fe2758752a5" },
		{ "d5794ead078fee72fd10fc396aef511c96f8279c" },

		{ "e78edb136329d34c7f0a7263506351f89912bc4b" },
		{ "8ba35ed1784cb6d7903a9092abefb3d9afd7a683" },

		{ "17ffbdc53895e2576f02f8279264d7c54f723671" },
		{ "cf92073110b1752ac6a1059229660457c4a9deef" },

		{ "c739ba4bed041192bc6167e03975fc5a709561b4" },
		{ "834f918f24da72a31bb6deb7b1e398446cf052a2" },
	};

	test_case(zimg::depth::DitherType::ERROR_DIFFUSION, false, false, expected_sha1);
}
