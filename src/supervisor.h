#ifndef PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_
#define PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_

#include <signal.h>

#include "proxy_server.h"

class Supervisor {
 public:
  Supervisor() = default;
  ~Supervisor() = default;

  void SetProxyServer(psql_tcp::ProxyServer *proxy_server) {
    proxy_server_ = proxy_server;
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
  if (signal == SIGINT) {
    std::cout << "\nПолучен сигнал завершения Ctrl+с" << std::endl;
  } else if (signal == SIGTERM) {
    std::cout << "\nПолучен сигнал завершения от команды kill" << std::endl;
  }
  supervisor.StopProxyServer();
  exit(EXIT_SUCCESS);
}

#endif  // PSQL_TCP_PROXY_SERVER_SUPERVISOR_H_