#include "Zenderer/Network/Socket.hpp"

using namespace zen;
using util::CLog;
using util::LogMode;
using net::CSocket;

bool CSocket::s_init = false;

bool CSocket::InitializeLibrary()
{
    if(s_init) return true;

#ifdef _WIN32
    WSADATA Data;
    if(WSAStartup(MAKEWORD(2, 0), &Data) != 0)
    {
        return (s_init = false);
    }
#endif  // _WIN32
    return (s_init = true);
}

bool CSocket::Init(const string_t& host, const string_t& port)
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

    addrinfo* result = tmp;
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

    freeaddrinfo(tmp);
    return m_socket != -1;
}

bool CSocket::Destroy()
{
    if(m_socket == -1) return false;
#ifdef _WIN32
    closesocket(m_socket);
#else
    close(m_socket);
#endif // _WIN32
    return (m_socket = -1);
}

string_t CSocket::RecvFrom(const size_t size, string_t& address,
                           string_t& port)
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

int CSocket::SendTo(const string_t& addr, const string_t& port,
                    const string_t& data)
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

int CSocket::SendBroadcast(const string_t& message, const string_t& port)
{
    if(m_Type != SocketType::UDP)
    {
        m_Log << m_Log.SetSystem("Network")
              << m_Log.SetMode(LogMode::ZEN_ERROR)
              << "Broadcasting is only for UDP packets." << CLog::endl;
        return -1;
    }

    this->SetSocketOption(SOL_SOCKET, SO_BROADCAST, true);
    int b = this->SendTo("255.255.255.255", port, message);
    this->SetSocketOption(SOL_SOCKET, SO_BROADCAST, false);
    return b;
}

bool CSocket::Ping()
{
    ZEN_ASSERTM(false, "not implemented");
    return false;
}

bool CSocket::SetSocketOption(const int type, const int option,
                              const bool flag)
{
    if(m_socket < 0) return false;
    char set = flag ? 1 : 0;
    return setsockopt(m_socket, type, option, &set, sizeof set) == 0;
}

bool CSocket::SetNonblocking(const bool flag)
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

int CSocket::GetError() const
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif // _WIN32
}

string_t CSocket::GetAddress(sockaddr_in& addr)
{
    // We would like to support Windows XP and above.
#ifdef _WIN32
    return string_t(inet_ntoa(addr.sin_addr));
#else
    char ip[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    return string_t(ip);
#endif // _WIN32
}

in_addr CSocket::GetAddress(const string_t& ip)
{
    in_addr s;

    // We would like to support Windows XP and above.
#ifdef _WIN32
    s.S_un.S_addr = inet_addr(ip.c_str());
#else
    inet_pton(AF_INET, ip.c_str(), &s);
#endif // _WIN32

    return s;
}
