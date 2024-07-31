#ifndef PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
#define PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

namespace psql_tcp {
class BerkeleySocket {
 public:
  ~BerkeleySocket() = default;
  static int CreateServerSocket(unsigned server_ip_address,
                                unsigned server_port);

 private:
  BerkeleySocket() = delete;
  BerkeleySocket(const BerkeleySocket &other) = delete;
  BerkeleySocket(BerkeleySocket &&other) = delete;
  void operator=(const BerkeleySocket &other) = delete;
  void operator=(const BerkeleySocket &&other) = delete;

  static void SetNonblockFD(int fd);
  static struct sockaddr_in GetSockaddrIn(unsigned ip_address, unsigned port);

  static void CheckResult(int result, const std::string &log_text);
  static void CheckResult(int result, int socket_fd,
                          const std::string &log_text);
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BERKELEY_SOCKET_H_
