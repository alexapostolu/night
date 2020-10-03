.PHONY: all install

all: build_night

build_night: src/night.cpp
    g++ -o night src/night.cpp

install:
    @mv night $(DESDIR)/usr/bin
        @echo Done
