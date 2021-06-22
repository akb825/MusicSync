#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

struct Options
{
	static const char* const cProgramName;

	Options();
	bool getFromCommandLine(unsigned int argc, const char* const* argv);
	static void printHelp();

	bool removePlaylists;
	bool removeSongs;
	bool windowsSeparators;
	bool noUnicode;
	std::string pathTrim;
	std::string pathPrefix;
	std::string playlistInput;
	std::string playlistOutput;
	std::string songOutput;
};

#endif
