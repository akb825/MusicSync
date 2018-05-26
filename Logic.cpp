#include "Logic.h"
#include "Playlist.h"
#include "Helpers.h"
#include "Options.h"
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <cstdio>
#include <cassert>
#include <boost/filesystem.hpp>

typedef std::unordered_map<std::string, std::string> SongMap;

struct PlaylistInfo
{
	Playlist playlist;
	std::string fileName;
	std::time_t modifiedTime;
};

static bool validateLocations(const Options& options)
{
	if (!boost::filesystem::is_directory(options.playlistInput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open playlist input directory '%s'.\n",
			options.playlistInput.c_str());
		return false;
	}

	if (!boost::filesystem::is_directory(options.playlistOutput) &&
		!boost::filesystem::create_directories(options.playlistOutput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open playlist output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}

	if (!boost::filesystem::is_directory(options.songOutput) &&
		!boost::filesystem::create_directories(options.songOutput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open song output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}

	return true;
}

static bool isPlaylist(const boost::filesystem::directory_entry& entry)
{
	if (entry.status().type() != boost::filesystem::regular_file)
		return false;
	return entry.path().extension() == Playlist::cExtension;
}

static void readPlaylists(std::list<PlaylistInfo>& playlists,
	const Options& options)
{
	std::printf("Reading playlists...\n");

	for (boost::filesystem::directory_iterator iter(options.playlistInput);
		iter != boost::filesystem::directory_iterator(); ++iter)
	{
		if (!isPlaylist(*iter))
			continue;

		playlists.push_back(PlaylistInfo());
		if (!playlists.back().playlist.load(iter->path().string()))
		{
			playlists.pop_back();
			continue;
		}

		playlists.back().fileName = iter->path().filename().string();
		playlists.back().modifiedTime =
			boost::filesystem::last_write_time(iter->path()); 
	}

	std::printf("Done.\n");
}

static void getSongPaths(SongMap& songs,
	const std::list<PlaylistInfo>& playlists, const Options& options)
{
	std::string finalPath;
	for (std::list<PlaylistInfo>::const_iterator pIter = playlists.begin();
		pIter != playlists.end(); ++pIter)
	{
		const Playlist::EntryVector& entries = pIter->playlist.getEntries();
		for (Playlist::EntryVector::const_iterator eIter = entries.begin();
			eIter != entries.end(); ++eIter)
		{
			if (Helpers::getRelativePath(finalPath, eIter->song,
				options.pathTrim))
			{
				finalPath = Helpers::repairFilename(finalPath);
				songs.insert(std::make_pair(eIter->song, finalPath));
			}
			else
			{
				std::fprintf(stderr, "Error: Error processing song '%s'.\n",
					eIter->song.c_str());
			}
		}
	}
}

static void writePlaylists(std::list<PlaylistInfo>& playlists,
	const SongMap& songs, const Options& options)
{
	std::printf("Writing modified playlists...\n");

	std::string songPath;
	boost::filesystem::path playlistPath;
	for (std::list<PlaylistInfo>::const_iterator pIter = playlists.begin();
		pIter != playlists.end(); ++pIter)
	{
		playlistPath = options.playlistOutput;
		playlistPath /= pIter->fileName;

		//See if it's already up to date.
		boost::system::error_code result;
		std::time_t timeStamp = boost::filesystem::last_write_time(playlistPath,
			result);
		if (result == boost::system::errc::success &&
			timeStamp >= pIter->modifiedTime)
		{
			continue;
		}

		Playlist newPlaylist;
		const Playlist::EntryVector& entries = pIter->playlist.getEntries();
		for (Playlist::EntryVector::const_iterator eIter = entries.begin();
			eIter != entries.end(); ++eIter)
		{
			SongMap::const_iterator foundIter = songs.find(eIter->song);
			if (foundIter == songs.end())
				continue;

			songPath = Helpers::getPlaylistSongPath(foundIter->second,
				options.pathPrefix, options.windowsSeparators);
			newPlaylist.addSong(songPath, eIter->info);
		}
		newPlaylist.save(playlistPath.string());
	}

	std::printf("Done.\n");
}

static void removeDeletedPlaylists(std::list<PlaylistInfo>& playlists,
	const Options& options)
{
	std::printf("Removing deleted playlists...\n");

	std::vector<boost::filesystem::path> removeFiles;

	for (boost::filesystem::directory_iterator dIter(options.playlistOutput);
		dIter != boost::filesystem::directory_iterator(); ++dIter)
	{
		if (!isPlaylist(*dIter))
			continue;
		bool found = false;
		for (std::list<PlaylistInfo>::const_iterator pIter = playlists.begin();
			pIter != playlists.end(); ++pIter)
		{
			if (pIter->fileName == dIter->path().filename())
			{
				found = true;
				break;
			}
		}
		if (found)
			continue;

		std::printf("Removing file '%s'.\n", dIter->path().string().c_str());
		removeFiles.emplace_back(dIter->path());
	}

	for (const boost::filesystem::path& path : removeFiles)
		boost::filesystem::remove(path);

	std::printf("Done.\n");
}

static void syncSongs(const SongMap& songs, const Options& options)
{
	std::printf("Synchronizing songs...\n");

	boost::filesystem::path srcBase = options.playlistInput;
	boost::filesystem::path dstBase = options.songOutput;
	for (SongMap::const_iterator iter = songs.begin(); iter != songs.end();
		++iter)
	{
		boost::filesystem::path srcPath = iter->first;
		//Assume it's reative to the playlist input if it's not absolute.
		if (!srcPath.is_absolute())
			srcPath = srcBase/srcPath;

		boost::filesystem::path dstPath = dstBase/iter->second;

		boost::system::error_code result;
		std::time_t srcTimeStamp = boost::filesystem::last_write_time(srcPath,
			result);
		if (result != boost::system::errc::success)
		{
			std::fprintf(stderr, "Error: Couldn't read file '%s'.\n",
				srcPath.string().c_str());
			continue;
		}
		std::time_t dstTimeStamp = boost::filesystem::last_write_time(dstPath,
			result);
		if (result == boost::system::errc::success)
		{
			//See if it's already up to date.
			if (srcTimeStamp <= dstTimeStamp)
				continue;
		}
		
		boost::filesystem::create_directories(dstPath.parent_path());
		boost::filesystem::copy_file(srcPath, dstPath,
			boost::filesystem::copy_option::overwrite_if_exists, result);
		if (result == boost::system::errc::success)
			std::printf("Copied song to '%s'.\n", iter->second.c_str());
		else
		{
			std::fprintf(stderr, "Error: Couldn't copy song '%s' to '%s'.\n",
				iter->first.c_str(), iter->second.c_str());
		}
	}

	std::printf("Done.\n");
}

static void removeDeletedSongs(const SongMap& songs, const Options& options)
{
	std::printf("Removing deleted songs...\n");

	typedef std::unordered_set<std::string> SongSet;
	SongSet relativePaths;
	for (SongMap::const_iterator iter = songs.begin(); iter != songs.end();
		++iter)
	{
		relativePaths.insert(iter->second);
	}

	std::vector<boost::filesystem::path> removeFiles;

	for (boost::filesystem::recursive_directory_iterator
			dIter(options.songOutput);
		dIter != boost::filesystem::recursive_directory_iterator(); ++dIter)
	{
		if (dIter->status().type() != boost::filesystem::regular_file)
			continue;
		std::string relativePath;
		if (!Helpers::getRelativePath(relativePath, dIter->path().string(),
			options.songOutput))
		{
			assert(false);
			continue;
		}
		if (relativePaths.find(relativePath) == relativePaths.end())
		{
			std::printf("Removing song '%s'.\n", relativePath.c_str());
			removeFiles.emplace_back(dIter->path());
		}
	}

	for (const boost::filesystem::path& path : removeFiles)
		boost::filesystem::remove(path);

	std::printf("Done.\n");
}

namespace Logic
{

bool syncMusic(const Options& options)
{
	if (!validateLocations(options))
		return false;

	std::list<PlaylistInfo> playlists;
	SongMap songs;

	readPlaylists(playlists, options);
	std::printf("\n");
	getSongPaths(songs, playlists, options);
	if (options.removePlaylists)
	{
		removeDeletedPlaylists(playlists, options);
		std::printf("\n");
	}
	if (options.removeSongs)
	{
		removeDeletedSongs(songs, options);
		std::printf("\n");
	}
	writePlaylists(playlists, songs, options);
	std::printf("\n");
	syncSongs(songs, options);

	return true;
}

}
