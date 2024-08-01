#ifndef PSQL_TCP_PROXY_SERVER_BRIDGE_H_
#define PSQL_TCP_PROXY_SERVER_BRIDGE_H_

#include <fstream>

#include "berkeley_socket.h"

namespace psql_tcp {
class Bridge {
 public:
  explicit Bridge(int socket_fd, int status);
  ~Bridge();

  static void SetFilename(const std::string &filename) noexcept;

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
  const static inline std::string kLocalHost = "127.0.0.1";

  static inline std::string filename_ = "";

  Bridge() = delete;
  Bridge(const Bridge &other) = delete;
  Bridge(Bridge &&other) = delete;
  void operator=(const Bridge &other) = delete;
  void operator=(const Bridge &&other) = delete;

  static void WriteLog(const std::string &log_text);
  void SetMessageLength();

  int client_socket_{};
  int psql_socket_{};
  int status_{};

  char buf_[kBufLen]{};
  int read_bytes_{};
  long write_bytes_{};

  std::string client_request_ = "";
  std::string psql_response_ = "";
  long unsigned client_message_length_ = 0LU;
  long unsigned sent_request_length_ = 0LU;
  long unsigned message_length_ = 0LU;
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BRIDGE_H_
