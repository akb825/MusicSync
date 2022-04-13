cmake_minimum_required(VERSION 3.8)
project(MusicSync)

set(CMAKE_CXX_STANDARD 17)

add_executable(MusicSync
	Helpers.cpp
	Helpers.h
	Logic.cpp
	Logic.h
	main.cpp
	md5.cpp
	md5.h
	Options.cpp
	Options.h
	Playlist.cpp
	Playlist.h
)
