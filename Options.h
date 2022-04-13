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

#include <string>

#pragma once

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
