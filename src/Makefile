CC = g++
CFLAGS = -Wall -Wextra -Werror -Wuninitialized
FILES = *.h *.cc
TARGET = psql_tcp_proxy_server
PORT = 8080
LOG = log.txt

all: build

run:
	./$(TARGET) $(PORT) $(LOG)

man_mode: CFLAGS += -DMAN_MODE
man_mode: build

build: clean
	@ mkdir -p build
	@ cd build && \
	  $(CC) $(CFLAGS) ../*.cc -o ../$(TARGET) && \
	  cd ../
	@ rm -rf build

clean:
	@ rm -f $(TARGET)

googlestyle:
	@ clang-format -style=Google -i $(FILES)

cppcheck:
	@ cppcheck --enable=all --suppress=missingIncludeSystem --language=c++ \
	 -UO_NONBLOCK -USO_REUSEPORT $(FILES)

valgrind: clean
	@ mkdir -p build
	@ cd build && \
	 $(CC) $(CFLAGS) -DLEAK_CHECK ../*.cc -o ./$(TARGET) && \
	 valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s \
	 ./$(TARGET) $(PORT) $(LOG) && \
	 cd .. && rm -rf build
