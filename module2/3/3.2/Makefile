CC = gcc
TARGET = 3_2

all: $(TARGET)

$(TARGET): main.c network.c header.h
	$(CC) -Wall -g main.c network.c -o $(TARGET)

clean:
	rm -f $(TARGET) 
	