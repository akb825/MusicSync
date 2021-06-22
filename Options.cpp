#include "Options.h"
#include <cstring>
#include <cstdio>

static const char* const cRemovePlaylists = "--remove-old-playlists";
static const char* const cRemoveSongs = "--remove-old-songs";
static const char* const cWindowsSeparators = "--windows-separators";
static const char* const cNoUnicode = "--no-unicode";
static const char* const cPathTrim = "--trim-prefix";
static const char* const cPathPrefix = "--path-prefix";
static const char* const cPlaylistInput = "--playlist-input-dir";
static const char* const cPlaylistOutput = "--playlist-output-dir";
static const char* const cSongOutput = "--song-output-dir";

const char* const Options::cProgramName = "MusicSync";

static bool getNextString(unsigned int& index, std::string& string,
	unsigned int argc, const char* const* argv, const Options& options)
{
	++index;
	if (index >= argc)
	{
		options.printHelp();
		return false;
	}
	string = argv[index];
	++index;
	return true;
}

Options::Options()
	: removePlaylists(false), removeSongs(false), windowsSeparators(false), noUnicode(false)
{
}

bool Options::getFromCommandLine(unsigned int argc, const char* const* argv)
{
	unsigned int index = 1;
	while (index < argc)
	{
		if (std::strcmp(argv[index], cRemovePlaylists) == 0)
		{
			++index;
			removePlaylists = true;
		}
		else if (std::strcmp(argv[index], cRemoveSongs) == 0)
		{
			++index;
			removeSongs = true;
		}
		else if (std::strcmp(argv[index], cWindowsSeparators) == 0)
		{
			++index;
			windowsSeparators = true;
		}
		else if (std::strcmp(argv[index], cPathTrim) == 0)
		{
			if (!getNextString(index, pathTrim, argc, argv, *this))
				return false;
		}
		else if (std::strcmp(argv[index], cPathPrefix) == 0)
		{
			if (!getNextString(index, pathPrefix, argc, argv, *this))
				return false;
		}
		else if (std::strcmp(argv[index], cPlaylistInput) == 0)
		{
			if (!getNextString(index, playlistInput, argc, argv, *this))
				return false;
		}
		else if (std::strcmp(argv[index], cPlaylistOutput) == 0)
		{
			if (!getNextString(index, playlistOutput, argc, argv, *this))
				return false;
		}
		else if (std::strcmp(argv[index], cSongOutput) == 0)
		{
			if (!getNextString(index, songOutput, argc, argv, *this))
				return false;
		}
		else
		{
			printHelp();
			return false;
		}
	}
	if (playlistInput.empty() || playlistOutput.empty() || songOutput.empty())
	{
		printHelp();
		return false;
	}
	return true;
}

void Options::printHelp()
{
	std::fprintf(stderr,
		"Usage: %s [%s] [%s]\n"
		"         [%s] [%s prefix] [%s prefix]\n"
		"         %s path %s path\n"
		"         %s path\n"
		"\nOptions:\n"
		"   %s: Remove playlists that appear in the playlist output\n"
			"     directory but not the input directory.\n"
		"   %s: Remove songs that appear in the song output directory\n"
			"     but not in any playlist.\n"
		"   %s: Replace '/' with '\\' in playlist paths.\n"
		"   %s: Remove Unicode characters in filenames.\n"
		"   %s: A prefix to trim from every song path in a playlist file.\n"
		"   %s: A prefix to add to every song path in a playlist file.\n"
		"   %s: The input directory to read M3U playlists from.\n"
		"   %s: The output directory to write M3U playlists to.\n"
		"   %s: The output directory to write song fiels to.\n",
		cProgramName, cRemovePlaylists, cRemoveSongs, cWindowsSeparators,
		cNoUnicode, cPathTrim, cPathPrefix, cPlaylistInput, cPlaylistOutput,
		cSongOutput, cRemovePlaylists, cRemoveSongs, cWindowsSeparators,
		cPathTrim, cPathPrefix, cPlaylistInput, cPlaylistOutput, cSongOutput);
}
