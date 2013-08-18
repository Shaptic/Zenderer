/**
 * @file
 *  Zenderer/Network/Socket.hpp - A low-level, cross-platform, socket
 *  wrapper API class.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup Network
 *  A group with interfaces and routines for more simplified multiplayer
 *  or networked development that allows for an easy cross platform interface
 *  to be shared for a variety of architectures. The group provides
 *  facilities for peer-to-peer, client-to-server, and other standard
 *  communication protocols for various applications.
 *
 * @{
 **/

#ifndef ZENDERER__NETWORK__SOCKET_HPP
#define ZENDERER__NETWORK__SOCKET_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Utilities/Log.hpp"

#ifdef _WIN32
  #include <thread>
  #include <WinSock2.h>
  #include <WS2TCPIP.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <unistd.h>
  #include <errno.h>
  #include <netdb.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif // _WIN32

namespace zen
{
namespace net
{
// Windows has a custom ICMP API that avoids the need
// for raw sockets and administrative rights.
#ifndef _WIN32
#pragma pack(1)
    struct IPHeader
    {
        unsigned int    h_len:4;
        unsigned int    version:4;
        uint8_t         tos;
        uint16_t        total_len;
        uint16_t        id;
        uint16_t        flags;
        uint8_t         ttl;
        uint8_t         proto;
        uint16_t        chk;
        uint32_t        source_ip;
        uint32_t        dest_ip;
    };

    struct ICMPHeader
    {
        uint8_t     type;
        uint8_t     code;
        uint16_t    chk;
        uint16_t    id;
        uint16_t    seq;
        uint32_t    time;
    };
#pragma pack()

    enum ICMPReply
    {
        ICMP_ECHO_REPLY         = 0,
        ICMP_DEST_UNREACHABLE   = 3,
        ICMP_ECHO_REQUEST       = 8
        ICMP_TTL_EXPIRE         = 11,
    };

    enum class ICMPPacket
    {
        PACKET_SIZE     = 8,
        DEFAULT_SIZE    = 32,
        MAX_DATA_SIZE   = 1024,
        MAX_PACKET_SIZE = MAX_DATA_SIZE + sizeof(IPHeader);
    };
#endif  // _WIN32

    enum class SocketType { TCP, UDP, RAW };

    using util::CLog;
    using util::LogMode;

    class ZEN_API CSocket
    {
    public:
        CSocket(const SocketType& Type) : m_Type(Type), m_socket(-1) {}
        virtual ~CSocket() { this->Destroy(); }

        static bool InitializeLibrary()
        {
            //if(s_init) return true;

#ifdef _WIN32
            WSADATA Data;
            if(WSAStartup(MAKEWORD(2, 0), &Data) != 0)
            {
                std::cerr << "WinSock2 failed to initialize!\n";
                return (/*s_init =*/ false);
            }
#endif  // _WIN32
            return (/*s_init =*/ true);
        }

        virtual bool Init(const string_t& host, const string_t& port)
        {
            addrinfo hints;
            addrinfo* tmp;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_flags = AI_PASSIVE ? host.empty() : 0;

            if(m_Type == SocketType::TCP)
                hints.ai_socktype = SOCK_STREAM;

            else if(m_Type == SocketType::RAW)
                hints.ai_socktype = SOCK_RAW;

            if(getaddrinfo(host.empty() ? nullptr : host.c_str(),
                           port.empty() ? nullptr : port.c_str(),
                           &hints, &tmp) != 0)
            {
                return false;
            }

            auto result = tmp;
            for( ; result != nullptr; result = result->ai_next)
            {
                if((m_socket = socket(result->ai_family,
                                      result->ai_socktype,
                                      result->ai_protocol)) == -1)
                {
                    continue;
                }

                this->SetSocketOption(SOL_SOCKET, SO_REUSEADDR, true);

                if(bind(m_socket, result->ai_addr, result->ai_addrlen) == -1)
                {
                    this->Destroy();
                    continue;
                }
            }

            if(m_socket == -1) return false;

            freeaddrinfo(tmp);
            return true;
        }

        bool Destroy()
        {
            if(m_socket == -1) return false;
#ifdef _WIN32
            closesocket(m_socket);
#else
            close(m_socket);
#endif // _WIN32
            return (m_socket = -1);
        }

        /**
         * Receives any incoming data on the socket.
         *  This call will block the process until some amount of data
         *  is received, unless SetNonblocking() is called.
         *
         * @param   size    The maximum size of the data to receive
         * @param   address The IP address of the sender
         *
         * @return  Data that was received from the socket, if any.
         *
         * @warning Packets less than `size` will NOT be removed from
         *          the socket queue.
         **/
        string_t RecvFrom(const size_t size, string_t& address, string_t& port)
        {
            if(m_socket <= 0) return "";

            address = "";
            sockaddr_in addr;
            int addrlen = sizeof(addr);
            char* buffer = new char[size];

            int bytes = recvfrom(m_socket, buffer, size, 0,
                                 (sockaddr*)&addr, &addrlen);
            if(bytes == -1)
            {
                delete[] buffer;
                return string_t("");
            }

            address = CSocket::GetAddress(addr);
            std::stringstream ss;
            ss << ntohs(addr.sin_port);
            port = ss.str();

            string_t ret(buffer, bytes);
            delete[] buffer;

#ifdef ZEN_DEBUG_BUILD
            CLog& Log = CLog::GetEngineLog();
            Log << Log.SetMode(LogMode::ZEN_DEBUG) << Log.SetSystem("Network")
                << "Received '" << ret << "' from " << address << ':'
                << port << '.' << CLog::endl;
#endif  // ZEN_DEBUG_BUILD

            return ret;
        }

        /**
         * Sends a message to an IP:port address.
         *
         * @param   addr    The IP address to send to
         * @param   port    The port to send to
         * @param   data    The data to send
         *
         * @return  The number of bytes sent, or -1 on error.
         *
         * @warning This is only available for UDP sockets.
         *
         * @see     GetError()
         * @see     SendAll()
         * @see     SocketType
         **/
        int SendTo(const std::string& addr,
                   const std::string& port,
                   const std::string& data)
        {
            if(m_socket < 0 || m_Type == SocketType::TCP) return -1;

#ifdef ZEN_DEBUG_BUILD
            CLog& Log = CLog::GetEngineLog();
            Log << Log.SetMode(LogMode::ZEN_DEBUG) << Log.SetSystem("Network")
                << "Sending '" << data << "' to " << addr << ':' << port
                << '.' << CLog::endl;
#endif  // ZEN_DEBUG_BUILD

            sockaddr_in sock;
            sock.sin_family = AF_INET;
            sock.sin_port   = htons(std::stoi(port));
            sock.sin_addr   = this->GetAddress(addr);

            return sendto(m_socket, data.c_str(), data.size(), 0,
                         (sockaddr*)&sock, sizeof(sock));
        }

        int SendBroadcast(const string_t& message, const string_t& port)
        {
            this->SetSocketOption(SOL_SOCKET, SO_BROADCAST, true);
            int b = this->SendTo("255.255.255.255", port, message);
            this->SetSocketOption(SOL_SOCKET, SO_BROADCAST, false);
            return b;
        }

        bool Ping()
        {
            ZEN_ASSERTM(false, "not implemented");
        }

        bool SetSocketOption(const int type, const int option, const bool flag)
        {
            if(m_socket < 0) return false;
            char set = flag ? 1 : 0;
            return setsockopt(m_socket, type, option, &set, sizeof set) == 0;
        }

        /**
         * Enables or disables blocking for socket operations.
         *
         *
         * @param   flag    Set blocking mode, or no?
         *
         * @return  `true`  if the non-blocking flag could be set,
         *          `false` otherwise, or if a socket hasn't been created.
         *
         *
         **/
        bool SetNonblocking(const bool flag)
        {
            if(m_socket < 0) return false;

#ifdef _WIN32
            unsigned long set = flag ? 1 : 0;
            return ioctlsocket(m_socket, FIONBIO, &set) == 0;
#else
            int set = flag ? 1 : 0;
            return fcntl(m_socket, F_SETFL, O_NONBLOCK, &set) == 0;
#endif
        }

        /**
         * Retrieves the last error number.
         *
         * @return  The last error code, if any.
         *
         * @warning This is platform dependant.
         *
         * @see http://linux.die.net/man/3/errno
         * @see http://msdn.microsoft.com/en-us/library/aa924071.aspx
         **/
        int GetError() const
        {
#ifdef _WIN32
            return WSAGetLastError();
#else
            return errno;
#endif // _WIN32
        }

    private:
        static string_t GetAddress(sockaddr_in& addr)
        {
            char ip[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
            return string_t(ip);
        }

        static in_addr GetAddress(const std::string& ip)
        {
            in_addr s;
            uint32_t addr;
            inet_pton(AF_INET, ip.c_str(), &s);
            return s;
        }

        /*static*/ bool s_init;
        SocketType m_Type;
        int m_socket;
    };
}

    /*
    int main()
    {
        net::CSocket::InitializeLibrary();

        {
            net::CSocket Server(net::SocketType::UDP);
            Server.Init("localhost", "12345");

            string_t cli;
            std::cout << "Client sent: '" << Server.RecvFrom(1024, cli) << "'\n";

            Server.Destroy();
        }

        {
            net::CSocket Client(net::SocketType::UDP);
            std::cout << "Client sent "
                        << Client.SendTo("localhost", "12345", "What's up dudes?")
                        << " bytes.\n";
            Client.Destroy();
        }

        return 0;
    }
    */
}

#endif // ZENDERER__NETWORK__SOCKET_HPP

/** @} **/
