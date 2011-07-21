all:
	g++ -O3 main.cpp Playlist.cpp Helpers.cpp Options.cpp Logic.cpp -o MusicSync

clean:
	rm MusicSync
