# DLP-System
Simple client-server keylogger
Server Only linux
лиент подключится к серверу и начнет отправлять нажатия клавиш вместе с окном, находящимся в фокусе.  

Серверная программа запускается на вашем компьютере с Linux и получает нажатия клавиш, записывает их в файл в каталоге `log/` (он будет создан после первого запуска), а также распечатывает их.

  Клиент работает только на Windows
  Сервер работает только на Ubuntu


# Dependencies

Вам понадобится MinGW-w64 для компиляции исполняемого файла Windows в Linux.

И обычный gcc.

Ubuntu/Debian:
```shell
sudo apt install mingw-w64 gcc
```

# Использование

## Опции

Настройки win_client куда будут отправлять данные, нужно поменять на свои приразворачивании сервера

```c
#define HOST "127.0.0.1"    // CHANGE ME
#define PORT 8080           // CHANGE ME
```

## Компиляция

```shell
make
```
Запуск make скомпилирует и сервер, и клиент.
Но вы также можете запустить make server или make client.


## Run server

Run the server on whatever port you set.

```shell
./bin/server -p <PORT>
```
