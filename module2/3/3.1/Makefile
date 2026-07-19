CC = gcc
TARGET = 3_1

all: $(TARGET)

$(TARGET): main.c userchmod.c header.h
	$(CC) -Wall -g main.c userchmod.c -o $(TARGET)

clean:
	rm -f $(TARGET) 
	