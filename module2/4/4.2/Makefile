CC = gcc
TARGET = 4_2

all: $(TARGET)

$(TARGET): main.c queuePriority.c header.h
	$(CC) -Wall -g main.c queuePriority.c -o $(TARGET)

clean:
	rm -f $(TARGET) 
	