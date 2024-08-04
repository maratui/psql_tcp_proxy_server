#include "supervisor.h"

int main(int argc, char *argv[]) {
  if (argc == 3) {
    psql_tcp::ProxyServer proxy_server(argv);

    supervisor.SetProxyServer(&proxy_server);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

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
