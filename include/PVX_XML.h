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
		std::map<std::wstring, std::wstring> Attributes;
		std::vector<Element> Child;
		static Element Parse(const std::wstring& Text);
	};
}