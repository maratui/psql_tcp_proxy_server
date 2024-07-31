#include "berkeley_socket.h"

using namespace psql_tcp;

int BerkeleySocket::CreateServerSocket(unsigned server_ip_address,
                                       unsigned server_port) {
  struct sockaddr_in sockaddr {};
  int client_listener_fd{};
  int function_result{};

  /*
    socket() - создаёт конечную точку соединения и возвращает дескриптор
    AF_INET - для сетевого протокола IPv4
    SOCK_STREAM - надёжная потокоориентированная служба (TCP)
    IPPROTO_TCP (Transmission Control Protocol — протокол управления передачей)
    — один из основных протоколов передачи данных интернета
  */
  client_listener_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  CheckResult(client_listener_fd, "ошибка при создании сокета для сервера");

  /*
    позволяет сокету принудительно привязаться к адресу, используемому другим
    сокетом
  */
  SetReuseSockOpt(client_listener_fd);

  /*
    cтруктура sockaddr_in описывает сокет для работы с протоколами IP
  */
  sockaddr = GetSockaddrIn(server_ip_address, server_port);

  /*
    bind() - связывает сокет с конкретным адресом
  */
  function_result =
      bind(client_listener_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
  CheckResult(function_result, client_listener_fd,
              "ошибка связывания сокета сервера с конкретным адресом");

  /*
    нужно чтобы для обработки одного пакета мы вызвали select только один раз
  */
  SetNonblockFD(client_listener_fd);

  /*
    listen() - подготавливает привязываемый сокет к принятию входящих соединений
    (так называемое «прослушивание»)
    SOMAXCONN - число установленных соединений,
    которые могут быть обработаны в любой момент времени
  */
  function_result = listen(client_listener_fd, SOMAXCONN);
  CheckResult(function_result, client_listener_fd, "Ошибка прослушивания");

  return client_listener_fd;
}

int BerkeleySocket::CreateClientSocket(const std::string& client_ip_address,
                                       unsigned client_port) {
  struct sockaddr_in sockaddr {};
  int client_socket_fd{};
  int function_result{};

  client_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket_fd < 0) {
    function_result = -1;
    std::cout << "ошибка при создании сокета для клиента" << std::endl;
  } else {
    SetReuseSockOpt(client_socket_fd);
    function_result = SetSockaddrIn(sockaddr, client_ip_address, client_port);
    CheckResult(function_result, client_socket_fd, "Ошибка SetSockaddrIn");
  }

  /*
    connect() - устанавливает соединение с сервером
    загруженный сервер может отвергнуть попытку соединения, поэтому
    предусматриваем повторные попытки соединения
  */
  if (function_result > -1) {
    int connect_count{};
    bool is_not_connect{};

    connect_count = kMaxConn;
    do {
      function_result = connect(client_socket_fd, (struct sockaddr*)&sockaddr,
                                sizeof(sockaddr));
      is_not_connect = (--connect_count > 0) && (function_result < 0);
      if (is_not_connect) {
        usleep(10);
      }
    } while (is_not_connect);
    if (function_result < 0) {
      close(client_socket_fd);
      client_socket_fd = -1;
      std::cout << "ошибка соединения с сервером" << std::endl;
    }
  }

  if (function_result > -1) {
    SetNonblockFD(client_socket_fd);
  }

  return client_socket_fd;
}

int BerkeleySocket::AcceptConnection(int client_listener_fd) {
  int client_socket_fd{};

  /*
    accept() - используется для принятия запроса на установление соединения от
    удаленного хоста
  */
  client_socket_fd = accept(client_listener_fd, NULL, NULL);
  CheckResult(client_socket_fd, client_listener_fd,
              "Ошибка принятия запроса на установление соединения");

  SetNonblockFD(client_socket_fd);

  return client_socket_fd;
}

void BerkeleySocket::SetReuseSockOpt(int socket_fd) {
  int reuse = 1;

  /*
    Указывает, что правила, используемые при проверке адресов
    предоставленный в вызове bind() должен позволять повторное использование
    локального адреса
  */
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse,
                 sizeof(reuse)) < 0) {
    std::cout << "ошибка при установлении setsockopt(SO_REUSEADDR)"
              << std::endl;
  }

  /*
    Позволяет привязывать несколько сокетов на идентичный адрес сокета
  */
#if defined(SO_REUSEPORT)
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse,
                 sizeof(reuse)) < 0) {
    std::cout << "ошибка при установлении setsockopt(SO_REUSEPORT)"
              << std::endl;
  }
#endif
}

void BerkeleySocket::SetNonblockFD(int fd) {
  int flags{};
  int function_result;

  /*
    O_NONBLOCK - устанавливает  режим  неблокирования (стандартизировано POSIX)
    до стандартизации были ioctl(... FIONBIO... ) и fcntl(... O_NDELAY... )
  */
#if defined(O_NONBLOCK)
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
    flags = 0;
  }
  function_result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  function_result = ioctl(fd, FIONBIO, &flags);
#endif

  if (function_result < 0) {
    std::cout << "ошибка при установлении режима неблокирования для сокета"
              << std::endl;
  }
}

struct sockaddr_in BerkeleySocket::GetSockaddrIn(unsigned ip_address,
                                                 unsigned port) {
  struct sockaddr_in sockaddr;

  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr.s_addr = htonl(ip_address);

  return sockaddr;
}

int BerkeleySocket::SetSockaddrIn(struct sockaddr_in& sockaddr,
                                  const std::string& ip_address,
                                  unsigned port) {
  int result = 0;

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

  return result;
}

void BerkeleySocket::CheckResult(int result, const std::string& log_text) {
  if (result < 0) {
    utils::ExitWithLog(log_text);
  }
}

void BerkeleySocket::CheckResult(int result, int socket_fd,
                                 const std::string& log_text) {
  if (result < 0) {
    close(socket_fd);
    utils::ExitWithLog(log_text);
  }
}
