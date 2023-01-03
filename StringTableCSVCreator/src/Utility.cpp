#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include "Utility.h"

#include <fstream>
#include <codecvt>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <unordered_set>

using namespace std;

namespace utility
{
	string getModuleNameLowerCase(const char* moduleName)
	{
		string result(moduleName);

		ranges::for_each(result, [](char& c) { c = tolower(c); });

		ranges::replace(result, ' ', '_');

		return result;
	}

	string getModuleNameUpperCase(const char* moduleName)
	{
		string result(moduleName);

		while (result.contains(' '))
		{
			size_t position = result.find(' ');

			try
			{
				char& nextChar = result.at(position + 1);

				nextChar = toupper(nextChar);
			}
			catch (const out_of_range&)
			{

			}

			result.erase(result.begin() + position);
		}

		return result;
	}

	string fromUTF16ToUTF8(const filesystem::path& pathToProject)
	{
		string bytes = (ostringstream() << ifstream(pathToProject, ios::binary).seekg(2).rdbuf()).str();
		wstring data;
		size_t len = bytes.size();

		if (len & 1)
		{
			len--;
		}

		for (size_t i = 0; i < len;)
		{
			// little-endian
			int lo = bytes[i++] & 0xFF;
			int hi = bytes[i++] & 0xFF;

			data.push_back(hi << 8 | lo);
		}

		return wstring_convert<codecvt_utf8_utf16<wchar_t>>().to_bytes(data);
	}

	wstring fromUTF8ToUTF16(const string& data)
	{
		return wstring_convert<codecvt_utf8_utf16<wchar_t>>().from_bytes(data);
	}

	wstring& replaceAllByRef(wstring& source, wstring_view oldValue, wstring_view newValue)
	{
		boyer_moore_horspool_searcher oldValueSearcher(oldValue.begin(), oldValue.end());

		while (true)
		{
			auto it = search(source.begin(), source.end(), oldValueSearcher);

			if (it == source.end())
			{
				break;
			}

			source.replace(it, it + oldValue.size(), newValue);
		}

		return source;
	}

	json::utility::jsonObject makeObject(const string& key, const string& source, const string& translation)
	{
		json::utility::jsonObject result;
		auto makeTextObject = [](const string& text) -> json::utility::jsonObject
		{
			json::utility::jsonObject result;

			result.data.push_back({ "Text", text });

			return result;
		};

		result.data.push_back({ "Source", makeTextObject(source) });

		result.data.push_back({ "Translation", makeTextObject(translation) });

		result.data.push_back({ "Key", key });

		return result;
	}

	int save(const json::JSONParser& archive, const string& pathToArchive)
	{
		wstring data = utility::fromUTF8ToUTF16((ostringstream() << archive).str());
		FILE* ptr = nullptr;

		utility::replaceAllByRef(data, L"\r\n", L"\n");

		fopen_s(&ptr, pathToArchive.data(), "w, ccs=UTF-16LE");

		if (!ptr)
		{
			cout << "Can't open file " << pathToArchive << endl;

			return 3;
		}

		fwrite(data.data(), sizeof(wchar_t), data.size(), ptr);

		fclose(ptr);

		return 0;
	}

	string convertDescription(string_view description, bool isLocalized)
	{
		static const unordered_map<char, pair<string, char>> tags =
		{
			{ 'a', { "<add>", '+' } },
			{ 's', { "<subtract>", '-' } },
			{ 'A', { "<additional>", '+' } },
			{ 'S', { "<substractional>", '-' } },
			{ 'i', { "<increase>", '+' } },
			{ 'r', { "<reduce>", '-' } },
			{ 'm', { "<more>", '+' } },
			{ 'l', { "<less>", '-' } }
		};
		static const unordered_set<char> numericTags =
		{
			'a',
			'A',
			's',
			'S'
		};
		string result;
		size_t currentIndex = 0;
		string_view currentFormat;
		auto replace = [&result, &currentIndex, &currentFormat, isLocalized]()
		{
			string replaceString;

			if (currentFormat.contains('t'))
			{
				if (currentFormat.contains('c'))
				{
					replaceString = format("{}{}c{} {}", '{', currentIndex++, '}', isLocalized ? json::utility::toUTF8JSON("сек", 1251) : "sec");
				}
				else
				{
					replaceString = format("{}{}{} {}", '{', currentIndex++, '}', isLocalized ? json::utility::toUTF8JSON("сек", 1251) : "sec");
				}
			}
			else
			{
				bool containsN = currentFormat.contains('n') || (!currentFormat.contains('n') && ranges::any_of(currentFormat, [](char c) { return numericTags.contains(c); }));

				for (char c : currentFormat)
				{
					if (!isalpha(c))
					{
						continue;
					}

					auto it = tags.find(c);

					if (it != tags.end())
					{
						const auto& [tag, symbol] = (it->second);

						if (currentFormat.contains('n'))
						{
							if (currentFormat.contains('c'))
							{
								replaceString.insert(0, format("{}{}{}{}c{}</>", tag, symbol, '{', currentIndex++, '}'));
							}
							else
							{
								replaceString.insert(0, format("{}{}{}{}{}</>", tag, symbol, '{', currentIndex++, '}'));
							}
						}
						else
						{
							if (currentFormat.contains('c'))
							{
								replaceString.insert(0, format("{}{}{}{}c{}%</>", tag, symbol, '{', currentIndex++, '}'));
							}
							else
							{
								replaceString.insert(0, format("{}{}{}{}{}%</>", tag, symbol, '{', currentIndex++, '}'));
							}
						}
					}
					else if (c == 'n' || c == 'c')
					{
						continue;
					}
					else
					{
						replaceString += c;
					}
				}

				if (replaceString.empty())
				{
					if (containsN)
					{
						if (currentFormat.contains('c'))
						{
							replaceString = format("{}{}c{}", '{', currentIndex++, '}');
						}
						else
						{
							replaceString = format("{}{}{}", '{', currentIndex++, '}');
						}
					}
					else
					{
						if (currentFormat.contains('c'))
						{
							replaceString = format("{}{}c{}%", '{', currentIndex++, '}');
						}
						else
						{
							replaceString = format("{}{}{}%", '{', currentIndex++, '}');
						}
					}
				}
			}

			result += replaceString;
		};

		result.reserve(description.size());

		for (size_t i = 0; i < description.size();)
		{
			switch (description[i])
			{
			case '{':
				currentFormat = string_view(description.begin() + i, description.begin() + description.find('}', i) + 1);

				replace();

				i += currentFormat.size();

				break;

			default:
				result += description[i++];
			}
		}

		return result;
	}
}
