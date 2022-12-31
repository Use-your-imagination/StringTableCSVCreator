#include <iostream>
#include <format>
#include <fstream>

#include <Windows.h>

#include "Utility.h"
#include "JSONParser.h"
#include "JSONBuilder.h"
#include "RecursiveJSONIterator.h"
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

#define	EXPORT extern "C" __declspec(dllexport)

using namespace std;

EXPORT void createCSV(const char* inputModuleName, const char* inputModuleDescription, const char* inputPlatinumModuleDescription)
{
	string lower = utility::getModuleNameLowerCase(inputModuleName);
	string upper = utility::getModuleNameUpperCase(inputModuleName);

	if (!filesystem::exists("generated_csv"))
	{
		filesystem::create_directory("generated_csv");
	}

	ofstream(format("generated_csv\\{}.csv", lower)) << "Key,SourceString" << endl
		<< format(R"("{}Name","{}")", upper, lower) << endl
		<< format(R"("{}Description","{}")", upper, utility::convertDescription(inputModuleDescription)) << endl
		<< format(R"("Platinum{}Description","{}")", upper, utility::convertDescription(inputPlatinumModuleDescription)) << endl;
}

EXPORT void createJSON(const char* inputModuleName, const char* inputModuleDescription, const char* inputPlatinumModuleDescription, const char* localizedModuleDescription, const char* localizedPlatinumModuleDescription)
{
	using namespace json::utility;

	string lower = utility::getModuleNameLowerCase(inputModuleName);
	string upper = utility::getModuleNameUpperCase(inputModuleName);
	json::JSONBuilder builder(CP_UTF8);
	vector<jsonObject> children;

	if (!filesystem::exists("generated_json"))
	{
		filesystem::create_directory("generated_json");
	}

	appendArray(utility::makeObject(format("{}Name", upper), lower, lower), children);
	appendArray(utility::makeObject(upper + "Description", utility::convertDescription(inputModuleDescription), utility::convertDescription(localizedModuleDescription, true)), children);
	appendArray(utility::makeObject(format("Platinum{}Description", upper), utility::convertDescription(inputPlatinumModuleDescription), utility::convertDescription(localizedPlatinumModuleDescription, true)), children);

	builder["Namespace"] = upper;
	builder["Children"] = move(children);

	ofstream(format("generated_json\\{}.json", lower)) << builder;
}

EXPORT int applyLocalization(const char* inputModuleName)
{
	using json::utility::jsonObject;

	if (!filesystem::exists("path_to_project.txt"))
	{
		cout << "path_to_project.txt doesn't exist" << endl;

		return 1;
	}

	filesystem::path pathToProject = (ostringstream() << ifstream("path_to_project.txt").rdbuf()).str();

	if (!filesystem::exists(pathToProject))
	{
		cout << format(R"(Path "{}" doesn't exist)", pathToProject.string()) << endl;

		return 2;
	}

	string lower = utility::getModuleNameLowerCase(inputModuleName);
	string upper = utility::getModuleNameUpperCase(inputModuleName);
	filesystem::path pathToArchive = pathToProject / "Content" / "Localization" / "Game" / "ru" / "Game.archive";
	json::JSONParser archive(utility::fromUTF16ToUTF8(pathToArchive));
	json::JSONParser module(ifstream(format("generated_json\\{}.json", lower)));

	for (const jsonObject& value : archive.getArray("Subnamespaces"))
	{
		const jsonObject& tem = get<jsonObject>(value.data[0].second);

		if (tem.getString("Namespace") == upper)
		{
			const_cast<jsonObject&>(tem) = module.getParsedData();

			break;
		}
	}

	return utility::save(archive, pathToArchive.string());
}

DWORD playSound(LPVOID module)
{
	PlaySoundW(MAKEINTRESOURCE(IDR_WAVE1), static_cast<HMODULE>(module), SND_RESOURCE);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// CreateThread(NULL, NULL, playSound, hinstDLL, NULL, NULL);
	}

	return true;
}
