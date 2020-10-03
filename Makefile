.PHONY: all build install 

all: night

night: src/Night.cpp
	g++ -o night -I. src/Night.cpp -o night

install:
	@mv night $(DESDIR)/usr/bin
		@echo Done
		
clean:
	rm night