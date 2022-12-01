#include <iostream>
#include <format>
#include <fstream>

#include "Utility.h"
#include "JSONParser.h"
#include "JSONBuilder.h"
#include "RecursiveJSONIterator.h"

#define	EXPORT extern "C" __declspec(dllexport)

using namespace std;

EXPORT void createCSV(const char* inputModuleName, const char* inputModuleDescription, const char* inputPlatinumModuleDescription)
{
	string moduleName = utility::getModuleName(inputModuleName);

	ofstream(moduleName + ".csv") << "Key,SourceString" << endl
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

	if (!filesystem::exists("localization_files"))
	{
		filesystem::create_directory("localization_files");
	}

	appendArray(utility::makeObject(moduleName, inputModuleName, inputModuleName), children);
	appendArray(utility::makeObject(moduleName + "Description", utility::convertDescription(inputModuleDescription), utility::convertDescription(localizedModuleDescription, true)), children);
	appendArray(utility::makeObject(format("Platinum{}Description", moduleName), utility::convertDescription(inputPlatinumModuleDescription), utility::convertDescription(localizedPlatinumModuleDescription, true)), children);

	builder["Namespace"] = moduleName;
	builder["Children"] = move(children);

	ofstream(format("localization_files\\{}.json", moduleName)) << builder;
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
	json::JSONParser module(ifstream(format("localization_files\\{}.json", moduleName)));

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
