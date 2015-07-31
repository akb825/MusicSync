all:
	g++ -O3 -std=c++11 main.cpp Playlist.cpp Helpers.cpp Options.cpp Logic.cpp -lboost_filesystem -lboost_system -lssl -lcrypto -o MusicSync

clean:
	rm MusicSync
