CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
SRC = src/*.c
EXE = main

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXE)