#ifndef PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
#define PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>

namespace psql_tcp {
class BerkeleySocket {
 public:
  ~BerkeleySocket() = default;

  static int CreateServerSocket(const std::string &ip_address, unsigned port);
  static int CreateClientSocket(const std::string &ip_address, unsigned port);
  static int Accept(int listener_fd);

 private:
  static constexpr inline int kMaxConn = 10;
  static constexpr inline int kReuse = 1;

  BerkeleySocket() = delete;
  BerkeleySocket(const BerkeleySocket &other) = delete;
  BerkeleySocket(BerkeleySocket &&other) = delete;
  void operator=(const BerkeleySocket &other) = delete;
  void operator=(const BerkeleySocket &&other) = delete;

  static int CreateSocket(int &socket_fd);
  static int SetReuseSockOpt(int &socket_fd);
  static int SetNonblockFD(int &fd);
  static int SetSockaddrIn(int &socket_fd, struct sockaddr_in &sockaddr,
                           const std::string &ip_address, unsigned port);
  static int Bind(int &socket_fd, const struct sockaddr_in &sockaddr);
  static int Listen(int &socket_fd);
  static int Connect(int &socket_fd, const struct sockaddr_in &sockaddr);

  static void CheckResult(int result, int &socket_fd,
                          const std::string &log_text);
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
