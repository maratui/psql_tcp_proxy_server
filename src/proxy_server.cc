#include "proxy_server.h"

using namespace psql_tcp;

ProxyServer::ProxyServer(char *argv[]) {
  std::istringstream str_iss(argv[1]);
  unsigned proxy_server_port{};

  str_iss >> proxy_server_port;
  client_listener_ =
      BerkeleySocket::CreateServerSocket(kLocalHost, proxy_server_port);
  CheckResult(client_listener_, "Ошибка создания сокета прокси-сервера");

  Bridge::SetFilename(argv[2]);
}

ProxyServer::~ProxyServer() {
  if (client_listener_ > -1) {
    shutdown(client_listener_, SHUT_RDWR);
    close(client_listener_);
  }
  for (auto bridges_iter = bridges_.begin(); bridges_iter != bridges_.end();
       ++bridges_iter) {
    delete (*bridges_iter);
  }
}

void ProxyServer::Run() {
  /*
    Циклическое ожидание входящих соединений или входящих данных
    на любом из подключенных сокетов.
  */
  do {
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    SetFDSet();

    /*
      select() - позволяяет приложениям мультиплексировать свои операции
      ввода-вывода
    */
    std::cout << "Ожидание select() ..." << std::endl;
    // std::cin >> result_;
    result_ =
        select(max_socket_ + 1, &read_fd_set_, &write_fd_set_, NULL, NULL);
    CheckResult(result_, "Ошибка select()");

    if (result_ > 0) {
      sockets_ready_ = result_;
      std::cout << sockets_ready_ << " сокетов готовы к обработке "
                << std::endl;
      if (FD_ISSET(client_listener_, &read_fd_set_)) {
        AcceptConnection();
      }
      ProcessConnections();
    } else if (result_ == 0) {
      std::cout << "select() завершился с результатом 0." << std::endl;
      std::cout << "Завершение работы приложения" << std::endl;
    }
  } while (result_ > 0);
}

void ProxyServer::SetFDSet() {
  max_socket_ = client_listener_;
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
    if (socket_fd > max_socket_) {
      max_socket_ = socket_fd;
    }
  }
}

void ProxyServer::AcceptConnection() {
  std::cout << "Прослушиваемый сокет " << client_listener_
            << " доступен для чтения" << std::endl;
  do {
    new_socket_ = BerkeleySocket::Accept(client_listener_);

    if (new_socket_ > -1) {
      Bridge *bridge = new Bridge(new_socket_, kRecvRequest);

      std::cout << "Новое входящее соединение - " << new_socket_ << std::endl;
      if (bridge->GetServerSocket() > -1) {
        bridges_.push_back(bridge);
      } else {
        delete (bridge);
        std::cout << "Ошибка при создании моста" << std::endl;
      }
    } else if (errno != EWOULDBLOCK) {
      /*
        Принятие не выполняется с ошибкой EWOULDBLOCK когда мы их всех приняли.
        Любая другая ошибка завершает работу прокси-сервера.
      */
      std::cerr << "Ошибка принятия запроса на установление соединения"
                << std::endl;
      result_ = 0;
    }
  } while (new_socket_ > -1);

  sockets_ready_--;
}

void ProxyServer::ProcessConnections() {
  for (auto bridges_iter = bridges_.begin();
       (bridges_iter != bridges_.end()) && (sockets_ready_ > 0);) {
    int client_socket = (*bridges_iter)->GetClientSocket();
    int server_socket = (*bridges_iter)->GetServerSocket();
    int result = 0;

    if (FD_ISSET(client_socket, &read_fd_set_)) {
      std::cout << "Сокет клиента " << client_socket
                << " доступен для чтения запроса\n";
      if ((result = (*bridges_iter)->RecvRequest()) == 0) {
        (*bridges_iter)->SetStatus(kSendRequest);
      }
      sockets_ready_--;
    } else if (FD_ISSET(server_socket, &read_fd_set_)) {
      std::cout << "Сокет psql " << server_socket
                << " доступен для чтения ответа на запрос\n";
      if ((result = (*bridges_iter)->RecvResponse()) == 0) {
        (*bridges_iter)->SetStatus(kSendResponse);
      }
      sockets_ready_--;
    }

    if (FD_ISSET(server_socket, &write_fd_set_)) {
      std::cout << "Сокет psql " << server_socket
                << " доступен для отправки запроса\n";
      if ((result = (*bridges_iter)->SendRequest()) == 0) {
        (*bridges_iter)->SetStatus(kRecvResponse);
      }
      sockets_ready_--;
    } else if (FD_ISSET(client_socket, &write_fd_set_)) {
      std::cout << "Сокет клиента " << client_socket
                << " доступен для отправки ответа на запрос\n";
      if ((result = (*bridges_iter)->SendResponse()) == 0) {
        (*bridges_iter)->SetStatus(kRecvRequest);
      }
      sockets_ready_--;
    }

    if (result < 0) {
      delete (*bridges_iter);
      bridges_iter = bridges_.erase(bridges_iter);
      std::cout << "\nМост " << client_socket << " -- " << server_socket
                << " закрыт\n"
                << std::endl;
    } else {
      ++bridges_iter;
    }
  }
}

void ProxyServer::CheckResult(int result, const std::string &log_text) {
  if (result < 0) {
    std::cerr << log_text << std::endl;
    std::cout << "Завершение работы приложения" << std::endl;
    exit(EXIT_FAILURE);
  }
}
