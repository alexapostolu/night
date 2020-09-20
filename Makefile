.PHONY: all build night dusk install 

all: build

build: night dusk

night:
	g++ night.cpp -o night

dusk:
	go build -ldflags "-s -w" dusk/dusk.go -o dusk

install:
	@mv night dusk /usr/bin
	@echo Done