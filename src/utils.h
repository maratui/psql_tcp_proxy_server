#ifndef PROXY_SERVER_PSQL_UTILS_H_
#define PROXY_SERVER_PSQL_UTILS_H_

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <cstring>
#include <fstream>
#include <iostream>

namespace utils {
struct sockaddr_in GetSockaddrIn(int port_number);
int SetNonblockFD(int fd);

void OutMessage(const std::string& message);
void WriteLog(const std::string& filename, const std::string& log_text);
void CheckResult(int result, const std::string& log_text);
void ExitWithLog(const std::string& log_text);
}  // namespace utils

#endif  // PROXY_SERVER_PSQL_UTILS_H_
