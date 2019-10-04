#pragma once
#include <string>
#include <vector>
#include <map>

namespace PVX::XML {
	class Element {
	public:
		enum class ElementType {
			Tag, Text, CDATA, Pre, Input, OpenTag, CloseTag
		};
		ElementType Type;
		std::wstring Name;
		std::wstring Text;
		std::map<std::wstring, std::wstring> Attributes;
		std::vector<Element> Child;
	};
	Element Parse(const std::wstring& Text);
	std::wstring Serialize(const Element& xml);
}