#ifndef PSQL_TCP_PROXY_SERVER_BRIDGE_H_
#define PSQL_TCP_PROXY_SERVER_BRIDGE_H_

#include <algorithm>
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
  struct message_s {
    std::string string;
    size_t sent_length;
    size_t length;
  };

  static const inline std::string kLocalHost = "127.0.0.1";
  static const int kPSQLPortNumber = 5432;
  static const int kBufLen = 10240;

  Bridge() = delete;
  Bridge(const Bridge &other) = delete;
  Bridge(Bridge &&other) = delete;
  void operator=(const Bridge &other) = delete;
  void operator=(const Bridge &&other) = delete;

  static int WriteQueryToLog(const std::string &query);

  int Recv(int socket_fd, struct message_s &message);
  int Send(int socket_fd, struct message_s &message);
  long unsigned GetMessageLength(const std::string &message);

  static inline std::string filename_ = "";

  struct message_s client_request_;
  struct message_s psql_response_;

  int client_socket_{};
  int psql_socket_{};
  int status_{};

  char buf_[kBufLen]{};
  ssize_t read_bytes_{};
  ssize_t write_bytes_{};
};
}  // namespace psql_tcp

#endif  // PSQL_TCP_PROXY_SERVER_BRIDGE_H_
