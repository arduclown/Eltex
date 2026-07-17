CC = gcc
TARGET = 2_3

all: $(TARGET)

$(TARGET): main.c calculator.c header.h
	$(CC) -Wall -g main.c calculator.c -o $(TARGET)

clean:
	rm -f $(TARGET) 
	