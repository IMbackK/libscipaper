\page BUILD build

# Build instructions

These build instructions assume that UNIX is being used as the host os, however building on MacOS or Microsoft Windows is possible given only minor alterations to the procedure below.

## Requirments

* [PkgConfig](https://www.freedesktop.org/wiki/Software/pkg-config/)
	* Is required for the build system to find all the other dependancies.
* [GLib](https://docs.gtk.org/glib/)  version 2.41 or better
* [GModule](https://docs.gtk.org/gmodule/)
* [CURL](https://curl.se/)
* [git](https://git-scm.com/) is required to get the source.
* a C compiler like [GCC](https://gcc.gnu.org/) is required.
* optinonally:
	* [libxml2](https://en.wikipedia.org/wiki/Libxml2) is required for the scihub backend
	* [Doxygen](https://www.doxygen.nl/index.html) is required to generate the documentation

## Compileing and Installing

First we need to get the source, this can be perfomed by cloning https://github.com/IMbackK/libscipaper or https://git-ce.rwth-aachen.de/carl_philipp.klemm/libscipaper/ using git. Note that these two versions might not allways be totaly in sync.

```
$ git clone https://github.com/IMbackK/libscipaper
```

Then we must build and install the libary, to do this we must in the libscipaper do the following

```
$ mkdir build; cd build
$ cmake ..
$ make
$ sudo make install
```

You may want to change the default install prefix from /usr/local to something else, for this add `-DCMAKE_INSTALL_PREFIX=` with the path you desire to the cmake command.

## Linking

With scipaper installed the header can be included with #include `<scipaper/scipaper.h>` and link with `-lscipaper`
