#include "bridge.h"

using namespace psql_tcp;

Bridge::Bridge(int socket_fd, int status, const std::string& filename)
    : client_socket_(socket_fd), status_(status), filename_(filename) {
  psql_socket_ =
      BerkeleySocket::CreateClientSocket(kLocalHost, kPSQLPortNumber);
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

int Bridge::RecvRequest() {
  if ((read_bytes_ = recv(client_socket_, buf_, kBufLen, MSG_NOSIGNAL)) > 0) {
    client_request_.append(buf_, read_bytes_);

    if ((query_message_length_ == 0) && (buf_[0] == 'Q')) {
      SetQueryMessageLength();
    }

    if ((client_request_.size() < kBufLen) ||
        ((client_request_[0] == 'Q') &&
         (client_request_.size() == query_message_length_))) {
      client_message_length_ = client_request_.size();

      if ((client_message_length_ > 0) && (client_request_[0] == 'Q')) {
        query_message_length_ = 0;
        WriteLog(filename_,
                 client_request_.substr(5, client_request_.size() - 6));
      }

      read_bytes_ = 0;
    }
  } else if ((read_bytes_ == 0) && (errno == EAGAIN)) {
    read_bytes_ = 1;
  }

  return read_bytes_;
}

int Bridge::RecvResponse() {
  if ((read_bytes_ = recv(psql_socket_, buf_, kBufLen, MSG_NOSIGNAL)) > 0) {
    psql_response_.append(buf_, read_bytes_);
    read_bytes_ = 0;
  } else if ((read_bytes_ == 0) && (errno == EAGAIN)) {
    read_bytes_ = 1;
  }

  return read_bytes_;
}

int Bridge::SendRequest() {
  if ((write_bytes_ =
           send(psql_socket_, client_request_.c_str() + sent_request_length_,
                client_message_length_, MSG_NOSIGNAL)) > 0) {
    sent_request_length_ += write_bytes_;
    client_message_length_ -= write_bytes_;

    if (client_message_length_ == 0) {
      sent_request_length_ = 0;
      client_request_.clear();
      write_bytes_ = 0;
    }
  } else if ((write_bytes_ == 0) && (errno == EAGAIN)) {
    write_bytes_ = 1;
  }

  return write_bytes_;
}

int Bridge::SendResponse() {
  if ((write_bytes_ = send(client_socket_, psql_response_.c_str(),
                           psql_response_.size(), MSG_NOSIGNAL)) > 0) {
    psql_response_.clear();
    write_bytes_ = 0;
  } else if ((write_bytes_ == 0) && (errno == EAGAIN)) {
    write_bytes_ = 1;
  }

  return write_bytes_;
}

int Bridge::GetClientSocket() const noexcept { return client_socket_; }

int Bridge::GetServerSocket() const noexcept { return psql_socket_; }

int Bridge::GetStatus() const noexcept { return status_; }

void Bridge::SetStatus(int status) noexcept { status_ = status; }

void Bridge::SetQueryMessageLength() {
  if ((client_request_.length() >= 5) && (client_request_[0] == 'Q')) {
    query_message_length_ =
        (long unsigned)((unsigned char)(client_request_[1]) << 24 |
                        (unsigned char)(client_request_[2]) << 16 |
                        (unsigned char)(client_request_[3]) << 8 |
                        (unsigned char)(client_request_[4])) +
        1LU;
  }
}

void Bridge::WriteLog(const std::string& filename,
                      const std::string& log_text) {
  std::ofstream out(filename.c_str(), std::ios::app);

  if (out.is_open()) {
    out << std::endl << log_text << std::endl;
    out.close();
  } else {
    std::cerr << "Ошибка открытия лог файла" << std::endl;
  }
}
