
.PHONY: all build install 

all: night


night: Night.cpp
		g++ Night.cpp -o night -I.

install:
		@mv night $(DESDIR)/usr/bin
			@echo Done 
clean:
	rm night

