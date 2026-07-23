CC = gcc
TARGET = 4_3

all: $(TARGET)

$(TARGET): main.c contactB.c header.h
	$(CC) -Wall -g main.c contactB.c -o $(TARGET)

clean:
	rm -f $(TARGET) 
	