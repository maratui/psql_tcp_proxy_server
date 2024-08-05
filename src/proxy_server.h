#ifndef PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
#define PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_

#include <list>
#include <sstream>

#include "bridge.h"

namespace psql_tcp {
class ProxyServer {
 public:
  explicit ProxyServer(char *argv[]);
  ~ProxyServer();

  void Run();

 private:
  static const inline std::string kLocalHost = "127.0.0.1";

  enum { kRecvRequest, kSendRequest, kRecvResponse, kSendResponse };

  ProxyServer() = delete;
  ProxyServer(const ProxyServer &other) = delete;
  ProxyServer(ProxyServer &&other) = delete;
  void operator=(const ProxyServer &other) = delete;
  void operator=(const ProxyServer &&other) = delete;

  void SetFDSet();
  void AcceptConnection();
  void ProcessConnections();
  void CheckResult(int result, const std::string &log_text);

  std::list<Bridge *> bridges_;

  int client_listener_{};
  int new_socket_{};
  int sockets_ready_{};

  fd_set read_fd_set_{};
  fd_set write_fd_set_{};
  int max_socket_{};

  int result_{};
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
