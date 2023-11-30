/** @file */

#pragma once

#define	EXPORT extern "C" __declspec(dllexport)

/**
 * @brief Create CSV file for Unreal Engine StringTable import
 * @param moduleName Module name
 * @param moduleDescription Module description
 * @param platinumModuleDescription Platinum module description
*/
EXPORT void createCSV(const char* moduleName, const char* moduleDescription, const char* platinumModuleDescription);

/**
 * @brief Create JSON file for Unreal Engine localization
 * @param moduleName Module name
 * @param moduleDescription Module description
 * @param platinumModuleDescription Platinum module description
 * @param localizedModuleDescription RU localization module description
 * @param localizedPlatinumModuleDescription RU localization platinum module description
*/
EXPORT void createJSON(const char* moduleName, const char* moduleDescription, const char* platinumModuleDescription, const char* localizedModuleDescription, const char* localizedPlatinumModuleDescription);

/**
 * @brief Modify Unreal Engine localization files
 * @param moduleName Module name
 * @return Error code(1 - path_to_project.txt doesn't exist, 2 - path to project doesn't exist, 3 - Can't open Unreal Engine archive file with localization)
*/
EXPORT int applyLocalization(const char* moduleName);
