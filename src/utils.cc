#include "utils.h"

int utils::SetNonblockFD(int fd) {
  int flags{};
  int result{};

#if defined(O_NONBLOCK)
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
    flags = 0;
  }
  result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  result = ioctl(fd, FIONBIO, &flags);
#endif

  return result;
}

void utils::OutMessage(const std::string& message) {
  std::cout << message << "\n";
}

void utils::WriteLog(const std::string& filename, const std::string& log_text) {
  std::ofstream out(filename.c_str(), std::ios::app);

  if (out.is_open()) {
    out << std::endl << log_text << std::endl;
    out.close();
  } else {
    utils::ExitWithLog("Error occured when open log file");
  }
}

void utils::CheckResult(int result, const std::string& log_text) {
  if (result < 0) {
    utils::ExitWithLog(log_text);
  }
}

void utils::Log(const std::string& log_text) {
  if (errno) {
    std::cerr << log_text << ": " << std::strerror(errno) << std::endl;
  } else {
    std::cerr << log_text << std::endl;
  }
}

void utils::ExitWithLog(const std::string& log_text) {
  if (errno) {
    std::cerr << log_text << ": " << std::strerror(errno) << std::endl;
    std::cout << "Завершение работы приложения" << std::endl;
    exit(errno);
  } else {
    std::cerr << log_text << std::endl;
    std::cout << "Завершение работы приложения" << std::endl;
    exit(EXIT_FAILURE);
  }
}
