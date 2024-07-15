#ifndef PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
#define PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <list>
#include <sstream>

#include "bridge.h"
#include "utils.h"

namespace psql_tcp {
class ProxyServer {
 public:
  ProxyServer() = delete;
  explicit ProxyServer(char *argv[]);
  ~ProxyServer();

  void Run();

 private:
  enum { kRecvRequest, kSendRequest, kRecvResponse, kSendResponse };

  void SetFDSet();
  void AcceptConnection();
  void ProcessConnections();

  std::string filename_ = "";

  int client_listener_{};

  fd_set read_fd_set_;
  fd_set write_fd_set_;
  int sockets_max_{};

  std::list<Bridge *> bridges_;

  int function_result_{};
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_PROXY_SERVER_H_
