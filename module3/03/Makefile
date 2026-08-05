CC = gcc
TARGET = chat

all: $(TARGET)

$(TARGET): main.c source.c header.h
	$(CC) -Wall -g main.c source.c -o $(TARGET) -lrt

clean:
	rm -rf $(TARGET) $(TARGET).dSYM
