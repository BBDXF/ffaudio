# README.md
A simple audio parse and play sdk for using ffmpeg. ffmpeg is too big.

- get metadata, basic audio info.
- change volume, speed
- local file and url source support. http header support.
- play in thread. control in current.
- play status callback. play control.
- C export using shared library.
- meson build system.

# Build
meson + ninja + ffmpeg shared library.
meson + ninja + ffmpeg static library. 

## shared ffmpeg
ffmpeg is big. but it's ok in linux. windows is ok if already have ffmpeg shared library.

## static library
buildin ffmpeg functions in ffaudio dll.

# APIs
TODO


# Author
BBDXF
