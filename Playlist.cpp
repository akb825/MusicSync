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

#include "Playlist.h"

#include "Helpers.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>

static const char* const cHeader = "#EXTM3U";
static const char* const cInfo = "#EXTINF";
const char * const Playlist::cExtension = ".m3u";

bool Playlist::load(const std::string& fileName)
{
	std::ifstream stream;
	stream.open(fileName.c_str(), std::ifstream::in);
	if (!stream)
	{
		std::fprintf(stderr, "Error: Couldn't open file '%s'.\n",
			fileName.c_str());
		return false;
	}

	std::string header;
	if (!Helpers::readLine(header, stream))
	{
		std::fprintf(stderr, "Error: Error reading file '%s'.\n", fileName.c_str());
		return false;
	}

	if (header != cHeader)
	{
		std::fprintf(stderr, "Error: File '%s' isn't a valid M3U file.\n",
			fileName.c_str());
		return false;
	}

	std::string info, songPath;
	do
	{
		if (!Helpers::readLine(info, stream))
		{
			std::fprintf(stderr, "Error: Error reading file '%s'.\n", fileName.c_str());
			return false;
		}
		if (info.empty())
			continue;
		if (info.find(cInfo) != 0 || stream.eof())
		{
			std::fprintf(stderr, "Error: File '%s' isn't a valid M3U file.\n", fileName.c_str());
			return false;
		}
		if (!Helpers::readLine(songPath, stream))
		{
			std::fprintf(stderr, "Error: Error reading file '%s'.\n", fileName.c_str());
			return false;
		}
		addSong(songPath, info);
	} while (!stream.eof());

	std::printf("Loaded playlist '%s'.\n", fileName.c_str());
	return true;
}

bool Playlist::save(const std::string& fileName) const
{
	std::ofstream stream;
	stream.open(fileName.c_str(), std::ofstream::out);
	if (!stream)
	{
		std::fprintf(stderr, "Error: Couldn't save file '%s'.\n",
			fileName.c_str());
		return false;
	}

	stream << cHeader << std::endl;

	for (const Entry& entry : m_entries)
	{
		stream << entry.info << std::endl;
		stream << entry.song << std::endl;
	}

	std::printf("Saved playlist '%s'.\n", fileName.c_str());
	return true;
}

void Playlist::addSong(const std::string& song, const std::string& info)
{
	m_entries.push_back(Entry(song, info));
}
