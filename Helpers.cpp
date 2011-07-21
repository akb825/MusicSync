#include "Helpers.h"
#include <boost/filesystem.hpp>

namespace Helpers
{

bool readLine(std::string& line, std::istream& stream)
{
	line.clear();
	const unsigned int cBufferSize = 1024;
	char buffer[cBufferSize];
	stream.getline(buffer, cBufferSize);
	if (!stream.fail())
	{
		line = buffer;
		return true;
	}
	else if (stream.bad())
		return false;

	unsigned int currBufferSize = cBufferSize;
	do
	{
		currBufferSize *= 2;
		char* dynBuffer = new char[currBufferSize];
		stream.getline(buffer, cBufferSize);
		if (!stream.fail())
		{
			line = buffer;
			return true;
		}
		else if (stream.bad())
			return false;
	} while (true);

	//Shouldn't reach here.
	return false;
}

bool getRelativePath(std::string& finalPath, const std::string& path,
	const std::string& trimFront)
{
	if (!trimFront.empty() && path.find(trimFront) != 0)
		return false;
	finalPath = path.substr(trimFront.substr(trimFront.size()));
}

bool processPath(std::string& finalPath, const std::string& path,
	const std::string& trimFront, const std::string& newFront,
	bool windowsSeparators)
{
	if (!trimFront.empty() && path.find(trimFront) != 0)
		return false;
	std::string trimmed = path.substr(trimFront.size());
	finalPath = (boost::filesystem::path(newFront)/trimmed).native();
	return true;
}

std::string convertSeprators(const std::string& path)
{
	std::string finalPath = path;
	for (std::string::iterator iter = finalPath.begin();
		iter != finalPath.end(); ++iter)
	{
		if (*iter == cPathSeparator)
			*iter = cWindowsPathSeparator;
	}
	return path;
}

}
