#include <PVX_XML.h>
#include <PVX_String.h>
#include <PVX_Regex.h>
#include <PVX_Encode.h>

namespace PVX::XML {
	std::vector<std::wstring> RemoveStrings(std::wstring& txt) {
		std::wstring ret;
		std::vector<std::wstring> Strings;
		int Escape = 0;
		int Start = -1;
		int Out = 1;
		for (auto i = 0; i < txt.size(); i++) {
			auto c = txt[i];
			if (c == '\"' && !Escape) {
				if (Out) {
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
		txt = ret;
		return Strings;
	}
	std::vector<std::wstring> RemoveCDATA(std::wstring& ret, int& Error) {
		std::vector<std::wstring> CDATA;
		auto Start = ret.find(L"<![CDATA[");
		while (Start != std::wstring::npos) {
			Start += 9;
			auto End = ret.find(L"]]>");
			if (End==std::wstring::npos) { Error++; break; }
			CDATA.push_back(ret.substr(Start, End-Start));
			ret.replace(Start-9, End - Start + 12, L"<!-->");
			Start = ret.find(L"<![CDATA[", Start - (9 - 5));
		}
		return CDATA;
	}
	std::wstring RemoveComments(const std::wstring& txt, int& Error) {
		std::wstring ret = txt;
		auto Start = ret.find(L"<!--");
		while (Start != std::wstring::npos) {
			Start += 4;
			auto End = ret.find(L"-->");
			if (End==std::wstring::npos) { Error++; break; }
			ret.replace(Start-4, End - Start + 7, L"");
			Start = ret.find(L"<!--", Start - 4);
		}
		return ret;
	}
	void removeHead(std::wstring& txt, int& Error) {
		size_t c = 0;
		for (; c<txt.size()&&(txt[c]==L' '||txt[c]==L'\t'||txt[c]==L'\r'||txt[c]==L'\n'); c++);
		if (c<(txt.size()-2) && c == txt.find(L"<?xml")) {
			auto end = txt.find(L"?>", c);
			if (end!=std::wstring::npos) {
				txt.replace(c, end - c + 2, L"");
				return;
			}
			Error++;
		}
	}

	//const std::wregex jsonSpaces(LR"regex(\s+)regex", std::regex_constants::optimize);
	//const std::wregex jsonObjectStrings(LR"regex(\"((\\\"|[^\"])*)\")regex", std::regex_constants::optimize);
	
	const std::wregex  openTag(LR"regex(\s*<([a-zA-Z][-a-zA-Z0-9]*)(\s+([^>/]*))?>)regex", std::regex_constants::optimize);
	const std::wregex  fullTag(LR"regex(\s*<([a-zA-Z][-a-zA-Z0-9]*)(\s+([^>]*))?/>)regex", std::regex_constants::optimize);
	const std::wregex closeTag(LR"regex(\s*</([a-zA-Z][-a-zA-Z0-9]*)\s*>)regex", std::regex_constants::optimize);
	const std::wregex   rCDATA(LR"regex(\s*<!-->)regex", std::regex_constants::optimize);
	const std::wregex  attribs(LR"regex(([a-zA-Z][-a-zA-Z0-9]*)\s*(=\s*"\s)?)regex", std::regex_constants::optimize);

	Element Element::Parse(const std::wstring& Text) {
		int Error = 0;
		std::wstring txt = RemoveComments(Text, Error);
		removeHead(txt, Error);
		auto CDATA = RemoveCDATA(txt, Error);
		auto NextCDATA = 0;
		auto Strings = RemoveStrings(txt);
		auto NextString = 0;
		//txt = std::regex_replace(txt, jsonSpaces, L" ");

		size_t cur = 0;
		std::wsmatch match;

		std::vector<Element> tags;

		while (cur < txt.size()) {
			if (std::regex_search(txt.cbegin()+cur, txt.cend(), match, openTag, std::regex_constants::match_continuous)) {
				std::map<std::wstring, std::wstring> attrs;
				if (match.size()>3) {
					PVX::onMatch(match[3].str(), attribs, [&](const std::wsmatch& m) { attrs[m[1]] = m.size()>2? Strings[NextString++]:L""; });
				}

				tags.push_back({ PVX::XML::Element::ElementType::OpenTag, match[1].str(), attrs });
				cur += match.str().size();
			} else if (std::regex_search(txt.cbegin()+cur, txt.cend(), match, fullTag, std::regex_constants::match_continuous)) {
				std::map<std::wstring, std::wstring> attrs;
				if (match.size()>3) {
					PVX::onMatch(match[3].str(), attribs, [&](const std::wsmatch& m) {
						if (m.size()>2)
							attrs[m[1]] = Strings[NextString++];
						else
							attrs[m[1]] = L"";
					});
				}

				tags.push_back({ PVX::XML::Element::ElementType::Tag, match[1].str(), attrs });
				cur += match.str().size();
			} else if (std::regex_search(txt.cbegin()+cur, txt.cend(), match, closeTag, std::regex_constants::match_continuous)) {
				tags.push_back({ PVX::XML::Element::ElementType::CloseTag, match[1].str() });
				cur += match.str().size();
			} else if(std::regex_search(txt.cbegin()+cur, txt.cend(), match, rCDATA, std::regex_constants::match_continuous)){
				tags.push_back({ PVX::XML::Element::ElementType::CDATA, CDATA[NextCDATA++] });
				cur += match.str().size();
			} else {
				auto end = txt.find(L"<", cur);
				if (end==std::wstring::npos) end = txt.size();
				tags.push_back({ PVX::XML::Element::ElementType::Text, txt.substr(cur, end-cur) });
				cur = end;
			}
		}

		return Element();
	}
}