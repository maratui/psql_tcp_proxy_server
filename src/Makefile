PROJECTNAME=$(shell basename "$(PWD)")

CC = g++
CFLAGS = -Wall -Wextra -Werror -Wuninitialized
FILES = *.h *.cc
TARGET = psql_tcp_proxy_server
PORT = 8080
LOG = log.txt

## make:          сборка прокси-сервера
all: build

## make run:      запуск прокси-сервера
run:
	./$(TARGET) $(PORT) $(LOG)

## make man_mode: сборка прокси-сервера в пошаговом режиме
man_mode: CFLAGS += -DMAN_MODE
man_mode: build

build: clean
	@ mkdir -p build
	@ cd build && \
	  $(CC) $(CFLAGS) ../*.cc -o ../$(TARGET) && \
	  cd ../
	@ rm -rf build

## make clean:    удалить прокси-сервер
clean:
	@ rm -f $(TARGET)

googlestyle:
	@ clang-format -style=Google -i $(FILES)

cppcheck:
	@ cppcheck --enable=all --suppress=missingIncludeSystem --language=c++ \
	 -UO_NONBLOCK -USO_REUSEPORT $(FILES)

## make valgrind: проверить работу прокси-сервера на утечки памяти
valgrind:
	@ mkdir -p build
	@ cd build && \
	 $(CC) $(CFLAGS) -DLEAK_CHECK ../*.cc -o ./$(TARGET) && \
	 valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s \
	 ./$(TARGET) $(PORT) $(LOG) && \
	 cd .. && rm -rf build

.PHONY: help
help: Makefile
	@echo "\n Выберите команду для запуска в "$(PROJECTNAME)":\n"
	@sed -n 's/^##/ /p' $<
	@echo
