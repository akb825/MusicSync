#include "Logic.h"
#include "Playlist.h"
#include "Helpers.h"
#include "Options.h"
#include <tr1/unordered_map>
#include <list>
#include <cstdio>
#include <boost/filesystem.hpp>

struct PlaylistInfo
{
	Playlist playlist;
	std::string fileName;
	time_t modifiedTime;
};

static bool validateLocations(const Option& options)
{
	if (!boost::filesystem::is_directory(options.playlistInput))
	{
		std::fprintf(stderr, "Couldn't open playlist input directory '%s'.\n",
			options.playlistInput.c_str());
		return false;
	}

	if (!boost::filesystem::is_directory(options.playlistOutput) ||
		!boost::filesystem::create_directories(options.playlistOuptut))
	{
		std::fprintf(stderr, "Couldn't open playlist output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}
	closedir(directory);

	if (!boost::filesystem::is_directory(options.songOutput) ||
		!boost::filesystem::create_directories(options.songOuptut))
	{
		std::fprintf(stderr, "Couldn't open song output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}
	closedir(directory);

	return true;
}

static bool readPlaylists(std::list<PlaylistInfo>& playlists,
	const Options& options)
{
	std::printf("Reading playlists...\n");

	for (boost::filesystem::directory_iterator iter(options.playlistInput);
		iter != boost::filesystem::directory_iterator(); ++iter)
	{
		const boost::filesystem::directory_entry& entry = *iter;
		if (entry.status().type() != boost::filesystem::regular_file &&
			entry.status().type() != boost::filesystem::symlink_file)
			continue;

		if (entry.path().extension() != Playlist::cExtension)
			continue;

		playlists.push_back(PlaylistInfo());
		std::string path = Helpers::appendPath(options.playlistInput,
			entry.path().native());
		if (!playlists.back().playlist.load(path))
		{
			playlists.pop_back();
			continue;
		}

		playlists.back().fileName = path.filename().native();
		playlists.back.modifiedTime = boost::filesystem::last_write_time(path); 
	} while (true);

	std::printf("Done.\n");
}

void getSongPaths(std::tr1::unordered_map<std::string, std::string>& fileNames,
	const std::list<PlaylistInfo>& playlists, const Options& options)
{
	std::string finalPath;
	for (std::list<PalylistInfo>::const_iterator pIter = playlists.begin();
		pIter != playlists.end(); ++pIter)
	{
		const Playlist::EntryVector& entries = pIter->playlist.getEntries();
		for (Playlist::EntryVector::const_iterator eIter = entries.begin();
			eIter != entries.end(); ++eIter)
		{
			if (Helpers::processPath(finalPath, eIter->song, options.pathTrim,
				options.pathPrefix))
			{
				fileNames.insert(std::make_pair(eIter->song, finalPath));
			}
			else
			{
				std::printf(stderr, "Error processing song '%s'.\n",
					eIter->song.c_str());
			}
		}
	}
}

void writePlaylists(std::list<PlaylistInfo>& playlists, const Option& options)
{
	std::printf("Writing modified playlists...\n");

	std::string songPath;
	boost::filesystem::path playlistPath;
	for (std::list<PalylistInfo>::const_iterator pIter = playlists.begin();
		pIter != playlists.end(); ++pIter)
	{
		playlistPath = options.playlistOutput;
		playlistPath /= pIter->fileName;
		if (boost::filesystem::last_write_type(playlistPath) >=
			pIter->modifiedTime)
		{
			continue;
		}

		Playlist newPlaylist;
		const Playlist::EntryVector& entries = pIter->playlist.getEntries();
		for (Playlist::EntryVector::const_iterator eIter = entries.begin();
			eIter != entries.end(); ++eIter)
		{
			if (!Helpers::processPath(songPath, eIter->song, options.pathTrim,
				options.pathPrefix))
			{
				continue;
			}

			if (options.windowsSeparators)
				songPath = Helpers::convertSeparators(songPath);
			newPlaylist.push_back(songPath, eIter.info);
		}
		newPlaylist.save(playlistPath.native());
	}

	std::printf("Done.\n");
}

namespace Logic
{

bool syncMusic(const Options& options)
{
	if (!validateLocations(options))
		return false;
}

}
