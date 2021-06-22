#include "Helpers.h"
#include <openssl/md5.h>
#include <boost/filesystem.hpp>
#include <cctype>
#include <cstdio>
#include <cassert>

namespace Helpers
{

bool readLine(std::string& line, std::istream& stream)
{
	line.clear();
	const unsigned int cBufferSize = 1024;
	char buffer[cBufferSize];
	stream.getline(buffer, cBufferSize);
	if (stream.eof() || !stream.fail())
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
		if (stream.eof() || !stream.fail())
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
	finalPath = path.substr(trimFront.size());
	while (!finalPath.empty() && *finalPath.begin() == cPathSeparator)
		finalPath.erase(finalPath.begin());
	return true;
}

static bool isValid(char c, bool noUnicode)
{
	const char cReserved[] = {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};

	if (noUnicode && !std::isprint(c))
		return false;
	for (std::size_t i = 0; i < sizeof(cReserved)/sizeof(char); ++i)
	{
		if (c == cReserved[i])
			return false;
	}
	return true;
}

std::string repairFilename(const std::string& path, bool noUnicode)
{
	boost::filesystem::path origPath = path;
	std::string fileName = origPath.filename().string();

	unsigned int numInvalid = 0;
	for (std::string::const_iterator iter = fileName.begin();
		iter != fileName.end(); ++iter)
	{
		if (!isValid(*iter, noUnicode))
			++numInvalid;
	}

	if (numInvalid == 0)
		return path;

	const float cMaxToReplace = 0.25f;
	unsigned int nameLength = (unsigned int)(fileName.size() -
		origPath.extension().native().size());

	if (float(numInvalid)/nameLength <= cMaxToReplace)
	{
		const char cReplaceChar = '_';
		for (std::string::iterator iter = fileName.begin();
			iter != fileName.end(); ++iter)
		{
			if (!isValid(*iter, noUnicode))
				*iter = cReplaceChar;
		}
	}
	else
	{
		const unsigned int cBufferSize = 256;
		char buffer[cBufferSize];
		unsigned char hash[MD5_DIGEST_LENGTH];
		MD5((const unsigned char*)path.c_str(), path.size(), hash);

		//2 hex digits per byte
		char hashString[MD5_DIGEST_LENGTH*2 + 1];
		for (unsigned int i = 0; i < MD5_DIGEST_LENGTH; ++i)
			std::snprintf(hashString + i*2, 3, "%02X", hash[i]);

		int result = snprintf(buffer, cBufferSize, "%s%s", hashString,
			origPath.extension().string().c_str());
		assert(result >= 0 && result < cBufferSize);
		fileName = buffer;
	}
	return (origPath.parent_path()/fileName).string();
}

std::string getPlaylistSongPath(const std::string& relativePath,
	const std::string& prefix, bool windowsSeparators)
{
	boost::filesystem::path finalPath = prefix;
	finalPath /= relativePath;
	std::string retVal = finalPath.string();
	if (windowsSeparators)
	{
		for (std::string::iterator iter = retVal.begin();
			iter != retVal.end(); ++iter)
		{
			if (*iter == cPathSeparator)
				*iter = cWindowsPathSeparator;
		}
	}
	return retVal;
}

bool processPath(std::string& finalPath, const std::string& path,
	const std::string& trimFront, const std::string& newFront,
	bool windowsSeparators)
{
	if (!trimFront.empty() && path.find(trimFront) != 0)
		return false;
	std::string trimmed = path.substr(trimFront.size());
	finalPath = (boost::filesystem::path(newFront)/trimmed).string();
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
