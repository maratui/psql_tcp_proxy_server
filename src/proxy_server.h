#ifndef PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
#define PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_

#include <algorithm>
#include <list>
#include <sstream>

#include "berkeley_socket.h"
#include "bridge.h"

namespace psql_tcp {
class ProxyServer {
 public:
  ProxyServer() = delete;
  explicit ProxyServer(char *argv[]);
  ~ProxyServer();

  void Run();

 private:
  enum { kRecvRequest, kSendRequest, kRecvResponse, kSendResponse };

  const std::string kLocalHost = "127.0.0.1";

  void SetFDSet();
  void AcceptConnection();
  void ProcessConnections();
  void CheckResult(int result, const std::string &log_text);

  std::string filename_ = "";

  int client_listener_{};

  fd_set read_fd_set_;
  fd_set write_fd_set_;
  int sockets_max_{};

  std::list<Bridge *> bridges_;

  int result_{};
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
