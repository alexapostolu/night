.PHONY: all install

all: build_night

build_night: src/night.cpp
	g++ -o night src/night.cpp

install:
	@mv -i night /usr/bin || mv -i night $HOME/usr/bin
	@echo Done
