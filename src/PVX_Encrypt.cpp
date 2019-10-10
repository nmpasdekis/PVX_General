#include <PVX_Encrypt.h>

namespace PVX::Encrypt {
	using u32 = unsigned int;
	using u64 = unsigned long long;

	u32 leftrotate(u32 v, int i) {
		return (v << i) | (v >> (32-i));
	}

#define endian(v) ((leftrotate(v, 24) & 0xff00ff00) | (leftrotate(v, 8) & 0x00ff00ff))
#define do_all(i, nk) \
	k = (nk);\
	u32 temp = leftrotate(a, 5) + f + e + k + w[i]; \
    e = d; \
	d = c; \
	c = leftrotate(b, 30); \
	b = a; \
	a = temp;

#define do_00_19(i) { f = (b & c) | ((~b) & d); do_all((i), 0x5A827999);}
#define do_20_39(i) { f = b ^ c ^ d; do_all((i), 0x6ED9EBA1);}
#define do_40_59(i) { f = (b & c) | (b & d) | (c & d); do_all((i), 0x8F1BBCDC);}
#define do_60_79(i) { f = b ^ c ^ d; do_all((i), 0xCA62C1D6);}


	struct sha1Context {
		u32 h0 = 0x67452301;
		u32 h1 = 0xEFCDAB89;
		u32 h2 = 0x98BADCFE;
		u32 h3 = 0x10325476;
		u32 h4 = 0xC3D2E1F0;

		void Update(u32* block) {
			u32 w[80], f, k;
			u32 a = h0;
			u32 b = h1;
			u32 c = h2;
			u32 d = h3;
			u32 e = h4;

			for (int i = 0; i<16; i++) w[i] = endian(block[i]);
			for (int i = 16; i < 80; i++) w[i] = leftrotate(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

			for (int i = 0; i < 20; i++) do_00_19(i);
			for (int i = 20; i < 40; i++) do_20_39(i);
			for (int i = 40; i < 60; i++) do_40_59(i);
			for (int i = 60; i < 80; i++) do_60_79(i);

			h0 += a;
			h1 += b;
			h2 += c;
			h3 += d;
			h4 += e;
		}
	};

	std::array<unsigned char, 20> SHA1(const void* msg, size_t sz) {
		sha1Context context;
		unsigned char* Msg = (unsigned char*)msg;
		u64 ml = sz * 8;
		{
			u32 c1 = (ml >> 32) & 0x00ffffffff; c1 = endian(c1);
			u32 c0 = (ml) & 0x00ffffffff; c0 = endian(c0);
			ml = c1 | (((u64)c0) << 32);
		}
		while (sz>=64) {
			context.Update((u32*)Msg);
			Msg += 64;
			sz -= 64;
		}

		unsigned char tmp[128]{ 0 };
		u64* tmp64 = ((u64*)tmp);
		tmp64[15] = ml;

		memcpy(tmp, Msg, sz);
		tmp[sz] = 0x80;
		if (sz < 56) {
			tmp64[7] = ml;
			context.Update((u32*)tmp);
		} else {
			context.Update((u32*)tmp);
			context.Update((u32*)(tmp+64));
		}

		std::array<unsigned char, 20> ret;
		u32* retDword = (u32*)&ret[0];
		retDword[0] = endian(context.h0);
		retDword[1] = endian(context.h1);
		retDword[2] = endian(context.h2);
		retDword[3] = endian(context.h3);
		retDword[4] = endian(context.h4);

		return ret;
	}


	void SHA1_Algorithm::ProcessBlock(void* Block) {
		u32* block = (u32*)Block;
		u32 w[80], f, k;
		u32 a = h0;
		u32 b = h1;
		u32 c = h2;
		u32 d = h3;
		u32 e = h4;

		for (int i = 0; i<16; i++) w[i] = endian(block[i]);
		for (int i = 16; i < 80; i++) w[i] = leftrotate(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

		for (int i = 0; i < 20; i++) do_00_19(i);
		for (int i = 20; i < 40; i++) do_20_39(i);
		for (int i = 40; i < 60; i++) do_40_59(i);
		for (int i = 60; i < 80; i++) do_60_79(i);

		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	HashAlgorithm<20>& SHA1_Algorithm::Update(const void* Message, size_t MessageSize) {
		unsigned char* Msg = (unsigned char*)Message;
		BitCount += MessageSize * 8;

		if (More) {
			int less = 64 - More;
			if (MessageSize<less)less = MessageSize;
			memcpy(tmp+More, Message, less);
			More += less;
			MessageSize -= less;
			Msg += less;
			if (More<64) {
				return *this;
			}
			ProcessBlock(tmp);
			More = 0;
			memset(tmp, 0, 128);
		}

		while (MessageSize>=64) {
			ProcessBlock(Msg);
			Msg += 64;
			MessageSize -= 64;
		}

		memcpy(tmp, Msg, MessageSize);
		More = MessageSize;

		return *this;
	}

	std::array<unsigned char, 20> SHA1_Algorithm::operator()() {
		u64 ml = BitCount;
		{
			u32 c1 = (ml >> 32) & 0x00ffffffff; c1 = endian(c1);
			u32 c0 = (ml) & 0x00ffffffff; c0 = endian(c0);
			ml = c1 | (((u64)c0) << 32);
		}

		u64* tmp64 = ((u64*)tmp);
		tmp64[15] = ml;

		tmp[More] = 0x80;
		if (More < 56) {
			tmp64[7] = ml;
			ProcessBlock(tmp);
		} else {
			ProcessBlock(tmp);
			ProcessBlock((tmp+64));
		}

		std::array<unsigned char, 20> ret;
		u32* retDword = (u32*)&ret[0];
		retDword[0] = endian(h0);
		retDword[1] = endian(h1);
		retDword[2] = endian(h2);
		retDword[3] = endian(h3);
		retDword[4] = endian(h4);

		h0 = 0x67452301;
		h1 = 0xEFCDAB89;
		h2 = 0x98BADCFE;
		h3 = 0x10325476;
		h4 = 0xC3D2E1F0;

		BitCount = 0;
		More = 0;
		memset(tmp, 0, 128);
		return ret;
	}

}