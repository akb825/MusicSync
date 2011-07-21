#include "Playlist.h"
#include "Helpers.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cassert>

static const char* const cHeader = "#EXTM3U";
static const char* const cInfo = "#EXTINFO";
const char * const Playlist::cExtension = ".m3u";

bool Playlist::load(const std::string& fileName)
{
	std::ifstream stream;
	stream.open(fileName.c_str(), std::ifstream::in);
	if (!stream)
	{
		std::fprintf(stderr, "Couldn't open file '%s'.\n", fileName.c_str());
		return false;
	}

	std::string header;
	if (!Helpers::readLine(header, stream))
	{
		std::fprintf(stderr, "Error reading file '%s'.\n", fileName.c_str());
		return false;
	}

	if (header != cHeader)
	{
		std::fprintf(stderr, "File '%s' isn't a valid M3U file.\n",
			fileName.c_str());
		return false;
	}

	std::string info, songPath;
	do
	{
		if (!Helpers::readLine(info, stream))
		{
			std::fprintf(stderr, "Error reading file '%s'.\n", fileName.c_str());
			return false;
		}
		if (info.find(cInfo) != 0 || stream.eof())
		{
			std::fprintf(stderr, "File '%s' isn't a valid M3U file.\n",
				fileName.c_str());
			return false;
		}
		if (!Helpers::readLine(songPath, stream))
		{
			std::fprintf(stderr, "Error reading file '%s'.\n", fileName.c_str());
			return false;
		}
		addSong(songPath, info);
	} while (!stream.eof());

	std::printf("Read playlist '%d'.", fileName.c_str());
	return true;
}

bool Playlist::save(const std::string& fileName) const
{
	std::ofstream stream;
	stream.open(fileName.c_str(), std::ofstream::out);
	if (!stream)
	{
		std::fprintf(stderr, "Couldn't save file '%s'.\n", fileName.c_str());
		return false;
	}

	stream << cHeader << std::endl;

	for (EntryVector::const_iterator iter = m_entries.begin();
		iter != m_entries.end(); ++iter)
	{
		stream << std::endl;
		stream << iter->info << std::endl;
		stream << iter->song << std::endl;
	}

	std::printf("Saved playlist '%s'.\n", fileName.c_str());
	return true;
}

void Playlist::addSong(const std::string& song, const std::string& info)
{
	m_entries.push_back(Entry(song, info));
}

