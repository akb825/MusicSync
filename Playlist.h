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

#pragma once

#include <string>
#include <vector>

class Playlist
{
public:

	static const char* const cExtension;

	struct Entry
	{
		Entry() {}
		Entry(const std::string& initSong, const std::string& initInfo)
			: song(initSong), info(initInfo) {}
		std::string song;
		std::string info;

		bool operator==(const Entry& other) const
		{
			return song == other.song && info == other.info;
		}
		bool operator!=(const Entry& other) const	{return !(*this == other);}
	};

	bool load(const std::string& fileName);
	bool save(const std::string& fileName) const;

	void addSong(const std::string& song, const std::string& info);

	const std::vector<Entry>& getEntries() const		{return m_entries;}

	bool operator==(const Playlist& other) const
	{
		return m_entries == other.m_entries;
	}
	bool operator!=(const Playlist& other) const	{return !(*this == other);}

private:
	std::vector<Entry> m_entries;
};
