#!/bin/bash

if [[ ! -d libtcod-1.5.1 ]]; then
	wget -O - -o /dev/null https://bitbucket.org/libtcod/libtcod/downloads/libtcod-1.5.1-linux64.tar.gz | tar zx
fi
if [[ ! -d build ]]; then
	mkdir build
fi
if [[ ! -f terminal.png ]]; then
	ln -s libtcod-1.5.1/terminal.png terminal.png
fi

mkdir -p src/util &>/dev/null
if [[ ! -f src/util/cpptoml.h ]]; then
	wget -O src/util/cpptoml.h -o /dev/null https://raw.githubusercontent.com/skystrife/cpptoml/master/include/cpptoml.h
fi

if [[ ! -d entityx ]]; then
	git clone https://github.com/alecthomas/entityx
fi

cd build
cmake ..
echo "
Done bootstrapping, type \`make run\` to run"
