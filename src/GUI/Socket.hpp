#include "Zenderer/Core/Types.hpp"

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

    class ZEN_API CSocket
    {
    public:
        CSocket(const SocketType& Type) : m_type(Type), m_socket(-1) {}
        virtual ~CSocket();
        
        static bool InitializeLibrary()
        {
            if(s_init) return true;
            
#ifdef _WIN32
            WSADATA Data;
            if(WSAStartup(MAKEWORD(2, 0), &Data) != 0)
            {
                std::cerr << "WinSock2 failed to initialize!\n";
                return false;
            }
#endif  // _WIN32
            return (s_init = true);
        }
        
        bool Init(const string_t& host, const string_t& port)
        {
            addrinfo hints;
            addrinfo* tmp;
            
            memset(&hints, NULL, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;
                
            if(m_Type == SocketType::TCP)
                hints.ai_socktype = SOCK_STREAM;
                
            else if(m_Type == SocketType::RAW)
                hints.ai_socktype = SOCK_RAW;

            if(getaddrinfo(host, port, &hints, &tmp) != 0)
            {
                return false;
            }
            
            auto result = tmp;
            for( ; result != nullptr; result = result->ai_next)
            {
                if((m_socket = socket(result->ai_family,
                                      result->ai_socktype,
                                      result->ai_protocol) == -1)
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
            
            if(result == nullptr) return false;
            
            freeaddrinfo(tmp);
        }
        
        bool Destroy()
        {
#ifdef _WIN32
            closesocket(m_socket);
#else
            close(m_socket);
#endif // _WIN32
        }
        
        string_t RecvFrom(const size_t size, string_t& address)
        {
            if(m_socket <= 0) return -1;
            
            sockaddr_in addr;
            int addrlen = 0;
            char* buffer = new char[size];
            
            int bytes = recv(m_socket, buffer, size, 0, &addr, &addrlen);
            if(bytes == -1)
            {
                delete[] buffer;
                return string_t("");
            }
            
            address = CSocket::GetAddress(addr);
            
            string_t ret(buffer, bytes);
            delete[] buffer;
            
            return ret;
        }
        
        int SendTo(const std::string& addr, const std::string& port, const std::string& data)
        {
            if(m_socket < 0) return -1;
            
            sockaddr_in sock = { CSocket::GetAddress(addr) };
            return sendto(m_socket, data.c_str(), data.size(), 0, &sock, sizeof(sock));
        }
        
        bool SetSocketOption(const int type, const int option, const bool flag)
        {
            if(m_socket < 0) return false;
            int set = flag ? 1 : 0;
            return setsockopt(m_socket, type, option, &set) == 0;
        }
        
        bool SetNonblocking(const bool flag)
        {
            if(m_socket < 0) return false;
            
            int set = flag ? 1 : 0;
#ifdef _WIN32
            return ioctlsocket(m_socket, FIONBIO, &set) == 0;
#endif
        }
        
    private:
        static string_t GetAddress(sockaddr_in& addr)
        {
            char ip[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
            return string_t(ip);
        }
        
        static uint32_t GetAddress(const std::string& ip)
        {
            uint32_t addr;
            inet_pton(AF_INET, ip.c_str(), &addr);
            return addr;
        }
        
        static bool s_init;
        int m_socket;
    };
}

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
}
