#include<PVX_json.h>
#include<PVX_Encode.h>
#include<sstream>
#include <stdio.h>
#include <PVX_String.h>
#include <PVX_Encode.h>

static std::wstring JsonString(const std::wstring& s) {
	std::wstringstream ret;
	ret << '"';
	for (auto c : s) {
		switch (c) {
			case '"': ret << "\\\""; break;
			case '\n': ret << "\\n"; break;
			case '\t': ret << "\\t"; break;
			case '\r': ret << "\\r"; break;
			case '\\': ret << "\\\\"; break;
			case 0:
				goto forceEnd;
			default: ret << c;
		}
	}
forceEnd:
	ret << '"';
	return ret.str();
}

namespace PVX {
	namespace JSON {
		static void WriteNumber(FILE* f, size_t n) {
			fwrite(&n, 4, 1, f);
		}
		static void WriteString(FILE* f, const std::wstring& s) {
			WriteNumber(f, s.size());
			fwrite(&s[0], sizeof(wchar_t), s.size(), f);
		}
		void Item::WriteBin(void* f) {
			WriteNumber((FILE*)f, (size_t)Type);
			WriteNumber((FILE*)f, std::holds_alternative<double>(Value));
			switch (Type) {
				case JSON::jsElementType::Number: fwrite(&Integer(), sizeof(long long), 1, (FILE*)f); break;
				case JSON::jsElementType::String: WriteString((FILE*)f, String()); break;
				case JSON::jsElementType::Array:
				{
					auto& Array = std::get<std::vector<Item>>(Value);
					WriteNumber((FILE*)f, Array.size());
					for (auto& i : Array)
						i.WriteBin(f);
					break;
				}
				case JSON::jsElementType::Object:
				{
					auto& Object = std::get<std::map<std::wstring, Item>>(Value);
					WriteNumber((FILE*)f, Object.size());
					for (auto& i : Object) {
						if (i.second.Type!= JSON::jsElementType::Undefined && i.second.Type!=JSON::jsElementType::Null)
							WriteString((FILE*)f, i.first);
						i.second.WriteBin(f);
					}
					break;
				}

			};
		}
		int Item::SaveBinary(const wchar_t* Filename) {
			FILE* fout;
			if (!_wfopen_s(&fout, Filename, L"wb")) {
				WriteBin(fout);
				fclose(fout);
			}
			return 0;
		}

		int Item::SaveBinary(const char* Filename) {
			FILE* fout;
			if (!fopen_s(&fout, Filename, "wb") && fout) {
				WriteBin(fout);
				fclose(fout);
			}
			return 0;
		}
		static double ReadDouble(FILE* f) {
			double ret;
			fread(&ret, sizeof(double), 1, f);
			return ret;
		}
		static size_t ReadInt(FILE* f) {
			size_t ret = 0;
			fread(&ret, 4, 1, f);
			return ret;
		}
		static long long ReadLong(FILE* f) {
			long long ret = 0;
			fread(&ret, sizeof(long long), 1, f);
			return ret;
		}
		static std::wstring ReadString(FILE* f) {
			size_t tmp = 0;
			fread(&tmp, 4, 1, f);
			std::wstring ret;
			ret.resize(tmp);
			fread(&ret[0], sizeof(wchar_t), tmp, f);
			return ret;
		}
		Item Item::ReadBin(void* f) {
			JSON::jsElementType type = (JSON::jsElementType)ReadInt((FILE*)f);
			int IsFloat = ReadInt((FILE*)f);
			size_t count;
			switch (type) {
				case JSON::jsElementType::Number:
					if (IsFloat) return ReadDouble((FILE*)f);
					else return ReadLong((FILE*)f);
				case JSON::jsElementType::String:
					return ReadString((FILE*)f);
				case JSON::jsElementType::Array:
				{
					count = ReadInt((FILE*)f);
					Item ret = JSON::jsElementType::Array;
					auto& Array = std::get<std::vector<Item>>(ret.Value);
					for (auto i = 0; i < count; i++) Array.push_back(ReadBin(f));
					return ret;
				};
				case JSON::jsElementType::Object:
				{
					count = ReadInt((FILE*)f);
					Item ret = JSON::jsElementType::Object;
					auto& Object = std::get<std::map<std::wstring, Item>>(ret.Value);
					for (auto i = 0; i < count; i++) { auto name = ReadString((FILE*)f); Object[name] = ReadBin(f); }
					return ret;
				};
			};
			return type;
		}
		Item Item::ReadBinary(const char* Filename) {
			FILE* fin;
			if (!fopen_s(&fin, Filename, "rb")) {
				return ReadBin(fin);
			}
			return jsElementType::Undefined;
		}
		Item Item::ReadBinary(const wchar_t* Filename) {
			FILE* fin;
			if (!_wfopen_s(&fin, Filename, L"rb")) {
				return ReadBin(fin);
			}
			return jsElementType::Undefined;
		}

		//Item::Item() : Type{ jsElementType::Undefined }, NumericFloat{ 0 }, _Integer{ 0 } {}
		//Item::Item(const enum jsElementType& tp) : Type{ tp }, NumericFloat{ 0 }, _Integer{ 0 }{}
		//Item::Item(const bool& v) : Type{ jsElementType::Boolean }, NumericFloat{ 0 }, _Integer{ v } {}
		//Item::Item(const int& v) : Type{ jsElementType::Number }, NumericFloat{ 0 }, _Integer{ v } {}
		////Item::Item(const enum jsBoolean& v) : Type{ jsElementType::Number }, NumericFloat{ 0 }, _Integer{ (bool)v } {}
		//Item::Item(const long long& v) : Type{ jsElementType::Number }, NumericFloat{ 0 }, _Integer{ v } {}
		//Item::Item(const float& v) : Type{ jsElementType::Number }, NumericFloat{ 1 }, _Double{ v } {}
		//Item::Item(const double& v) : Type{ jsElementType::Number }, NumericFloat{ 1 }, _Double{ v } {}
		//Item::Item(const std::wstring& s) : Type{ jsElementType::String }, NumericFloat{ 0 }, _Integer{ 0 } { String = s; }
		//Item::Item(const wchar_t* s) : Type{ jsElementType::String }, NumericFloat{ 0 }, _Integer{ 0 } { String = s; }
		//Item::Item(const nullptr_t&) : Type{ jsElementType::Null }, NumericFloat{ 0 }, _Integer{ 0 } {}
		//Item::Item(const std::string& s) : Type{ jsElementType::String }, NumericFloat{ 0 }, _Integer{ 0 } { for (auto c : s) String.push_back(c); }
		//Item::Item(const char* str) : Type{ jsElementType::String }, NumericFloat{ 0 }, _Integer{ 0 } { int i = 0; while (str[i]) String.push_back(str[i++]); }
		Item::Item(const jsArray& its) : Type{ JSON::jsElementType::Array } {
			auto& Array = std::get<std::vector<Item>>(Value);
			for (auto& it : its.itms) {
				Array.push_back(it);
			}
		}

		static std::wstring wstr(const std::string& str) {
			std::wstring ret; ret.resize(str.size());
			memcpy(&ret[0], &str[0], str.size());
			return ret;
		}
		Item& Item::operator=(const jsElementType tp) {
			Type = tp;
			switch (tp) {
				case jsElementType::Undefined: Value = false; break;
				case jsElementType::Null: Value = false; break;
				case jsElementType::Number: Value = 0.0; break;
				case jsElementType::String: Value = L""; break;
				case jsElementType::Array: Value = std::vector<Item>(); break;
				case jsElementType::Object: Value = std::map<std::wstring, Item>(); break;
				case jsElementType::Boolean: Value = false; break;
			}
			return *this;
		}
		Item& Item::operator=(const int v) {
			Type = JSON::jsElementType::Number;
			Value = (long long)v;
			return *this;
		}
		Item& Item::operator=(const long long v) {
			Type = JSON::jsElementType::Number;
			Value = (long long)v;
			return *this;
		}
		Item& Item::operator=(const float v) {
			Type = JSON::jsElementType::Number;
			Value = (double)v;
			return *this;
		}
		Item& Item::operator=(const double v) {
			Type = JSON::jsElementType::Number;
			Value = v;
			return *this;
		}
		Item& Item::operator=(const bool v) {
			Type = JSON::jsElementType::Boolean;
			Value = v;
			return *this;
		}
		Item& Item::operator=(const std::wstring& v) {
			Type = JSON::jsElementType::String;
			Value = v;
			return *this;
		}

		Item& Item::operator=(const std::string& s) {
			Type = JSON::jsElementType::String;
			std::wstring String;
			String.reserve(s.size());
			for (auto c : s) String.push_back(c);
			Value = String;
			return *this;
		}
		Item& Item::operator=(const wchar_t* v) {
			Type = JSON::jsElementType::String;
			Value = std::wstring(v);
			return *this;
		}

		Item& Item::operator=(const char* s2) {
			std::string s = s2;
			Type = JSON::jsElementType::String;
			std::wstring String;
			String.reserve(s.size());
			for (auto c : s) String.push_back(c);
			Value = String;
			return *this;
		}

		Item& Item::operator=(const std::vector<unsigned char>& v) {
			Type = JSON::jsElementType::String;
			std::string tmp = PVX::Encode::Base64(v);
			std::wstring String;
			String.reserve(tmp.size());
			for (auto c : tmp) String.push_back(c);
			Value = String;
			return *this;
		}

		Item& Item::operator=(const Item& obj) {
			Type = obj.Type;
			Value = obj.Value;
			return *this;
		}

		Item& Item::operator[](const std::wstring& Name) {
			Type = jsElementType::Object;
			return std::get<std::map<std::wstring, Item>>(Value)[Name];
		}

		Item& Item::operator[](const std::string& Name) {
			Type = jsElementType::Object;
			std::wstring n;
			for (auto c : Name)n.push_back(c);
			return std::get<std::map<std::wstring, Item>>(Value)[n];
		}

		Item& Item::operator[](int Index) {
			return std::get<std::vector<Item>>(Value)[Index];
		}

		Item Item::Get(const std::wstring& Name, const Item& Default) const {
			auto& Object = std::get<std::map<std::wstring, Item>>(Value);
			if (auto ret = Object.find(Name); ret!=Object.end())
				return ret->second;
			return Default;
		}

		const Item* Item::Has(const std::wstring& Name) const {
			auto& Object = std::get<std::map<std::wstring, Item>>(Value);
			if (auto ret = Object.find(Name); ret != Object.end())
				return &(ret->second);
			return nullptr;
		}

		Item& Item::operator<<(const std::wstring& s) {
			*this = parse(s.c_str());
			return *this;
		}

		void Item::push(const Item& it) {
			std::get<std::vector<Item>>(Value).push_back(it);
		}

		Item Item::pop() {
			auto& Array = std::get<std::vector<Item>>(Value);
			auto ret = Array.back();
			Array.pop_back();
			return ret;
		}

		int Item::length() {
			return (int)std::get<std::vector<Item>>(Value).size();
		}

		int Item::IsNull() {
			return Type == jsElementType::Null;
		}
		int Item::IsUndefined() {
			return Type == jsElementType::Undefined;
		}
		int Item::IsNullOrUndefined() {
			return Type == jsElementType::Null || Type == jsElementType::Undefined;
		}
		int Item::IsEmpty() {
			return
				Type == jsElementType::Null ||
				Type == jsElementType::Undefined ||
				(Type == jsElementType::Number && !Integer()) ||
				(Type == jsElementType::String && !String().size()) ||
				(Type == jsElementType::Array && !std::get<std::vector<Item>>(Value).size()) ||
				(Type == jsElementType::Object && !std::get<std::map<std::wstring, Item>>(Value).size());
		}

		std::vector<std::wstring> Item::Keys() const {
			std::vector<std::wstring> ret;
			for (auto& kv : std::get<std::map<std::wstring, Item>>(Value))
				ret.push_back(kv.first);
			return ret;
		}
		std::vector<PVX::JSON::Item> Item::Values() const {
			std::vector<PVX::JSON::Item> ret;
			for (auto& kv : std::get<std::map<std::wstring, Item>>(Value))
				ret.push_back(kv.second);
			return ret;
		}

		double Item::NumberSafeDouble() {
			if (Type == jsElementType::Number) {
				return Double();
			}if (Type == jsElementType::String)
				return _wtof(String().c_str());
			return 0.0;
		}

		long long Item::NumberSafeInteger() {
			if (Type == jsElementType::Number) {
				return Integer();
			}if (Type == jsElementType::String)
				return _wtoi(this->String().c_str());
			return 0.0;
		}

		std::vector<unsigned char> Item::Data() {
			return PVX::Decode::Base64(String());
		}

		std::wstring Item::GetString() const {
			if (Type== jsElementType::String)
				return String();
			return stringify(*this);
		}

		void Item::Data(const std::vector<unsigned char>& d) {
			Type = jsElementType::String;
			Value = PVX::Encode::ToString(PVX::Encode::Base64(d));
		}

		Item Item::map(std::function<Item(const Item&)> Convert) {
			if (Type == JSON::jsElementType::Array) {
				Item ret = JSON::jsElementType::Array;
				for (auto& i : std::get<std::vector<Item>>(Value)) {
					ret.push(Convert(i));
				}
				return ret;
			}
			return jsElementType::Undefined;
		}

		Item Item::map2(std::function<Item(const Item&, int Index)> Convert) {
			if (Type == JSON::jsElementType::Array) {
				Item ret = JSON::jsElementType::Array;
				int Index = 0;
				for (auto& i : std::get<std::vector<Item>>(Value)) {
					ret.push(Convert(i, Index++));
				}
				return ret;
			}
			return jsElementType::Undefined;
		}

		void Item::each(std::function<void(Item&)> Func) {
			if (Type == JSON::jsElementType::Array) for (auto& i : std::get<std::vector<Item>>(Value)) Func(i);
		}
		void Item::each(std::function<void(const Item&)> Func) const {
			if (Type == JSON::jsElementType::Array) for (auto& i : std::get<std::vector<Item>>(Value)) Func(i);
		}
		void Item::each2(std::function<void(Item&, int Index)> Func) {
			int Index = 0;
			if (Type == JSON::jsElementType::Array) for (auto& i : std::get<std::vector<Item>>(Value)) Func(i, Index++);
		}
		void Item::each2(std::function<void(const Item&, int Index)> Func) const {
			int Index = 0;
			if (Type == JSON::jsElementType::Array) for (auto& i : std::get<std::vector<Item>>(Value)) Func(i, Index++);
		}
		void Item::eachInObject(std::function<void(const std::wstring& Name, Item&)> Func) {
			if (Type!=jsElementType::Object)return;
			for (auto& [Name, Value] : std::get<std::map<std::wstring, Item>>(Value)) Func(Name, Value);
		}
		void Item::eachInObject(std::function<void(const std::wstring& Name, const Item&)> Func) const {
			if (Type!=jsElementType::Object)return;
			for (const auto& [Name, Value] : std::get<std::map<std::wstring, Item>>(Value)) Func(Name, Value);
		}
		Item Item::GroupBy(std::function<std::wstring(const Item&)> Func) {
			if (Type!=jsElementType::Array) return jsElementType::Undefined;
			Item ret = jsElementType::Object;
			each([&](const Item& it) {
				std::wstring Name = Func(it);
				if (!ret.Has(Name)) ret[Name] = jsElementType::Array;
				ret[Name].push(it);
			});
			return ret;
		}

		Item Item::filter(std::function<int(const Item&)> Test) {
			if (Type == JSON::jsElementType::Array) {
				Item ret = JSON::jsElementType::Array;
				for (auto& i : std::get<std::vector<Item>>(Value)) {
					if (Test(i))
						ret.push(i);
				}
				return ret;
			}
			return jsElementType::Undefined;
		}
		Item Item::find(std::function<int(const Item&)> Test, size_t Start) {
			if (Type == JSON::jsElementType::Array) for (auto i = Start; i < std::get<std::vector<Item>>(Value).size(); i++) if (Test(std::get<std::vector<Item>>(Value)[i])) return std::get<std::vector<Item>>(Value)[i];
			return jsElementType::Undefined;
		}

		int Item::findIndex(std::function<int(const Item&)> Test, size_t Start) {
			if (Type == JSON::jsElementType::Array) for (auto i = Start; i < std::get<std::vector<Item>>(Value).size(); i++) if (Test(std::get<std::vector<Item>>(Value)[i])) return i;
			return -1;
		}

		Item Item::sort(std::function<int(Item&, Item&)> Compare) {
			if (Type == JSON::jsElementType::Array) {
				Item ret = Copy();
				std::sort(std::get<std::vector<Item>>(ret.Value).begin(), std::get<std::vector<Item>>(ret.Value).end(), Compare);
				return ret;
			}
			return jsElementType::Undefined;
		}

		Item Item::Copy() {
			Item ret = Type;
			ret.Value = Value;
			return ret;
		}

		Item Item::DeepCopy() {
			switch (Type) {
				case PVX::JSON::jsElementType::Undefined:
				case PVX::JSON::jsElementType::Null:
				case PVX::JSON::jsElementType::Number:
				case PVX::JSON::jsElementType::String:
				case PVX::JSON::jsElementType::Boolean:
					return (*this);
				case PVX::JSON::jsElementType::Array:
					return map([](auto x) { return x.DeepCopy(); });
					break;
				case PVX::JSON::jsElementType::Object: {
					Item ret = Type;
					for (auto& [k, v]: std::get<std::map<std::wstring, Item>>(Value)) {
						ret[k] = v.DeepCopy();
					}
					return ret;
				}
			}
		}
		Item Item::DeepReducedCopy() {
			switch (Type) {
				case PVX::JSON::jsElementType::Undefined:
				case PVX::JSON::jsElementType::Null:
					return jsElementType::Undefined;
				case PVX::JSON::jsElementType::Number:
				case PVX::JSON::jsElementType::String:
				case PVX::JSON::jsElementType::Boolean:
					if (IsEmpty()) return jsElementType::Undefined;
					return (*this);
				case PVX::JSON::jsElementType::Array:
				{
					Item ret = jsElementType::Array;
					int count = 0;
					for (auto& x: std::get<std::vector<Item>>(Value)) {
						auto y = x.DeepReducedCopy();
						count += y.Type == jsElementType::Undefined;
						ret.push(y);
					}
					if (count == length()) return jsElementType::Undefined;
					return ret;
				}
				case PVX::JSON::jsElementType::Object: {
					Item ret = jsElementType::Object;
					for (auto& [k, v]: std::get<std::map<std::wstring, Item>>(Value)) {
						auto x = v.DeepReducedCopy();
						if (x.Type != jsElementType::Undefined)
							ret[k] = x;
					}
					return ret;
				}
			}
		}

		Item& Item::Cache() {
			return *cache;
		}

		Item& Item::Cache(const std::string& str) {
			cache = &((*this)[str]);
			return *cache;
		}

		Item& Item::Cache(const std::wstring& str) {
			cache = &((*this)[str]);
			return *cache;
		}

		Item& Item::Merge(const Item& With) {
			if (this->Type == jsElementType::Undefined) {
				(*this) = With;
				return *this;
			}
			if (this->Type == With.Type) {
				if (this->Type == JSON::jsElementType::Object) {
					for (auto& it : std::get<std::map<std::wstring, Item>>(With.Value))
						(*this)[it.first].Merge(it.second);
				} else if (this->Type == JSON::jsElementType::Array) {
					for (auto& it : std::get<std::vector<Item>>(With.Value))
						std::get<std::vector<Item>>(Value).push_back(it);
				}
			}
			return *this;
		}

		std::wstring _lvl(int l) {
			return L"\n" + std::wstring(l, L'\t');
		}

		std::wstring stringify(const Item& obj, int level, bool Format) {
			std::wstring Lvl = Format ? _lvl(level) : L"";
			std::wstring Lvl1 = Format ? _lvl(level + 1) : L"";
			std::wstring colon = Format ? L": " : L":";

			std::wstringstream ret;
			switch (obj.Type) {
				case jsElementType::Undefined:
					return L"undefined";
				case jsElementType::Null:
					return L"null";
				case jsElementType::Number:
					if (std::holds_alternative<double>(obj.Value)) {
						return std::to_wstring((long double)obj.Double());
					} else {
						return std::to_wstring(obj.Integer());
					}
				case jsElementType::Boolean:
					return obj.Boolean() ? L"true" : L"false";
				case jsElementType::String:
					return JsonString(obj.String());
				case jsElementType::Array:
					ret << "[";
					{
						auto& Array = std::get<std::vector<Item>>(obj.Value);
						if (Array.size()) {
							size_t i = 0;
							while (i< Array.size() && Array[i].Type==jsElementType::Undefined)i++;
							ret << Lvl1 << stringify(Array[i], level + 1); i++;
							while (i< Array.size() && Array[i].Type==jsElementType::Undefined)i++;
							for (; i < Array.size(); i++) {
								ret << "," << Lvl1 << stringify(Array[i], level + 1);
								while (i< Array.size() && Array[i].Type==jsElementType::Undefined)i++;
							}
						}
					}
					ret << Lvl << "]";
					return ret.str();
				case jsElementType::Object:
				{
					ret << "{";
					auto& Object = std::get<std::map<std::wstring, Item>>(obj.Value);
					auto iter = Object.begin();

					while (iter!=Object.end() && iter->second.Type == jsElementType::Undefined) iter++;

					if (iter != Object.end()) {
						ret << Lvl1 << JsonString(iter->first) << colon << stringify(iter->second, level + 1, Format);
						++iter;
					}

					for (; iter != Object.end(); ++iter) {
						if (iter->second.Type != jsElementType::Undefined)
							ret << "," << Lvl1 << JsonString(iter->first) << colon << stringify(iter->second, level + 1, Format);
					}

					ret << Lvl << "}";
					return ret.str();
				}
				default:
					return L"";
			}
		}

		std::wstring stringify(const Item& obj, bool Format) {
			return stringify(obj, 0, Format);
		}
		Item parse(const unsigned char* data, int size) {
			if (!size)return jsElementType::Undefined;
			return parse(Decode::UTF(data, size).c_str());
		}
		Item parse(const std::vector<unsigned char>& d) {
			return parse(d.data(), d.size());
		}


		struct jsonStack {
			char op;
			char Empty = 0;
			PVX::JSON::Item val;
			std::vector<jsonStack> Child;
		};
		static std::wregex jsonSpaces(LR"regex(\s+)regex", std::regex_constants::optimize);
		//static std::wregex jsonObjectStrings(LR"regex(\"((\\\"|[^\"])*)\")regex", std::regex_constants::optimize);
		static std::wregex jsonNulls(LR"regex(null)regex", std::regex_constants::optimize);



		void MakeObject(Item& obj, const jsonStack& s) {
			if (s.op == ':') {
				obj[s.Child[1].val.String()] = s.Child[0].val;
			} else if (s.op == ',') {
				for (auto& c : s.Child) MakeObject(obj, c);
			} else obj = jsElementType::Undefined;
		}
		void MakeArray(Item& obj, const jsonStack& s) {
			if (s.op == ',')
				for (long long i = long long(s.Child.size()) - 1; i >= 0; i--)
					MakeArray(obj, s.Child[i]);
			else obj.push(s.val);
		}

		std::wstring RemoveStrings(const std::wstring& txt, std::vector<std::wstring>& Strings) {
			std::wstring ret;
			int Escape = 0;
			long long Start = -1;
			int Out = 1;
			for (long long i = 0; i < txt.size(); i++) {
				auto c = txt[i];
				if (c == '\"' && !Escape) {
					if (Out) {
						//ret.pop_back();
						Start = i +1;
						Out = 0;
					} else {
						Strings.push_back(PVX::Decode::Unescape(txt.substr(Start, i - Start)));
						Out = 1;
					}
				}

				if (Out) ret.push_back(c);
				Escape = c == '\\';
			}
			return ret;
		}

		long long FindNum(std::wstring& txt, long long& cur, long long start) {
			if (cur==start)
				for (; txt[start]&&(txt[start]!='-'&&(txt[start]<L'0' || txt[start]>L'9')); start++, cur++);
			else
				for (; txt[start]&&(txt[start]!='-'&&(txt[start]<L'0' || txt[start]>L'9')); start++) {
					txt[cur++] = txt[start];
				}
			if (txt[start]) return start;
			//txt[cur] = 0;
			txt.resize(cur);
			return -1;
		}

		void removeNumbers(std::wstring& txt, std::vector<double>& Doubles, std::vector<long long>& Integers) {
			long long cur = 0;
			long long start = FindNum(txt, cur, 0);
			while (start != -1) {
				auto end = start + 1;
				while (txt[end]>=L'0' && txt[end]<=L'9') end++;
				if (txt[end]&&txt[end]==L'.') {
					end++;
					while (txt[end]>=L'0' && txt[end]<=L'9') end++;
					Doubles.push_back(_wtof(&txt[start]));
					txt[cur] = 'f';
				} else {
					Integers.push_back(_wtoi64(&txt[start]));
					txt[cur] = 'i';
				}
				cur++;

				start = FindNum(txt, cur, end);
			}
		}
		void removeSpaces(std::wstring& txt) {
			size_t j = 0;
			for (auto i = 0; i<txt.size(); i++) {
				if (txt[i]!=' '&&txt[i]!='\t'&&txt[i]!='\r'&&txt[i]!='\n') {
					txt[j++] = txt[i];
				}
			}
			txt.resize(j);
		}

		void removeBoolsAndNulls(std::wstring& txt) {
			std::vector<bool> ret;
			size_t j = 0;
			for (auto i = 0; i<txt.size(); i++) {
				if (txt[i] == 't') {
					ret.push_back(true);
					i += 3;
					txt[j++] = 't';
					continue;
				} else if (txt[i] == 'f') {
					ret.push_back(false);
					i += 4;
					txt[j++] = 'x';
					continue;
				} else if (txt[i] == 'n') {
					i += 3;
					txt[j++] = '0';
					continue;
				}
				txt[j++] = txt[i];
			}
			txt.resize(j);
		}

		Item parse2(const std::wstring& Json) {
			auto prec = [](int c) {
				switch (c) {
					case ',':
						return 10;
					case ':':
						return 20;
					case '[': case '{':
						return 0;
				}
				return -1;
			};
			std::vector<std::wstring> Strings;
			std::vector<long long> Integers;
			std::vector<double> Doubles;

			auto tmp = RemoveStrings(Json, Strings);
			removeSpaces(tmp);
			removeBoolsAndNulls(tmp);
			removeNumbers(tmp, Doubles, Integers);

			std::vector<jsonStack> Output, Stack2;
			Output.reserve(tmp.size());
			std::vector<char> Stack;
			int ItemCount = 0;
			int ints = 0, floats = 0, strings = 0;

			for (auto c : tmp) {
				char Empty = 0;
				switch (c) {
					case '"': Output.push_back({ 0, 0, Strings[strings++] }); ItemCount++; break;
					case 'i': Output.push_back({ 0, 0, Integers[ints++] }); ItemCount++; break;
					case 'f': Output.push_back({ 0, 0, Doubles[floats++] }); ItemCount++; break;
					case 't': Output.push_back({ 0, 0, true }); ItemCount++; break;
					case 'x': Output.push_back({ 0, 0, false }); ItemCount++; break;
					case '0': Output.push_back({ 0, 0, jsElementType::Null }); ItemCount++;  break;
					case '{': Stack.push_back('{'); ItemCount = 0; break;
					case '[': Stack.push_back('['); ItemCount = 0; break;
					case ']': {
						if (Stack.size() && Stack.back() == '[' && !ItemCount) Empty = 1;
						while (Stack.size() && Stack.back() != '[') {
							Output.push_back({ Stack.back(), 0 });
							Stack.pop_back();
						}
						if (!Stack.size()||Stack.back()!='[')
							return jsElementType::Undefined;
						Stack.pop_back();
						Output.push_back({ '[', Empty, jsElementType::Array });
						ItemCount++;
						break;
					}
					case '}': {
						if (Stack.size() && Stack.back() == '{' && !ItemCount) Empty = 1;
						while (Stack.size() && Stack.back() != '{') {
							Output.push_back({ Stack.back() });
							Stack.pop_back();
						}
						if (!Stack.size() || Stack.back() != '{')
							return jsElementType::Undefined;
						Stack.pop_back();
						Output.push_back({ '{', Empty,jsElementType::Object });
						ItemCount++;
						break;
					}
					default: {
						int p = prec(c);

						while (Stack.size() && prec(Stack.back()) > p) {
							Output.push_back({ Stack.back() });
							Stack.pop_back();
						}
						Stack.push_back(c);
						break;
					}

				}
			}
			while (Stack.size()) {
				Output.push_back({ Stack.back() });
				Stack.pop_back();
			}

			for (auto& s : Output) {
				if (s.op) {
					if (s.op == ':' || s.op == ',') {
						if (Stack2.size() >= 2) {
							s.Child.push_back(Stack2.back());
							Stack2.pop_back();
							s.Child.push_back(Stack2.back());
							Stack2.back() = s;
							continue;
						}
						return jsElementType::Undefined;
					} else {
						if (Stack2.size()) {
							if (s.op == '{') {
								if (!s.Empty) {
									MakeObject(s.val, Stack2.back());
									Stack2.back() = s;
								} else
									Stack2.push_back(s);
								continue;
							} else {
								if (!s.Empty) {
									MakeArray(s.val, Stack2.back());
									Stack2.back() = s;
								} else
									Stack2.push_back(s);
								continue;
							}
						}
						return jsElementType::Undefined;
					}
				} else {
					Stack2.push_back(s);
				}
			}
			if (Stack2.size() == 1) return Stack2[0].val;
			return jsElementType::Undefined;
		}
		Item parse(const std::wstring& Json) {
			std::vector<std::wstring> Strings;
			std::vector<long long> Integers;
			std::vector<double> Doubles;

			auto tmp = RemoveStrings(Json, Strings);
			removeSpaces(tmp);
			removeBoolsAndNulls(tmp);
			removeNumbers(tmp, Doubles, Integers);

			std::vector<jsonStack> Output, Stack2;

			std::function<void()> Dijkstra[128];
			{
				Output.reserve(tmp.size());
				int ItemCount = 0;
				int ints = 0, floats = 0, strings = 0;
				char Empty = 0;
				std::vector<unsigned char> Stack;
				int OK = 1;
				Dijkstra['"'] = [&]() { Output.push_back({ 0, 0, Strings[strings++] }); ItemCount++; };
				Dijkstra['i'] = [&]() { Output.push_back({ 0, 0, Integers[ints++] }); ItemCount++; };
				Dijkstra['f'] = [&]() { Output.push_back({ 0, 0, Doubles[floats++] }); ItemCount++; };
				Dijkstra['t'] = [&]() { Output.push_back({ 0, 0, true }); ItemCount++; };
				Dijkstra['x'] = [&]() { Output.push_back({ 0, 0, false }); ItemCount++; };
				Dijkstra['0'] = [&]() { Output.push_back({ 0, 0, jsElementType::Null }); ItemCount++; };
				Dijkstra['{'] = [&]() { Stack.push_back(25); ItemCount = 0; };
				Dijkstra['['] = [&]() { Stack.push_back(15); ItemCount = 0; };

				Dijkstra[']'] = [&]() {
					if (Stack.size() && Stack.back() == 15 && !ItemCount) Empty = 1;
					while (Stack.size() && Stack.back() != 15) {
						Output.push_back({ (char)Stack.back() });
						Stack.pop_back();
					}
					if (!Stack.size()||Stack.back()!=15) {
						OK = 0;
						return;
					}
					Stack.pop_back();
					Output.push_back({ '[', Empty, jsElementType::Array });
					ItemCount++;
				};
				Dijkstra['}'] = [&]() {
					if (Stack.size() && Stack.back() == 25 && !ItemCount) Empty = 1;
					while (Stack.size() && Stack.back() != 25) {
						Output.push_back({ (char)Stack.back() });
						Stack.pop_back();
					}
					if (!Stack.size() || Stack.back() != 25) {
						OK = 0;
						return;
					}
					Stack.pop_back();
					Output.push_back({ '{', Empty, jsElementType::Object });
					ItemCount++;
				};
				Dijkstra[':'] = [&]() {
					while (Stack.size() && Stack.back() > ':') {
						Output.push_back({ (char)Stack.back() });
						Stack.pop_back();
					}
					Stack.push_back(':');
				};
				Dijkstra[','] = [&]() {
					while (Stack.size() && Stack.back() > ',') {
						Output.push_back({ (char)Stack.back() });
						Stack.pop_back();
					}
					Stack.push_back(',');
				};

				for (auto c : tmp) {
					Empty = 0;
					Dijkstra[c]();
					if (OK) continue;
					return jsElementType::Undefined;
				}
				while (Stack.size()) {
					Output.push_back({ (char)Stack.back() });
					Stack.pop_back();
				}
			}

			for (auto& s : Output) {
				switch (s.op) {
					case 0:	Stack2.push_back(s); break;
					case ':':
					case ',': {
						if (Stack2.size() >= 2) {
							s.Child.push_back(Stack2.back());
							Stack2.pop_back();
							s.Child.push_back(Stack2.back());
							Stack2.back() = s;
							break;
						}
						return jsElementType::Undefined;
					}
					case '{': {
						if (Stack2.size()) {
							if (!s.Empty) {
								MakeObject(s.val, Stack2.back());
								Stack2.back() = s;
							} else
								Stack2.push_back(s);
							break;
						}
						return jsElementType::Undefined;
					}
					case '[': {
						if (Stack2.size()) {
							if (!s.Empty) {
								MakeArray(s.val, Stack2.back());
								Stack2.back() = s;
							} else
								Stack2.push_back(s);
							break;
						}
						return jsElementType::Undefined;
					}
				}
			}
			if (Stack2.size() == 1) return Stack2[0].val;
			return jsElementType::Undefined;
		}
	}
}