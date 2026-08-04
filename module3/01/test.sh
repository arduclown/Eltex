#!/bin/bash

make || exit 1

echo "Тестовые данные" > a.txt
head -c 5000 /dev/urandom > b.bin

echo "1. Неименованный канал, два файла"
./copier a.txt b.bin
cmp a.txt a.txt.copy && cmp b.bin b.bin.copy && echo "копии совпадают"

echo
echo "2. Именованный канал (-p)"
./copier -p mypipe a.txt b.bin
cmp a.txt a.txt.copy && cmp b.bin b.bin.copy && echo "копии совпадают"

echo
echo "3. Несуществующий файл (должна быть ошибка в stderr)"
./copier a.txt nofile.txt

echo
echo "4. Без имен файлов (ошибка)"
./copier

echo
echo "5. Неизвестный ключ (ошибка)"
./copier -x a.txt

echo
echo "6. Пустой файл"
: > empty.txt
./copier empty.txt
cmp empty.txt empty.txt.copy && echo "пустой файл скопирован"

echo
rm -f a.txt b.bin empty.txt *.copy mypipe
