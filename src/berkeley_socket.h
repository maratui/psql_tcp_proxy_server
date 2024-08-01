#ifndef PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
#define PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

namespace psql_tcp {
class BerkeleySocket {
 public:
  ~BerkeleySocket() = default;
  static int CreateServerSocket(const std::string &ip_address, unsigned port);
  static int CreateClientSocket(const std::string &ip_address, unsigned port);
  static int Accept(int listener_fd);

 private:
  static const int kMaxConn = 10;

  BerkeleySocket() = delete;
  BerkeleySocket(const BerkeleySocket &other) = delete;
  BerkeleySocket(BerkeleySocket &&other) = delete;
  void operator=(const BerkeleySocket &other) = delete;
  void operator=(const BerkeleySocket &&other) = delete;

  static int CreateSocket(int &socket_fd);
  static void SetReuseSockOpt(int socket_fd);
  static int SetSockaddrIn(int &socket_fd, struct sockaddr_in &sockaddr,
                           const std::string &ip_address, unsigned port);
  static int Bind(int &socket_fd, const struct sockaddr_in &sockaddr);
  static void SetNonblockFD(int fd);
  static int Listen(int &socket_fd);
  static int Connect(int &socket_fd, const struct sockaddr_in &sockaddr);

  static void CheckResult(int result, int &socket_fd,
                          const std::string &log_text);
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
