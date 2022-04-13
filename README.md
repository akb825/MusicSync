# MusicSync

MusicSync provides a simple command-line interface to synchronize a folder of M3U playlists with a folder, generally on an MP3 player or phone. To perform the sync, it does the following in order:

1. Removes any playlists on the device not in the input playlist folder. (when `--remove-old-playlists` is provided)
2. Removes any songs not referenced by the playlists. (when `--remove-old-songs` is provided)
3. Writes any playlists that have changed since the last sync or weren't previously present.
4. Writes any songs that have changed since the last sync or weren't previously present.

Run the tool without any arguments to get the full list of options to control the tool behavior.

# Building

The only requirements to build MusicSync are a modern C++ compiler and CMake 3.8 or later.

For example, run these commands to make a release build in a `build` directory:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
