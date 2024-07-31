#include <iostream>

#include "proxy_server.h"

int main(int argc, char *argv[]) {
  if (argc == 3) {
    psql_tcp::ProxyServer proxy_server(argv);

    proxy_server.Run();
  } else {
    std::cout
        << "TCP прокси-сервер для СУБД Postgresql должен получить из "
           "командной строки 2 параметра:\n 1) "
           "Номер порта прокси-сервера\n 2) Путь для логирования всех SQL "
           "запросов, проходящих через прокси-сервер\n";
  }

  return 0;
}
