#ifndef __PVX_JSON_H__
#define __PVX_JSON_H__

#include <string>
#include <map>
#include <set>
#include <vector>
#include <initializer_list>
#include <functional>
#include <PVX_String.h>

namespace PVX {
	namespace JSON {
		typedef enum class jsElementType {
			Undefined,
			Null,
			Number,
			String,
			Array,
			Object,
			Boolean,
		} jsElementType;

		enum class jsBoolean {
			False,
			True
		};
		
		class Tuple;
		class array;

		class Item {
		public:
			~Item();
			Item();

			Item(const std::initializer_list<Tuple> & its);
			Item(const array & its);
			Item(const enum class jsElementType&);
			Item(const int&);
			Item(const long long&);
			Item(const float&);
			Item(const double&);
			Item(const std::wstring&);
			Item(const std::string&);
			Item(const bool&);
			Item(const enum jsBoolean&);
			Item(const char*);
			Item(const wchar_t*);
			Item(const nullptr_t&);
			Item(const Item&) = default;

			template<typename T>
			inline Item(const std::map<std::wstring, T> & Dictionary) : Type{ JSON::jsElementType::Object } {
				for (auto& [n, v] : Dictionary) Object[n] = (JSON::Item)v;
			}

			template<typename T>
			inline Item(const std::map<std::string, T> & Dictionary) : Type{ JSON::jsElementType::Object } {
				for (auto& [n, v] : Dictionary) Object[PVX::String::ToString(n)] = (JSON::Item)v;
			}

			template<typename T>
			inline Item(const std::vector<T> & v) : Type{ JSON::jsElementType::Array } {
				for (auto & i : v) Array.push_back((JSON::Item)i);
			}

			template<typename T>
			inline Item(const std::set<T> & v) : Type{ JSON::jsElementType::Array } {
				for (auto & i : v) Array.push_back((JSON::Item)i);
			}


			Item & operator=(const enum jsElementType);
			Item & operator=(const int);
			Item & operator=(const long long);
			Item & operator=(const float);
			Item & operator=(const double);
			Item & operator=(const bool);
			Item & operator=(const std::wstring&);
			Item & operator=(const std::string&);
			Item & operator=(const wchar_t * str);
			Item & operator=(const char * str);
			Item & operator=(const std::vector<unsigned char>&);
			Item & operator=(const Item&);

			Item & operator[](const std::wstring&);
			Item & operator[](const std::string&);
			Item & operator[](int);
			Item Get(const std::wstring&, const Item& Default=jsElementType::Undefined) const;
			const Item * Has(const std::wstring &) const;

			Item & operator<<(const std::wstring&);

			void push(const Item&);
			Item pop();
			int length();

			int IsNull();
			int IsUndefined();
			int IsNullOrUndefined();
			int IsEmpty();

			jsElementType Type;
			int NumericFloat;
			std::vector<std::wstring> Keys() const;
			std::vector<PVX::JSON::Item> Values() const;

			double NumberSafeDouble();
			long long NumberSafeInteger();
			std::wstring GetString() const;

			long long & Integer() { return _Integer; };
			bool Boolean() { return !!_Integer; };
			double & Double() { return _Double; };

			long long Integer() const { return NumericFloat? (long long)_Double: _Integer; };
			double Double() const { return NumericFloat ? _Double : _Integer; };
			std::vector<unsigned char> Data();
			void Data(const std::vector<unsigned char> & d);

			Item map(std::function<Item(const Item&)> Convert);
			Item map2(std::function<Item(const Item&, int Index)> Convert);
			void each(std::function<void(Item&)> Func);
			void each(std::function<void(const Item&)> Func) const;
			void each2(std::function<void(Item&, int Index)> Func);
			void each2(std::function<void(const Item&, int Index)> Func) const;
			void eachInObject(std::function<void(const std::wstring& Name, Item&)> Func);
			void eachInObject(std::function<void(const std::wstring& Name, const Item&)> Func) const;
			Item GroupBy(std::function<std::wstring(const Item&)> Func);
			Item filter(std::function<int(const Item&)> Test);
			Item find(std::function<int(const Item&)> Test, size_t Start=0);
			int findIndex(std::function<int(const Item&)> Test, size_t Start = 0);
			Item sort(std::function<int(Item&, Item&)> Compare);
			Item Copy();
			Item DeepCopy();
			Item DeepReducedCopy();
			Item & Cache();
			Item & Cache(const std::string & str);
			Item & Cache(const std::wstring & str);

			Item & Merge(const Item & With);
			int SaveBinary(const wchar_t * Filename);
			int SaveBinary(const char * Filename);
			static Item ReadBinary(const char * Filename);
			static Item ReadBinary(const wchar_t * Filename);

			std::wstring String;
			std::vector<Item> Array;
			std::map<std::wstring, Item> Object;
		private:
			union {
				long long _Integer;
				double _Double;
			};
			void WriteBin(void *);
			static Item ReadBin(void *);
			void Release();
			Item * cache = nullptr;
		};

		class Tuple {
		protected:
			friend class Item;
			std::wstring Name;
			Item _Item;
		public:
			Tuple(const std::wstring & Name, const Item & item): Name{ Name }, _Item(item) {};
			Tuple(const std::wstring & Name, const char* item): Name{ Name }, _Item(item) {};
		};

		class array {
		protected:
			std::vector<Item> itms;
			friend class Item;
		public:
			array(const std::initializer_list<Item> & itm) : itms{itm}{}
		};

		std::wstring stringify(const Item&);
		Item parse(const unsigned char*, int size);
		Item parse(const std::vector<unsigned char> &);
		Item parse(const std::wstring & Json);
	}
}
#endif