CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
SRC = src/*.c
EXE = bejdecoder
GTESTHEADERS = #/opt/homebrew/Cellar/googletest/1.14.0/include/
GTESTLIB = #/opt/homebrew/Cellar/googletest/1.14.0/lib/

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

test: src/bej_decoder.c tests/test.c
	g++ -std=c++14 -I $(GTESTHEADERS) -L $(GTESTLIB) $^ -o tests/$@ -lgtest

clean:
	rm -f $(EXE)