CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
SRC = src/*.c
EXE = bejdecoder

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
	./$(EXE) example/memory.dict example/memory.bin

clean:
	rm -f $(EXE)