CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
SRC = src/*.c
EXE = bejdecoder

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $< -o $@
	./$(EXE) example/dictionary.bin example/example.bin

clean:
	rm -f $(EXE)