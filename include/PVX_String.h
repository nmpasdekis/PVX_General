#ifndef __PVX_STD_STRING_H__
#define __PVX_STD_STRING_H__
#include <string>
#include <vector>
#include<functional>
#include <sstream>
#include <regex>

namespace PVX{
	namespace String{
		inline void OnSplit(const std::string& Text, const std::string& Separator, std::function<void(const std::string&)> clb) {
			size_t ssz = Separator.size(), last = 0;
			int start = 0;
			while (-1 != (start = Text.find(Separator, start))) {
				if (((unsigned int)start) >= last)
					clb(Text.substr(last, start - last));
				start += ssz;
				last = start;
			}
			if (last <= Text.size())
				clb(Text.substr(last, Text.size() - last));
		}

		inline void OnSplit(const std::wstring& Text, const std::wstring& Separator, std::function<void(const std::wstring&)> clb) {
			size_t ssz = Separator.size(), last = 0;
			int start = 0;
			while (-1 != (start = Text.find(Separator, start))) {
				if (((unsigned int)start) >= last)
					clb(Text.substr(last, start - last));
				start += ssz;
				last = start;
			}
			if (last <= Text.size())
				clb(Text.substr(last, Text.size() - last));
		}

		std::vector<std::string> Split(const std::string & Text, const std::string & Separator);
		std::vector<std::wstring> Split(const std::wstring & Text, const std::wstring & Separator);
		std::vector<std::string> Split_Trimed(const std::string & Text, const std::string & Separator);
		std::vector<std::wstring> Split_Trimed(const std::wstring & Text, const std::wstring & Separator);
		std::string Join(const std::vector<std::string> & List, const std::string & separator);
		std::wstring Join(const std::vector<std::wstring> & List, const std::wstring & separator);
		std::vector<std::string> Split_No_Empties(const std::string & Text, const std::string & Separator);
		std::vector<std::wstring> Split_No_Empties(const std::wstring & Text, const std::wstring & Separator);
		std::vector<std::string> Split_No_Empties_Trimed(const std::string & Text, const std::string & Separator);
		std::vector<std::wstring> Split_No_Empties_Trimed(const std::wstring & Text, const std::wstring & Separator);
		std::string Trim(const std::string & s);
		std::wstring Trim(const std::wstring & s);

		std::string Replace(const std::string & Text, const std::regex & pattern, std::function<const std::string(const std::smatch&)> newWordFnc);
		std::string Replace(const std::string & Text, const std::string & pattern, std::function<const std::string(const std::smatch&)> newWordFnc);

		std::wstring Replace(const std::wstring& Text, const std::wregex& pattern, std::function<const std::wstring(const std::wsmatch&)> newWordFnc);
		std::wstring Replace(const std::wstring& Text, const std::wstring& pattern, std::function<const std::wstring(const std::wsmatch&)> newWordFnc);


		std::string Replace(const std::string& Text, const std::string& pattern, const std::string& newWord);
		std::wstring Replace(const std::wstring& Text, const std::wstring& pattern, const std::wstring& newWord);

		std::string ReplaceOne(const std::string& Text, const std::string& pattern, const std::string& newWord);
		std::wstring ReplaceOne(const std::wstring& Text, const std::wstring& pattern, const std::wstring& newWord);


		std::string ToLower(const std::string & txt);
		std::wstring ToLower(const std::wstring & txt);

		std::pair<std::string, std::string> Split2(const std::string& Text, const std::string& Separator);
		std::pair<std::wstring, std::wstring> Split2(const std::wstring& Text, const std::wstring& Separator);
		std::pair<std::string, std::string> Split2_Trimed(const std::string& Text, const std::string& Separator);
		std::pair<std::wstring, std::wstring> Split2_Trimed(const std::wstring& Text, const std::wstring& Separator);


		//template<typename T, typename T2>
		//inline std::wstring Join(const std::vector<T> & List, const std::wstring & separator, T2 fnc) {
		//	std::wstringstream ret;
		//	ret << fnc(List[0]);
		//	for (auto i = 1; i < List.size(); i++)
		//		ret << separator << fnc(List[i]);
		//	return ret.str();
		//}

		template<typename T, typename T2>
		inline std::wstring Join(const T & List, const std::wstring & separator, T2 fnc) {
			if (!List.size()) return L"";
			std::wstringstream ret;

			auto iter = List.begin();

			ret << fnc(*iter);
			iter++;
			for (; iter != List.end(); iter++)
				ret << separator << fnc(*iter);
			return ret.str();
		}

		template<typename T, typename T2>
		inline std::string Join(const std::vector<T> & List, const std::string & separator, T2 fnc) {
			std::stringstream ret;
			ret << fnc(List[0]);
			for (auto i = 1; i < List.size(); i++)
				ret << separator << fnc(List[i]);
			return ret.str();
		}
	}
}

#endif