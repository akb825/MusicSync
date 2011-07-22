all:
	g++ -O3 main.cpp Playlist.cpp Helpers.cpp Options.cpp Logic.cpp -lboost_filesystem -lssl -o MusicSync

clean:
	rm MusicSync
