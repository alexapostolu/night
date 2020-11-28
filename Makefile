.PHONY: build

build: src/main.cpp
	g++ -std=c++17 -o night src/main.cpp
