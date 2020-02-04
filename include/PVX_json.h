#ifndef __PVX_JSON_H__
#define __PVX_JSON_H__

#include <string>
#include <map>
#include <set>
#include <vector>
#include <initializer_list>
#include <functional>
#include <PVX_Encode.h>
#include <PVX_String.h>
#include <variant>

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

		class jsArray;

		class Item {
		public:
			Item() : Type{ jsElementType::Undefined } {}
			Item(const enum jsElementType& tp) : Type{ tp } {
				switch (tp) {
					case jsElementType::Undefined: Value = false; break;
					case jsElementType::Null: Value = false; break;
					case jsElementType::Number: Value = 0.0; break;
					case jsElementType::String: Value = L""; break;
					case jsElementType::Array: Value = std::vector<Item>(); break;
					case jsElementType::Object: Value = std::map<std::wstring, Item>(); break;
					case jsElementType::Boolean: Value = false; break;
				}
			}
			Item(const bool& v) : Type{ jsElementType::Boolean } {}
			Item(const int& v) : Type{ jsElementType::Number }, Value{ (long long)v } {}
			Item(const long long& v) : Type{ jsElementType::Number }, Value{ v } {}
			Item(const float& v) : Type{ jsElementType::Number }, Value{ v } {}
			Item(const double& v) : Type{ jsElementType::Number }, Value{ v } {}
			Item(const std::wstring& s) : Type{ jsElementType::String }, Value{ s } {; }
			Item(const wchar_t* s) : Type{ jsElementType::String }, Value{ s } { }
			Item(const nullptr_t&) : Type{ jsElementType::Null } {}
			Item(const std::string& s) : Type{ jsElementType::String } {
				std::wstring String;
				for (auto c : s) String.push_back(c);
				Value = String;
			}
			Item(const char* str) : Type{ jsElementType::String } {
				std::wstring String;
				int i = 0;
				while (str[i])
					String.push_back(str[i++]);
				Value = String;
			}


			Item(const jsArray& its);
			template<typename T>
			inline Item(const std::map<std::wstring, T>& Dictionary) : Type{ JSON::jsElementType::Object } {
				std::map<std::wstring, Item> Object;
				for (auto& [n, v] : Dictionary) Object[n] = (JSON::Item)v;
				Value = Object;
			}

			template<typename T>
			inline Item(const std::map<std::string, T>& Dictionary) : Type{ JSON::jsElementType::Object } {
				std::map<std::wstring, Item> Object;
				for (auto& [n, v] : Dictionary) Object[PVX::Encode::ToString(n)] = (JSON::Item)v;
				Value = Object;
			}

			template<typename T>
			inline Item(const std::vector<T>& v) : Type{ JSON::jsElementType::Array } {
				std::vector<Item> Array;
				for (auto& i : v) Array.push_back((JSON::Item)i);
				Value = Array;
			}

			template<typename T>
			inline Item(const std::set<T>& v) : Type{ JSON::jsElementType::Array } {
				std::vector<Item> Array;
				for (auto& i : v) Array.push_back((JSON::Item)i);
				Value = Array;
			}

			inline Item(const std::initializer_list<std::tuple<std::wstring, Item>>& dict) : Type{ JSON::jsElementType::Object } {
				std::map<std::wstring, Item> val;
				for (auto& [n, v] : dict) val[n] = v;
				Value = val;
			}

			Item& operator=(const enum jsElementType);
			Item& operator=(const int);
			Item& operator=(const long long);
			Item& operator=(const float);
			Item& operator=(const double);
			Item& operator=(const bool);
			Item& operator=(const std::wstring&);
			Item& operator=(const std::string&);
			Item& operator=(const wchar_t* str);
			Item& operator=(const char* str);
			Item& operator=(const std::vector<unsigned char>&);
			Item& operator=(const Item& obj) {
				Type = obj.Type;
				Value = obj.Value;
				return *this;
			}

			Item& operator[](const std::wstring&);
			Item& operator[](const std::string&);
			Item& operator[](int);
			const Item& operator[](const std::wstring&) const;
			const Item& operator[](const std::string&) const;
			const Item& operator[](int) const;
			Item Get(const std::wstring&, const Item& Default = jsElementType::Undefined) const;
			const Item* Has(const std::wstring&) const;

			Item& operator<<(const std::wstring&);

			void push(const Item&);
			Item pop();
			int length() const;

			bool IsNull() const;
			bool IsUndefined() const;
			bool IsNullOrUndefined() const;
			bool IsEmpty() const;
			inline bool IsInteger() const { return Type == jsElementType::Number && std::holds_alternative<long long>(Value); }
			inline bool IsDouble() const { return Type == jsElementType::Number && std::holds_alternative<double>(Value); }

			std::vector<std::wstring> Keys() const;
			std::vector<PVX::JSON::Item> Values() const;

			double NumberSafeDouble();
			long long NumberSafeInteger();
			std::wstring GetString() const;

			long long& Integer() { return std::get<long long>(Value); };
			long long Integer() const { return std::holds_alternative<long long>(Value) ? std::get<long long>(Value) : (long long)std::get<double>(Value); };
			double& Double() { return std::get<double>(Value); };
			double Double()const { return std::holds_alternative<long long>(Value) ? (double)std::get<long long>(Value) : std::get<double>(Value); };
			bool& Boolean() { return std::get<bool>(Value); };
			bool Boolean() const { return std::get<bool>(Value); };
			std::wstring& String() { return std::get<std::wstring>(Value); }
			std::wstring String() const { return std::get<std::wstring>(Value); }

			std::vector<unsigned char> Data();
			void Data(const std::vector<unsigned char>& d);

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
			Item find(std::function<int(const Item&)> Test, size_t Start = 0);
			int findIndex(std::function<int(const Item&)> Test, size_t Start = 0);
			Item sort(std::function<int(Item&, Item&)> Compare);
			Item Copy();
			Item DeepCopy();
			Item DeepReducedCopy();
			Item& Cache();
			Item& Cache(const std::string& str);
			Item& Cache(const std::wstring& str);

			Item& Merge(const Item& With);
			int SaveBinary(const wchar_t* Filename);
			int SaveBinary(const char* Filename);
			static Item ReadBinary(const char* Filename);
			static Item ReadBinary(const wchar_t* Filename);

			jsElementType Type;
			std::variant<bool, long long, double, std::wstring, std::vector<Item>, std::map<std::wstring, Item>> Value;

			inline std::vector<Item> getArray() {
				return std::get<std::vector<Item>>(Value);
			}
			inline const std::vector<Item>getArray() const {
				return std::get<std::vector<Item>>(Value);
			}
			inline std::map<std::wstring, Item> getObject() {
				return std::get<std::map<std::wstring, Item>>(Value);
			}
			inline const std::map<std::wstring, Item> getObject() const {
				return std::get<std::map<std::wstring, Item>>(Value);
			}
		private:
			void WriteBin(void*);
			static Item ReadBin(void*);
			Item* cache = nullptr;
		};

		class jsArray {
		protected:
			const std::initializer_list<Item>& itms;
			friend class Item;
		public:
			jsArray(const std::initializer_list<Item>& itm) : itms{ itm } {}
		};

		std::wstring stringify(const Item& Object, bool Format = false);
		Item parse(const unsigned char*, int size);
		Item parse(const std::vector<unsigned char>&);
		Item parse(const std::wstring& Json);
	}
}
#endif