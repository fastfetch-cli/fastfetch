#include "fastfetch.h"


/**
 * @brief Retrieves a specific version string for a Windows file.
 *
 * This function gets a version string from a Windows file's version information.
 *
 * @param filePath The path to the file for which version information is requested.
 * @param stringName The name of the specific version string to retrieve (e.g., "FileVersion", "ProductVersion").
 * @param version Pointer to an FFstrbuf where the version string will be stored.
 *
 * @return true if the version string was successfully retrieved, false otherwise.
 */
bool ffGetFileVersion(const wchar_t* filePath, const wchar_t* stringName, FFstrbuf* version);
