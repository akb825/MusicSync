all:
	g++ -O3 main.cpp Playlist.cpp Helpers.cpp Options.cpp Logic.cpp -lboost_filesystem -o MusicSync

clean:
	rm MusicSync
