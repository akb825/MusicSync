#include "Options.h"
#include "Logic.h"

int main(int argc, const char* const* argv)
{
	Options options;
	if (!options.getFromCommandLine(argc, argv))
		return -1;
	return !Logic::syncMusic(options);
	return 0;
}
