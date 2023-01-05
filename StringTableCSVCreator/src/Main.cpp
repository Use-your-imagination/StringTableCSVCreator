#include <iostream>
#include <format>
#include <fstream>
#include <sstream>

#include "Utility.h"
#include "JSONParser.h"
#include "JSONBuilder.h"
#include "RecursiveJSONIterator.h"

#define	EXPORT extern "C" __declspec(dllexport)

using namespace std;

EXPORT void createCSV(const char* moduleName, const char* moduleDescription, const char* platinumModuleDescription)
{
	string lower = utility::getModuleNameLowerCase(moduleName);
	string upper = utility::getModuleNameUpperCase(moduleName);
	ostringstream platinumDescription;

	platinumDescription << moduleDescription << endl << endl << platinumModuleDescription;

	if (!filesystem::exists("generated_csv"))
	{
		filesystem::create_directory("generated_csv");
	}

	ofstream(format("generated_csv\\{}.csv", lower)) << "Key,SourceString" << endl
		<< format(R"("{}Name","{}")", upper, lower) << endl
		<< format(R"("{}Description", "{}")", upper, utility::convertDescription(moduleDescription)) << endl
		<< format(R"("Platinum{}Description","{}")", upper, utility::convertDescription(platinumDescription.str())) << endl;
}

EXPORT void createJSON(const char* moduleName, const char* moduleDescription, const char* platinumModuleDescription, const char* localizedModuleDescription, const char* localizedPlatinumModuleDescription)
{
	using namespace json::utility;

	string lower = utility::getModuleNameLowerCase(moduleName);
	string upper = utility::getModuleNameUpperCase(moduleName);
	ostringstream platinumDescription;
	ostringstream platinumLocalizedDescription;
	json::JSONBuilder builder(CP_UTF8);
	vector<jsonObject> children;

	platinumDescription << moduleDescription << endl << endl << platinumModuleDescription;
	platinumLocalizedDescription << localizedModuleDescription << endl << endl << localizedPlatinumModuleDescription;

	if (!filesystem::exists("generated_json"))
	{
		filesystem::create_directory("generated_json");
	}

	appendArray(utility::makeObject(format("{}Name", upper), lower, lower), children);
	appendArray(utility::makeObject(upper + "Description", utility::convertDescription(moduleDescription), utility::convertDescription(localizedModuleDescription, true)), children);
	appendArray(utility::makeObject(format("Platinum{}Description", upper), utility::convertDescription(platinumDescription.str()), utility::convertDescription(platinumLocalizedDescription.str(), true)), children);

	builder["Namespace"] = upper;
	builder["Children"] = move(children);

	ofstream(format("generated_json\\{}.json", lower)) << builder;
}

EXPORT int applyLocalization(const char* moduleName)
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

	string lower = utility::getModuleNameLowerCase(moduleName);
	string upper = utility::getModuleNameUpperCase(moduleName);
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
