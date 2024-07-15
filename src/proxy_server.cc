#include "proxy_server.h"

using namespace psql_tcp;

ProxyServer::ProxyServer(char *argv[]) : filename_(argv[2]) {
  std::istringstream str_iss(argv[1]);
  int proxy_server_port{};
  struct sockaddr_in proxy_server_sockaddr;

  str_iss >> proxy_server_port;
  proxy_server_sockaddr = utils::GetSockaddrIn(proxy_server_port);

  client_listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  utils::CheckResult(client_listener_, "client_listener");

  function_result_ =
      bind(client_listener_, (struct sockaddr *)&proxy_server_sockaddr,
           sizeof(proxy_server_sockaddr));
  utils::CheckResult(function_result_, "bind");

  function_result_ = utils::SetNonblockFD(client_listener_);
  utils::CheckResult(function_result_, "SetNonblockFD");

  function_result_ = listen(client_listener_, SOMAXCONN);
  utils::CheckResult(function_result_, "listen");
}

ProxyServer::~ProxyServer() {
  for (auto bridges_iter = bridges_.begin(); bridges_iter != bridges_.end();
       ++bridges_iter) {
    close((*bridges_iter)->GetClientSocket());
    close((*bridges_iter)->GetServerSocket());
  }
}

void ProxyServer::Run() {
  while (true) {
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    SetFDSet();

    function_result_ =
        select(sockets_max_ + 1, &read_fd_set_, &write_fd_set_, NULL, NULL);
    utils::CheckResult(function_result_, "select");

    if (FD_ISSET(client_listener_, &read_fd_set_)) {
      AcceptConnection();
    }

    ProcessConnections();
  }
}

void ProxyServer::SetFDSet() {
  std::list<int> sockets;

  sockets.push_back(client_listener_);
  FD_SET(client_listener_, &read_fd_set_);

  for (auto bridges_iter = bridges_.begin(); bridges_iter != bridges_.end();
       ++bridges_iter) {
    int status = (*bridges_iter)->GetStatus();
    int socket_fd = 0;

    if (status == kRecvRequest) {
      socket_fd = (*bridges_iter)->GetClientSocket();
      FD_SET(socket_fd, &read_fd_set_);
    } else if (status == kRecvResponse) {
      socket_fd = (*bridges_iter)->GetServerSocket();
      FD_SET(socket_fd, &read_fd_set_);
    } else if (status == kSendResponse) {
      socket_fd = (*bridges_iter)->GetClientSocket();
      FD_SET(socket_fd, &write_fd_set_);
    } else if (status == kSendRequest) {
      socket_fd = (*bridges_iter)->GetServerSocket();
      FD_SET(socket_fd, &write_fd_set_);
    }
    sockets.push_back(socket_fd);
  }

  sockets_max_ = *std::max_element(sockets.begin(), sockets.end());
}

void ProxyServer::AcceptConnection() {
  int client_socket = accept(client_listener_, NULL, NULL);

  utils::CheckResult(client_socket, "accept");

  function_result_ = utils::SetNonblockFD(client_socket);
  utils::CheckResult(function_result_, "SetNonblockFD");

  bridges_.push_back(new Bridge(client_socket, kRecvRequest, filename_));
}

void ProxyServer::ProcessConnections() {
  for (auto bridges_iter = bridges_.begin(); bridges_iter != bridges_.end();) {
    int client_socket = (*bridges_iter)->GetClientSocket();
    int server_socket = (*bridges_iter)->GetServerSocket();
    int result = 0;

    if (FD_ISSET(client_socket, &read_fd_set_)) {
      if ((result = (*bridges_iter)->RecvRequest()) == 0) {
        (*bridges_iter)->SetStatus(kSendRequest);
      }
    } else if (FD_ISSET(server_socket, &read_fd_set_)) {
      if ((result = (*bridges_iter)->RecvResponse()) == 0) {
        (*bridges_iter)->SetStatus(kSendResponse);
      }
    }

    if (FD_ISSET(server_socket, &write_fd_set_)) {
      if ((result = (*bridges_iter)->SendRequest()) == 0) {
        (*bridges_iter)->SetStatus(kRecvResponse);
      }
    } else if (FD_ISSET(client_socket, &write_fd_set_)) {
      if ((result = (*bridges_iter)->SendResponse()) == 0) {
        (*bridges_iter)->SetStatus(kRecvRequest);
      }
    }

    if (result < 0) {
      delete (*bridges_iter);
      bridges_iter = bridges_.erase(bridges_iter);
      utils::OutMessage("connection closed");
    } else {
      ++bridges_iter;
    }
  }
}
