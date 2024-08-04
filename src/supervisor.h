#ifndef PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_
#define PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_

#include <signal.h>

#include "proxy_server.h"

class Supervisor {
 public:
  Supervisor() = default;
  ~Supervisor() {
    std::cout << "\nУказатель прокси-сервера супервайзера = " << proxy_server_
              << "." << std::endl;
    std::cout << "Выполнен деструктор супервайзера." << std::endl;
  }

  void SetProxyServer(psql_tcp::ProxyServer *proxy_server) {
    proxy_server_ = proxy_server;
    std::cout << "Супервайзером получен указатель " << proxy_server_
              << " на экземпляр прокси-сервера." << std::endl;
  }
  void StopProxyServer() {
    if (proxy_server_) {
      proxy_server_->~ProxyServer();
    }
    proxy_server_ = nullptr;
  }

 private:
  Supervisor(const Supervisor &other) = delete;
  Supervisor(Supervisor &&other) = delete;
  void operator=(const Supervisor &other) = delete;
  void operator=(const Supervisor &&other) = delete;

  psql_tcp::ProxyServer *proxy_server_ = nullptr;
};

Supervisor supervisor;

void SignalHandler(int signal) {
  (void)signal;
  std::cout << "\nПолучен ctrl + c" << std::endl;
  supervisor.StopProxyServer();
  exit(EXIT_SUCCESS);
}

#endif  // PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_