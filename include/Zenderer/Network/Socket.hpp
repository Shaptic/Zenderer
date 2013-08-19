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

    /// A low-level socket wrapper.
    class ZEN_API CSocket
    {
    public:
        CSocket(const SocketType& Type) : m_Type(Type),
            m_socket(-1), m_Log(util::GetEngineLog()) {}

        virtual ~CSocket() { this->Destroy(); }

        static bool InitializeLibrary();

        bool Init(const string_t& host, const string_t& port);
        bool Destroy();

        /**
         * Receives any incoming data on the socket.
         *  This call will block the process until some amount of data
         *  is received, unless SetNonblocking() is called.
         *
         * @param   size        The maximum size of the data to receive
         * @param   address     String to store the IP address of the sender
         * @param   address     String to store the port of the sender
         *
         * @return  Data that was received from the socket, if any.
         *
         * @warning Packets less than `size` will NOT be removed from
         *          the socket queue.
         **/
        string_t RecvFrom(const size_t size, string_t& address, string_t& port);

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
        int SendTo(const std::string& addr, const std::string& port,
                   const std::string& data);

        /**
         **/
        int SendBroadcast(const string_t& message, const string_t& port = "");

        bool Ping();
        bool SetSocketOption(const int type, const int option, const bool flag);

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
        bool SetNonblocking(const bool flag);

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
        int GetError() const;

    private:
        static string_t GetAddress(sockaddr_in& addr);
        static in_addr GetAddress(const std::string& ip);

        static bool s_init;
        util::CLog& m_Log;
        SocketType m_Type;
        int m_socket;
    };
}
}

#endif // ZENDERER__NETWORK__SOCKET_HPP

/**
 * @class   zen::net::CSocket
 * @details
 *  This API provides a low-level abstraction layer over a lot of unwieldy 
 *  and OS-dependant socket I/O operations.
 *
 * @note    There is currently only support for UDP communication via
 *          `sendto()` and `recvfrom()`.
 *
 * @todo    Support for TCP, `connect()`, etc.
 *
 * @example Sockets
 * @section udp Simple UDP Echo Client / Server
 * Client code
 * @code
 * #include "Zenderer/Network/Socket.hpp"
 * using namespace zen::net;
 *
 * CSocket Client(PacketType::UDP);
 * Client.Init("", "6969");
 * Client.SendTo("localhost", "7000", "Hello, Server!");
 * Client.Destroy();
 * @endcode
 *
 * Server code
 * @code
 * #include "Zenderer/Network/Socket.hpp"
 * using namespace zen::net;
 *
 * CSocket Server(PacketType::UDP);
 * Server.Init("", "7000");
 * zen::string_t addr, port;
 * zen::string_t data = Server.RecvFrom(1024, addr, port);
 * std::reverse(data.begin(), data.end());
 * Server.SendTo(addr, port, data);
 * Client.Destroy();
 * @endcode
 **/

/** @} **/
