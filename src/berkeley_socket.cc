#include "berkeley_socket.h"

using namespace psql_tcp;

int BerkeleySocket::CreateServerSocket(unsigned server_ip_address,
                                       unsigned server_port) {
  struct sockaddr_in sockaddr;
  int client_listener_fd;
  int function_result;

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
    (так называемое «прослушивание») SOMAXCONN - число установленных соединений,
    которые могут быть обработаны в любой момент времени
  */
  function_result = listen(client_listener_fd, SOMAXCONN);
  CheckResult(function_result, client_listener_fd, "Ошибка прослушивания");

  return client_listener_fd;
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
