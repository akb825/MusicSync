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

#include "Logic.h"

#include "Helpers.h"
#include "Options.h"
#include "Playlist.h"

#include <cstdio>
#include <cassert>
#include <list>
#include <filesystem>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

namespace
{

using SongMap = std::unordered_map<std::string, std::string>;

struct PlaylistInfo
{
	Playlist playlist;
	std::string fileName;
	std::time_t modifiedTime;
};

bool validateLocations(const Options& options)
{
	if (!std::filesystem::is_directory(options.playlistInput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open playlist input directory '%s'.\n",
			options.playlistInput.c_str());
		return false;
	}

	if (!std::filesystem::is_directory(options.playlistOutput) &&
		!std::filesystem::create_directories(options.playlistOutput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open playlist output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}

	if (!std::filesystem::is_directory(options.songOutput) &&
		!std::filesystem::create_directories(options.songOutput))
	{
		std::fprintf(stderr,
			"Error: Couldn't open song output directory '%s'.\n",
			options.playlistOutput.c_str());
		return false;
	}

	return true;
}

bool isPlaylist(const std::filesystem::directory_entry& entry)
{
	if (entry.status().type() != std::filesystem::file_type::regular)
		return false;
	return entry.path().extension() == Playlist::cExtension;
}

void readPlaylists(std::list<PlaylistInfo>& playlists, const Options& options)
{
	std::printf("Reading playlists...\n");

	for (std::filesystem::directory_iterator iter(options.playlistInput);
		iter != std::filesystem::directory_iterator(); ++iter)
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
			std::filesystem::last_write_time(iter->path()).time_since_epoch().count();
	}

	std::printf("Done.\n");
}

void getSongPaths(SongMap& songs, const std::list<PlaylistInfo>& playlists, const Options& options)
{
	std::string finalPath;
	for (const PlaylistInfo& playlistInfo : playlists)
	{
		for (const Playlist::Entry& entry : playlistInfo.playlist.getEntries())
		{
			if (Helpers::getRelativePath(finalPath, entry.song, options.pathTrim))
			{
				finalPath = Helpers::repairFilename(finalPath, options.noUnicode);
				songs.emplace(entry.song, finalPath);
			}
			else
				std::fprintf(stderr, "Error: Error processing song '%s'.\n", entry.song.c_str());
		}
	}
}

void writePlaylists(std::list<PlaylistInfo>& playlists, const SongMap& songs,
	const Options& options)
{
	std::printf("Writing modified playlists...\n");

	std::string songPath;
	std::filesystem::path playlistPath;
	for (const PlaylistInfo& playlistInfo : playlists)
	{
		playlistPath = options.playlistOutput;
		playlistPath /= playlistInfo.fileName;

		//See if it's already up to date.
		std::error_code error;
		std::time_t timeStamp =
			std::filesystem::last_write_time(playlistPath, error).time_since_epoch().count();
		if (!error && timeStamp >= playlistInfo.modifiedTime)
			continue;

		Playlist newPlaylist;
		for (const Playlist::Entry& entry : playlistInfo.playlist.getEntries())
		{
			SongMap::const_iterator foundIter = songs.find(entry.song);
			if (foundIter == songs.end())
				continue;

			songPath = Helpers::getPlaylistSongPath(foundIter->second, options.pathPrefix,
				options.windowsSeparators);
			newPlaylist.addSong(songPath, entry.info);
		}
		newPlaylist.save(playlistPath.string());
	}

	std::printf("Done.\n");
}

static void removeDeletedPlaylists(std::list<PlaylistInfo>& playlists,
	const Options& options)
{
	std::printf("Removing deleted playlists...\n");

	std::vector<std::filesystem::path> removeFiles;

	for (std::filesystem::directory_iterator dIter(options.playlistOutput);
		dIter != std::filesystem::directory_iterator(); ++dIter)
	{
		if (!isPlaylist(*dIter))
			continue;
		bool found = false;
		for (const PlaylistInfo& playlistInfo : playlists)
		{
			if (playlistInfo.fileName == dIter->path().filename())
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

	for (const std::filesystem::path& path : removeFiles)
		std::filesystem::remove(path);

	std::printf("Done.\n");
}

static void syncSongs(const SongMap& songs, const Options& options)
{
	std::printf("Synchronizing songs...\n");

	std::filesystem::path srcBase = options.playlistInput;
	std::filesystem::path dstBase = options.songOutput;
	for (const SongMap::value_type& songInfo : songs)
	{
		std::filesystem::path srcPath = songInfo.first;
		//Assume it's reative to the playlist input if it's not absolute.
		if (!srcPath.is_absolute())
			srcPath = srcBase/srcPath;

		std::filesystem::path dstPath = dstBase/songInfo.second;

		std::error_code error;
		std::time_t srcTimeStamp =
			std::filesystem::last_write_time(srcPath, error).time_since_epoch().count();
		if (error)
		{
			std::fprintf(stderr, "Error: Couldn't read file '%s'.\n",
				srcPath.string().c_str());
			continue;
		}
		std::time_t dstTimeStamp =
			std::filesystem::last_write_time(dstPath, error).time_since_epoch().count();
		if (!error)
		{
			//See if it's already up to date.
			if (srcTimeStamp <= dstTimeStamp)
				continue;
		}

		std::filesystem::create_directories(dstPath.parent_path());
		std::filesystem::copy_file(srcPath, dstPath,
			std::filesystem::copy_options::overwrite_existing, error);
		if (error)
		{
			std::fprintf(stderr, "Error: Couldn't copy song '%s' to '%s'.\n",
				songInfo.first.c_str(), songInfo.second.c_str());
		}
		else
			std::printf("Copied song to '%s'.\n", songInfo.second.c_str());
	}

	std::printf("Done.\n");
}

static void removeDeletedSongs(const SongMap& songs, const Options& options)
{
	std::printf("Removing deleted songs...\n");

	std::unordered_set<std::string> relativePaths;
	for (const SongMap::value_type& songInfo : songs)
		relativePaths.insert(songInfo.second);

	std::vector<std::filesystem::path> removeFiles;

	for (std::filesystem::recursive_directory_iterator dIter(options.songOutput);
		dIter != std::filesystem::recursive_directory_iterator(); ++dIter)
	{
		if (dIter->status().type() != std::filesystem::file_type::regular)
			continue;
		std::string relativePath;
		if (!Helpers::getRelativePath(relativePath, dIter->path().string(), options.songOutput))
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

	for (const std::filesystem::path& path : removeFiles)
		std::filesystem::remove(path);

	std::printf("Done.\n");
}

} // namespace

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
