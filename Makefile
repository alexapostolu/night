.PHONY: build

build: src/main.cpp
	g++ -std=c++17 -o night src/*.cpp src/front-end/*.cpp src/back-end/*.cpp