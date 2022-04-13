/*
 * Copyright 2011-2022 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Helpers.h"

#include "md5.h"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <filesystem>

namespace Helpers
{

bool readLine(std::string& line, std::istream& stream)
{
	line.clear();
	do
	{
		int c = stream.get();
		if (!stream.good())
			break;

		if (c == '\r')
			continue;
		else if (c == '\n')
			return true;
		line.push_back(static_cast<char>(c));
	} while (true);
	return stream.eof();
}

bool getRelativePath(std::string& finalPath, const std::string& path,
	const std::string& trimFront)
{
	if (!trimFront.empty() && path.find(trimFront) != 0)
		return false;
	finalPath = path.substr(trimFront.size());
	while (!finalPath.empty() && finalPath.front() == cPathSeparator)
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
	std::filesystem::path origPath = path;
	std::string fileName = origPath.filename().string();

	unsigned int numInvalid = 0;
	for (char c : fileName)
	{
		if (!isValid(c, noUnicode))
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
		for (char& c : fileName)
		{
			if (!isValid(c, noUnicode))
				c = cReplaceChar;
		}
	}
	else
		fileName = MD5(path).hexdigest() + origPath.extension().string();
	return (origPath.parent_path()/fileName).string();
}

std::string getPlaylistSongPath(const std::string& relativePath, const std::string& prefix,
	bool windowsSeparators)
{
	std::filesystem::path finalPath = prefix;
	finalPath /= relativePath;
	std::string retVal = finalPath.string();
	if (windowsSeparators)
	{
		for (char& c : retVal)
		{
			if (c == cPathSeparator)
				c = cWindowsPathSeparator;
		}
	}
	return retVal;
}

}
