#ifndef HELPERS_H
#define HELPERS_H

#include <istream>
#include <string>

namespace Helpers
{

static const char cPathSeparator = '/';
static const char cWindowsPathSeparator = '\\';
bool readLine(std::string& line, std::istream& stream);
bool getRelativePath(std::string& finalPath, const std::string& path,
	const std::string& trimFront);
std::string getPlaylistPath(const std::string& relativePath,
	const std::string& prefix, bool windowsSeparators);
std::string convertSeparators(const std::string& path);

}

#endif
