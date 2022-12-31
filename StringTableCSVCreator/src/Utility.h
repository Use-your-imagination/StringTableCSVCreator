#pragma once

#include <string>
#include <filesystem>

#include "JSONUtility.h"
#include "JSONParser.h"

namespace utility
{
	std::string getModuleNameLowerCase(const char* moduleName);

	std::string getModuleNameUpperCase(const char* moduleName);

	std::string fromUTF16ToUTF8(const std::filesystem::path& pathToProject);

	std::wstring fromUTF8ToUTF16(const std::string& data);

	std::wstring& replaceAllByRef(std::wstring& source, std::wstring_view oldValue, std::wstring_view newValue);

	json::utility::jsonObject makeObject(const std::string& key, const std::string& source, const std::string& translation);

	int save(const json::JSONParser& archive, const std::string& pathToArchive);

	std::string convertDescription(std::string_view description, bool isLocalized = false);
}
