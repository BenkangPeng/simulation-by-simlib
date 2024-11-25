CC=g++
CFLAGS=-O2 -Wall -I ../../simlib
DEBUGFLAGS=-O0 -g -Wall -I ../../simlib
SOURCE=main.cpp ../../simlib/simlib.c function.cpp
TARGET=main.out
DEBUG_TARGET=main_gdb.out
OUTPUT=esm.out

$(TARGET):$(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

$(DEBUG_TARGET):$(SOURCE)
	$(CC) $(DEBUGFLAGS) $(SOURCE) -o $(DEBUG_TARGET)
	
all:$(TARGET)


run:$(TARGET)
	./$(TARGET)

debug:$(DEBUG_TARGET)
	gdb $(DEBUG_TARGET)

clean:
	rm -f $(TARGET) $(DEBUG_TARGET) $(OUTPUT)

.PHONY:all run debug clean