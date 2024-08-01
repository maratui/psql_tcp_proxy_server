#include "berkeley_socket.h"

using namespace psql_tcp;

int BerkeleySocket::CreateServerSocket(const std::string &ip_address,
                                       unsigned port) {
  struct sockaddr_in sockaddr {};
  int socket_fd{};
  int result{};

  result = CreateSocket(socket_fd);

  if (result > -1) {
    SetReuseSockOpt(socket_fd);
    result = SetSockaddrIn(socket_fd, sockaddr, ip_address, port);
  }

  if (result > -1) {
    result = Bind(socket_fd, sockaddr);
  }

  if (result > -1) {
    SetNonblockFD(socket_fd);
    Listen(socket_fd);
  }

  return socket_fd;
}

int BerkeleySocket::CreateClientSocket(const std::string &ip_address,
                                       unsigned port) {
  struct sockaddr_in sockaddr {};
  int socket_fd{};
  int result{};

  result = CreateSocket(socket_fd);

  if (result > -1) {
    SetReuseSockOpt(socket_fd);
    result = SetSockaddrIn(socket_fd, sockaddr, ip_address, port);
  }

  if (result > -1) {
    result = Connect(socket_fd, sockaddr);
  }

  if (result > -1) {
    SetNonblockFD(socket_fd);
  }

  return socket_fd;
}

int BerkeleySocket::Accept(int listener_fd) {
  int socket_fd{};

  /*
    accept() - используется для принятия запроса на установление соединения от
    удаленного хоста
  */
  socket_fd = accept(listener_fd, NULL, NULL);
  CheckResult(socket_fd, socket_fd,
              "Ошибка принятия запроса на установление соединения");

  if (socket_fd > -1) {
    SetNonblockFD(socket_fd);
  }

  return socket_fd;
}

int BerkeleySocket::CreateSocket(int &socket_fd) {
  int result = 0;

  /*
    socket() - создаёт конечную точку соединения и возвращает дескриптор
    AF_INET - для сетевого протокола IPv4
    SOCK_STREAM - надёжная потокоориентированная служба (TCP)
    IPPROTO_TCP (Transmission Control Protocol — протокол управления передачей)
    — один из основных протоколов передачи данных интернета
  */
  socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd < 0) {
    result = -1;
    std::cout << "ошибка при создании сокета" << std::endl;
  }

  return result;
}

void BerkeleySocket::SetReuseSockOpt(int socket_fd) {
  int reuse = 1;

  /*
    Указывает, что правила, используемые при проверке адресов
    предоставленный в вызове bind() должен позволять повторное использование
    локального адреса
  */
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
                 sizeof(reuse)) < 0) {
    std::cout << "ошибка при установлении setsockopt(SO_REUSEADDR)"
              << std::endl;
  }

  /*
    Позволяет привязывать несколько сокетов на идентичный адрес сокета
  */
#if defined(SO_REUSEPORT)
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse,
                 sizeof(reuse)) < 0) {
    std::cout << "ошибка при установлении setsockopt(SO_REUSEPORT)"
              << std::endl;
  }
#endif
}

int BerkeleySocket::SetSockaddrIn(int &socket_fd, struct sockaddr_in &sockaddr,
                                  const std::string &ip_address,
                                  unsigned port) {
  int result = 0;

  /*
    cтруктура sockaddr_in описывает сокет для работы с протоколами IP
  */
  if ((port < 1024) || (port > 65535)) {
    result = -1;
    std::cout << "ошибка: не корректный порт" << std::endl;
  } else {
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address.c_str(), &sockaddr.sin_addr) == 0) {
      result = -1;
      std::cout << "ошибка: не корректный IP-адрес" << std::endl;
    }
  }
  CheckResult(result, socket_fd, "Ошибка SetSockaddrIn");

  return result;
}

int BerkeleySocket::Bind(int &socket_fd, const struct sockaddr_in &sockaddr) {
  int result = 0;

  /*
    bind() - связывает сокет с конкретным адресом
  */
  result = bind(socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
  CheckResult(result, socket_fd,
              "ошибка связывания сокета с конкретным адресом");

  return result;
}

void BerkeleySocket::SetNonblockFD(int fd) {
  int flags{};
  int result;

  /*
    O_NONBLOCK - устанавливает  режим  неблокирования (стандартизировано POSIX)
    до стандартизации были ioctl(... FIONBIO... ) и fcntl(... O_NDELAY... )
    нужно чтобы для обработки одного пакета мы вызвали select только один раз
  */
#if defined(O_NONBLOCK)
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
    flags = 0;
  }
  result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  result = ioctl(fd, FIONBIO, &flags);
#endif

  if (result < 0) {
    std::cout << "ошибка при установлении режима неблокирования" << std::endl;
  }
}

int BerkeleySocket::Listen(int &socket_fd) {
  int result = 0;

  /*
    listen() - подготавливает привязываемый сокет к принятию входящих соединений
    (так называемое «прослушивание»)
    SOMAXCONN - число установленных соединений,
    которые могут быть обработаны в любой момент времени
  */
  result = listen(socket_fd, SOMAXCONN);
  CheckResult(result, socket_fd, "Ошибка прослушивания");

  return result;
}

int BerkeleySocket::Connect(int &socket_fd,
                            const struct sockaddr_in &sockaddr) {
  int connect_count{};
  bool is_not_connect{};
  int result = 0;

  /*
    connect() - устанавливает соединение с сервером
    загруженный сервер может отвергнуть попытку соединения, поэтому
    предусматриваем повторные попытки соединения
  */
  connect_count = kMaxConn;
  do {
    result = connect(socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    is_not_connect = (--connect_count > 0) && (result < 0);
    if (is_not_connect) {
      usleep(10);
    }
  } while (is_not_connect);
  CheckResult(result, socket_fd, "Ошибка соединения с сервером");

  return result;
}

void BerkeleySocket::CheckResult(int result, int &socket_fd,
                                 const std::string &log_text) {
  if (result < 0) {
    close(socket_fd);
    socket_fd = -1;
    if (errno) {
      std::cerr << log_text << ": " << std::strerror(errno) << std::endl;
    } else {
      std::cerr << log_text << std::endl;
    }
  }
}
