/*
 * @Author: your name
 * @Date: 2021-11-29 15:21:24
 * @LastEditTime: 2021-12-01 15:57:37
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /irono/irono/net/SocketsOps.h
 */
#pragma once
#include <arpa/inet.h>
#include <endian.h>

namespace irono
{
namespace sockets
{

inline uint64_t hostToNetwork64(uint64_t host64)
{
    return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
    return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
    return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
    return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
    return ntohl(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
    return ntohs(net16);
}

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie();
int  connect(int sockfd, const struct sockaddr_in& addr);
void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int  accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toHostPort(char* buf, size_t size,
                const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port,
                  struct sockaddr_in* addr);
                  
struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
int getSocketError(int sockfd);

//是否自连接
bool isSelfConnect(int sockfd);
}

}
