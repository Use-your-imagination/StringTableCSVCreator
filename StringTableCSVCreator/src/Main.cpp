#include <iostream>
#include <format>
#include <fstream>

#include <Windows.h>

#include "Utility.h"
#include "JSONParser.h"
#include "JSONBuilder.h"
#include "RecursiveJSONIterator.h"

#pragma comment(lib, "Winmm.lib")

#define	EXPORT extern "C" __declspec(dllexport)

using namespace std;

EXPORT void createCSV(const char* inputModuleName, const char* inputModuleDescription, const char* inputPlatinumModuleDescription)
{
	string moduleName = utility::getModuleName(inputModuleName);

	if (!filesystem::exists("generated_csv"))
	{
		filesystem::create_directory("generated_csv");
	}

	ofstream(format("generated_csv\\{}.csv", moduleName)) << "Key,SourceString" << endl
		<< format(R"("{0}Name","{0}")", moduleName) << endl
		<< format(R"("{}Description","{}")", moduleName, utility::convertDescription(inputModuleDescription)) << endl
		<< format(R"("Platinum{}Description","{}")", moduleName, utility::convertDescription(inputPlatinumModuleDescription)) << endl;
}

EXPORT void createJSON(const char* inputModuleName, const char* inputModuleDescription, const char* inputPlatinumModuleDescription, const char* localizedModuleDescription, const char* localizedPlatinumModuleDescription)
{
	using namespace json::utility;

	string moduleName = utility::getModuleName(inputModuleName);
	json::JSONBuilder builder(CP_UTF8);
	vector<jsonObject> children;

	if (!filesystem::exists("generated_json"))
	{
		filesystem::create_directory("generated_json");
	}

	appendArray(utility::makeObject(moduleName, inputModuleName, inputModuleName), children);
	appendArray(utility::makeObject(moduleName + "Description", utility::convertDescription(inputModuleDescription), utility::convertDescription(localizedModuleDescription, true)), children);
	appendArray(utility::makeObject(format("Platinum{}Description", moduleName), utility::convertDescription(inputPlatinumModuleDescription), utility::convertDescription(localizedPlatinumModuleDescription, true)), children);

	builder["Namespace"] = moduleName;
	builder["Children"] = move(children);

	ofstream(format("generated_json\\{}.json", moduleName)) << builder;
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

	string moduleName = utility::getModuleName(inputModuleName);
	filesystem::path pathToArchive = pathToProject / "Content" / "Localization" / "Game" / "ru" / "Game.archive";
	json::JSONParser archive(utility::fromUTF16ToUTF8(pathToArchive));
	json::JSONParser module(ifstream(format("generated_json\\{}.json", moduleName)));

	for (const jsonObject& value : archive.getArray("Subnamespaces"))
	{
		const jsonObject& tem = get<jsonObject>(value.data[0].second);

		if (tem.getString("Namespace") == moduleName)
		{
			const_cast<jsonObject&>(tem) = module.getParsedData();

			break;
		}
	}

	return utility::save(archive, pathToArchive.string());
}

DWORD WINAPI playSound(LPVOID)
{
	string data = (ostringstream() << ifstream(LR"(C:\Users\semen\source\repos\StringTableCSVCreator\StringTableCSVCreator\WhyAmIHere.wav)", ios_base::binary).rdbuf()).str();

	PlaySoundA(data.data(), nullptr, SND_MEMORY);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL, NULL, playSound, NULL, NULL, NULL);
	}

	return true;
}
