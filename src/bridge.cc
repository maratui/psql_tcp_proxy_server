#include "bridge.h"

using namespace psql_tcp;

Bridge::Bridge(int socket_fd, int status)
    : client_socket_(socket_fd), status_(status) {
  psql_socket_ =
      BerkeleySocket::CreateClientSocket(kLocalHost, kPSQLPortNumber);

  if (psql_socket_ > -1) {
    client_request_.string = "";
    client_request_.sent_length = 0;
    client_request_.length = 0;

    psql_response_.string = "";
    psql_response_.sent_length = 0;
    psql_response_.length = 0;
  }
}

Bridge::~Bridge() {
  if (client_socket_ > -1) {
    shutdown(client_socket_, SHUT_RDWR);
    close(client_socket_);
  }
  if (psql_socket_ > -1) {
    shutdown(psql_socket_, SHUT_RDWR);
    close(psql_socket_);
  }
}

void Bridge::SetFilename(const std::string &filename) noexcept {
  filename_ = filename;
}

int Bridge::RecvRequest() {
  int result{};

  result = Recv(client_socket_, client_request_);
  if ((result == 0) && (errno == EWOULDBLOCK)) {
    if ((client_request_.length > 5) && (client_request_.string[0] == 'Q')) {
      WriteQueryToLog(
          client_request_.string.substr(5, client_request_.length - 6));
    }
  }

  return result;
}

int Bridge::RecvResponse() { return Recv(psql_socket_, psql_response_); }

int Bridge::SendRequest() { return Send(psql_socket_, client_request_); }

int Bridge::SendResponse() { return Send(client_socket_, psql_response_); }

int Bridge::GetClientSocket() const noexcept { return client_socket_; }

int Bridge::GetServerSocket() const noexcept { return psql_socket_; }

int Bridge::GetStatus() const noexcept { return status_; }

void Bridge::SetStatus(int status) noexcept { status_ = status; }

void Bridge::WriteQueryToLog(const std::string &query) {
  std::ofstream out(filename_.c_str(), std::ios::app);

  if (out.is_open()) {
    out << std::endl << query << std::endl;
    out.close();
  } else {
    std::cerr << "Ошибка открытия лог файла" << std::endl;
  }
}

int Bridge::Recv(int socket_fd, struct message_s &message) {
  /*
    Получим все входящие данные на этот сокет прежде чем вернемся и снова
    вызовем select
  */
  do {
    read_bytes_ = recv(socket_fd, buf_, kBufLen, MSG_NOSIGNAL);
    if (read_bytes_ > 0) {
      message.string.append(buf_, read_bytes_);

      if (message.length == 0) {
        if (read_bytes_ < kBufLen) {
          message.length = read_bytes_;
        } else {
          message.length = GetMessageLength(message.string);
        }
      }
    }
  } while (read_bytes_ > 0);

  std::cout << "recv = " << read_bytes_ << "; errno = " << errno
            << "; EAGAIN = " << EAGAIN << "; EWOULDBLOCK = " << EWOULDBLOCK
            << std::endl;
  /*
    Получаем данные по этому соединению до тех пор,
    пока не установится EWOULDBLOCK или EAGAIN.
    Если будут другие ошибки, то произошел сбой и мы закроем связь.
  */
  if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
    if (message.string.length() == message.length) {
      read_bytes_ = 0;
    } else {
      read_bytes_ = 1;
    }
  } else {
    read_bytes_ = -1;
  }

  return read_bytes_;
}

int Bridge::Send(int socket_fd, struct message_s &message) {
  do {
    write_bytes_ = send(socket_fd, message.string.c_str() + message.sent_length,
                        message.length, MSG_NOSIGNAL);
    if (write_bytes_ > 0) {
      message.sent_length += write_bytes_;
      message.length -= write_bytes_;
    }
  } while (write_bytes_ > 0);

  std::cout << "send = " << write_bytes_ << "; errno = " << errno
            << "; EAGAIN = " << EAGAIN << "; EWOULDBLOCK = " << EWOULDBLOCK
            << std::endl;

  if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
    if (message.length == 0) {
      message.string.clear();
      message.sent_length = 0;
      write_bytes_ = 0;
    } else {
      write_bytes_ = 1;
    }
  } else {
    write_bytes_ = -1;
  }

  return write_bytes_;
}

long unsigned Bridge::GetMessageLength(const std::string &message) {
  long unsigned message_length = 0LU;

  if (message.length() >= 5) {
    message_length = (long unsigned)((unsigned char)(message[1]) << 24 |
                                     (unsigned char)(message[2]) << 16 |
                                     (unsigned char)(message[3]) << 8 |
                                     (unsigned char)(message[4])) +
                     1LU;
  }

  return message_length;
}
