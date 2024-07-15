#ifndef PSQL_TCP_PROXY_SERVER_BRIDGE_H_
#define PSQL_TCP_PROXY_SERVER_BRIDGE_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

namespace psql_tcp {
class Bridge {
 public:
  Bridge() = delete;
  explicit Bridge(int socket_fd, int status, const std::string &filename);
  ~Bridge();

  int RecvRequest();
  int RecvResponse();
  int SendRequest();
  int SendResponse();

  int GetClientSocket() const noexcept;
  int GetServerSocket() const noexcept;
  int GetStatus() const noexcept;
  void SetStatus(int status) noexcept;

 private:
  const static int kPSQLPortNumber = 5432;
  const static int kBufLen = 1024;

  void SetQueryMessageLength();

  int client_socket_;
  int status_;
  std::string filename_;

  int psql_socket_{};

  char buf_[kBufLen]{};
  int read_bytes_{};
  long write_bytes_{};

  std::string client_request_ = "";
  long client_message_length_ = 0L;
  long sent_request_length_ = 0L;

  std::string psql_response_ = "";

  long unsigned query_message_length_ = 0LU;

  int function_result_{};
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BRIDGE_H_
