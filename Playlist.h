#ifndef PLAYLIST_H
#define PLAYLIST_H

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
	typedef std::vector<Entry> EntryVector;

	bool load(const std::string& fileName);
	bool save(const std::string& fileName) const;

	void addSong(const std::string& song, const std::string& info);

	const EntryVector& getEntries() const		{return m_entries;}

	bool operator==(const Playlist& other) const
	{
		return m_entries == other.m_entries;
	}
	bool operator!=(const Playlist& other) const	{return !(*this == other);}
private:
	EntryVector m_entries;
};

#endif
