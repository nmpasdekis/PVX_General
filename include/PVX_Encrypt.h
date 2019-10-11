#include <array>
#include <vector>
#include <functional>
#include <type_traits>

namespace PVX::Encrypt {
	std::array<unsigned char, 20> SHA1(const void* msg, size_t sz);

	template<int OutputSize>
	class HashAlgorithm {
	protected:
		virtual void ProcessBlock(void* Block) = 0;
	public:
		virtual HashAlgorithm<OutputSize>& Update(const void* Message, size_t MessageSize) = 0;
		virtual std::array<unsigned char, OutputSize> operator()() = 0;
		template<typename T>
		HashAlgorithm<OutputSize>& Update(const T& Message) {
			return Update(Message.data(), Message.size());
		}
	};

	class SHA1_Algorithm : public HashAlgorithm<20> {
	private:
		unsigned int h0 = 0x67452301;
		unsigned int h1 = 0xEFCDAB89;
		unsigned int h2 = 0x98BADCFE;
		unsigned int h3 = 0x10325476;
		unsigned int h4 = 0xC3D2E1F0;
		unsigned long long BitCount = 0;
		unsigned long long More = 0;
		unsigned char tmp[128]{ 0 };
	protected:
		void ProcessBlock(void* Block);
	public:
		HashAlgorithm<20>& Update(const void* Message, size_t MessageSize);
		std::array<unsigned char, 20> operator()();
		template<typename T>
		HashAlgorithm<20>& Update(const T& Message) {
			return Update(Message.data(), Message.size());
		}
	};

	class SHA256_Algorithm : public HashAlgorithm<20> {
	private:
		unsigned int h0 = 0x67452301;
		unsigned int h1 = 0xEFCDAB89;
		unsigned int h2 = 0x98BADCFE;
		unsigned int h3 = 0x10325476;
		unsigned int h4 = 0xC3D2E1F0;
		unsigned long long BitCount = 0;
		unsigned long long More = 0;
		unsigned char tmp[128]{ 0 };
	protected:
		void ProcessBlock(void* Block);
	public:
		HashAlgorithm<20>& Update(const void* Message, size_t MessageSize);
		std::array<unsigned char, 20> operator()();
		template<typename T>
		HashAlgorithm<20>& Update(const T& Message) {
			return Update(Message.data(), Message.size());
		}
	};

	template<typename Hash, int BlockSize>
	decltype(Hash()()) HMAC(const void* Key, size_t KeySize, const void* Message, size_t MessageSize) {
		using Block = std::array<unsigned char, BlockSize>;
		using OutBlock = decltype(Hash()());

		Block key = [](const unsigned char* k, size_t sz) {
			constexpr int OutputSize = sizeof(OutBlock);
			if (sz > BlockSize) {
				OutBlock tmpKey = Hash().Update(k, sz)();
				if constexpr (OutputSize < BlockSize) {
					Block ret{ 0 };
					memcpy(&ret[0], &tmpKey[0], OutputSize);
					return ret;
				} else {
					Block ret{ 0 };
					memcpy(&ret[0], &tmpKey[0], BlockSize);
					return ret;
				}
			} else {
				Block ret{ 0 };
				memcpy(&ret[0], k, sz);
				return ret;
			}
		}((unsigned char*)Key, KeySize);

		Block opad;
		Block ipad;
		for (int i = 0; i<BlockSize; i++) {
			opad[i] = 0x5c ^ key[i];
			ipad[i] = 0x36 ^ key[i];
		}
		auto Inner = Hash().Update(ipad).Update(Message, MessageSize)();
		return Hash().Update(opad).Update(Inner)();
	}

	inline auto HMAC_SHA1(const void* Key, size_t KeySize, const void* Message, size_t MessageSize) {
		return HMAC<SHA1_Algorithm, 64>(Key, KeySize, Message, MessageSize);
	}
	template<typename T1, typename T2>
	inline auto HMAC_SHA1(const T1& Key, const T2& Message) {
		return HMAC<SHA1_Algorithm, 64>(Key.data(), Key.size(), Message.data(), Message.size());
	}
}