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

namespace PVX {
	namespace JSON {
		typedef enum class jsElementType {
			Undefined,
			Null,
			Boolean,
			Integer,
			Float,
			String,
			Array,
			Object,
		} jsElementType;

		class jsArray;
		class Item;


		struct Variant_String {
			int refCount;
			std::wstring String;
		};
		struct Variant_Array {
			int refCount;
			std::vector<Item> Array;
		};
		struct Variant_Object {
			int refCount;
			std::map<std::wstring, Item> Object;
		};

		struct Variant {
			jsElementType GetType() const { return Type; }

			~Variant() { Release(); }
			Variant() : Type{ jsElementType::Undefined } { Value.Boolean = false; }
			Variant(const long long v) : Type{ jsElementType::Integer } { Value.Integer = v; }
			Variant(const double v) : Type{ jsElementType::Float } { Value.Double = v; }
			Variant(const bool v) : Type{ jsElementType::Boolean } { Value.Boolean = v; }
			Variant(const nullptr_t v) : Type{ jsElementType::Null } { Value.Integer = 0; }
			Variant(const std::wstring& v) : Type{ jsElementType::String } { Value.String = new Variant_String{ 1, v }; }
			Variant(const std::vector<Item>& v) : Type{ jsElementType::Array } { Value.Array = new Variant_Array{ 1, v }; }
			Variant(const std::map<std::wstring, Item>& v) : Type{ jsElementType::Object } { Value.Object = new Variant_Object{ 1, v }; }
			Variant(const jsElementType tp) : Type{ tp } {
				switch (tp) {
					case jsElementType::Undefined: Value.Boolean = false; break;
					case jsElementType::Null: Value.Boolean = false; break;
					case jsElementType::Boolean: Value.Boolean = false; break;
					case jsElementType::Integer: Value.Integer = 0ll; break;
					case jsElementType::Float: Value.Double = 0.0; break;
					case jsElementType::String: Value.String = new Variant_String{ 1 }; break;
					case jsElementType::Array: Value.Array = new Variant_Array{ 1 }; break;
					case jsElementType::Object: Value.Object = new Variant_Object{ 1 }; break;
				}
			}
			Variant(const Variant& v) :Value{ v.Value }, Type{ v.Type } {
				if (Type>=jsElementType::String) Value.String->refCount++;
			}


			bool& operator=(const bool v) { Release(); Type = jsElementType::Boolean; Value.Integer = v; return Value.Boolean; }
			long long& operator=(const int v) { Release(); Type = jsElementType::Integer; Value.Integer = v; return Value.Integer; }
			long long& operator=(const long long v) { Release(); Type = jsElementType::Integer; Value.Integer = v; return Value.Integer; }
			double& operator=(const float v) { Release(); Type = jsElementType::Float; Value.Double = v; return Value.Double; }
			double& operator=(const double v) { Release(); Type = jsElementType::Float; Value.Double = v; return Value.Double; }

			std::wstring& operator=(const wchar_t* v) {
				Release();
				Type = jsElementType::String;
				Value.String = new Variant_String{ 1, v };
				return Value.String->String;
			}
			std::wstring& operator=(const std::wstring& v) {
				Release();
				Type = jsElementType::String;
				Value.String = new Variant_String{ 1, v };
				return Value.String->String;
			}
			std::vector<Item>& operator=(const std::vector<Item>& v) { Release(); Type = jsElementType::Array; Value.Array = new Variant_Array{ 1, v }; return Value.Array->Array; }
			std::map<std::wstring, Item>& operator=(const std::map<std::wstring, Item>& v) { Release(); Type = jsElementType::Object; Value.Object = new Variant_Object{ 1, v }; return Value.Object->Object; }

			Variant& operator=(const Variant& v) {
				if (v.Type>=jsElementType::String) v.Value.String->refCount++;
				Release();
				Type = v.Type;
				Value = v.Value;
				return *this;
			}

			operator int() { return Value.Integer; }
			operator long long() { return Value.Integer; }
			operator float() { return Value.Double; }
			operator double() { return Value.Double; }
			operator bool() { return Value.Boolean; }

			long long& Integer() { return Value.Integer; }
			double& Double() { return Value.Double; }
			bool& Boolean() { return Value.Boolean; }

			long long Integer() const { return Value.Integer; }
			double Double() const { return Value.Double; }
			bool Boolean() const { return Value.Boolean; }

			std::wstring& String() { return Value.String->String; }
			std::vector<Item>& Array() { return Value.Array->Array; }
			std::map<std::wstring, Item>& Object() { return Value.Object->Object; }

			const std::wstring& String() const { return Value.String->String; }
			const std::vector<Item>& Array() const { return Value.Array->Array; }
			const std::map<std::wstring, Item>& Object() const { return Value.Object->Object; }
		private:
			jsElementType Type;
			union {
				bool Boolean;
				long long Integer;
				double Double;
				Variant_String* String;
				Variant_Array* Array;
				Variant_Object* Object;
			} Value;
			void Release() {
				switch (Type) {
					case PVX::JSON::jsElementType::String: if (!--Value.String->refCount)
						delete Value.String;
						break;
					case PVX::JSON::jsElementType::Array: if (!--Value.Array->refCount)
						delete Value.Array;
						break;
					case PVX::JSON::jsElementType::Object: if (!--Value.Object->refCount)
						delete Value.Object;
						break;
				}
				Type = jsElementType::Undefined;
			}
		};

		class Item {
		public:
			Item() {}
			Item(const enum jsElementType& tp) : Value{ tp } {}
			Item(const bool& v) : Value{ v } {}
			Item(const int& v) : Value{ (long long)v } {}
			Item(const long long& v) : Value{ v } {}
			Item(const float& v) : Value{ v } {}
			Item(const double& v) : Value{ v } {}
			Item(const std::wstring& s) : Value{ s } { ; }
			Item(const wchar_t* s) : Value{ s } {}
			Item(const nullptr_t&) : Value{ nullptr } {}
			Item(const std::string& s) {
				std::wstring String;
				for (auto c : s) String.push_back(c);
				Value = String;
			}
			Item(const char* str) {
				std::wstring String;
				int i = 0;
				while (str[i])
					String.push_back(str[i++]);
				Value = String;
			}


			Item(const jsArray& its);
			template<typename T>
			inline Item(const std::map<std::wstring, T>& Dictionary) {
				std::map<std::wstring, Item> Object;
				for (auto& [n, v] : Dictionary) Object[n] = (JSON::Item)v;
				Value = Object;
			}

			template<typename T>
			inline Item(const std::map<std::string, T>& Dictionary) {
				std::map<std::wstring, Item> Object;
				for (auto& [n, v] : Dictionary) Object[PVX::Encode::ToString(n)] = (JSON::Item)v;
				Value = Object;
			}

			template<typename T>
			inline Item(const std::vector<T>& v) {
				std::vector<Item> Array;
				for (auto& i : v) Array.push_back((JSON::Item)i);
				Value = Array;
			}

			template<typename T>
			inline Item(const std::set<T>& v) {
				std::vector<Item> Array;
				for (auto& i : v) Array.push_back((JSON::Item)i);
				Value = Array;
			}

			inline Item(const std::initializer_list<std::tuple<std::wstring, Item>>& dict) {
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

			jsElementType Type() const { return Value.GetType(); }

			void push(const Item&);
			Item pop();
			int length() const;

			bool IsNull() const;
			bool IsUndefined() const;
			bool IsNullOrUndefined() const;
			bool IsEmpty() const;
			inline bool IsInteger() const { return Value.GetType() == jsElementType::Integer; }
			inline bool IsDouble() const { return Value.GetType() == jsElementType::Float; }

			std::vector<std::wstring> Keys() const;
			std::vector<PVX::JSON::Item> Values() const;

			double NumberSafeDouble();
			long long NumberSafeInteger();
			std::wstring GetString() const;

			long long& Integer() { return Value.Integer(); };
			long long Integer() const { return Value.Integer(); };
			double& Double() { return Value.Double(); };
			double Double()const { return Value.Double(); };
			bool& Boolean() { return Value.Boolean(); };
			bool Boolean() const { return Value.Boolean(); };
			std::wstring& String() { return Value.String(); }
			std::wstring String() const { return Value.String(); }

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
			//Item& Cache();
			//Item& Cache(const std::string& str);
			//Item& Cache(const std::wstring& str);

			Item& Merge(const Item& With);
			int SaveBinary(const wchar_t* Filename);
			int SaveBinary(const char* Filename);
			static Item ReadBinary(const char* Filename);
			static Item ReadBinary(const wchar_t* Filename);


			Variant Value;

			inline std::vector<Item> getArray() {
				return Value.Array();
			}
			inline const std::vector<Item>getArray() const {
				return Value.Array();
			}
			inline std::map<std::wstring, Item> getObject() {
				return Value.Object();
			}
			inline const std::map<std::wstring, Item> getObject() const {
				return Value.Object();
			}
		private:
			void WriteBin(void*);
			static Item ReadBin(void*);
			//Item* cache = nullptr;
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
		//Item parse2(const std::wstring& Json);
		//Item parse3(const std::wstring& Json);
	}
}
#endif