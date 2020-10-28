.PHONY: all install

all: build_night

build_night: src/night.cpp
	g++ -o night src/night.cpp

install:
	@mv night /usr/bin || mv night ~/usr/bin
		@echo Done
