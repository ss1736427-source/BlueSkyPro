#include "mavlink_udp_transport_driver.hpp"
#include <array>
#include <cstring>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace bluesky::operations {
namespace {
#ifdef _WIN32
constexpr std::uintptr_t invalid_socket=static_cast<std::uintptr_t>(~0ULL);
#else
constexpr int invalid_socket=-1;
#endif
bool resolve_address(const MavlinkUdpEndpoint& e,sockaddr_in& a,bool allow_zero_port){
    if(e.host.empty()||(!allow_zero_port&&e.port==0))return false;
    std::memset(&a,0,sizeof(a)); a.sin_family=AF_INET; a.sin_port=htons(e.port);
#ifdef _WIN32
    return InetPtonA(AF_INET,e.host.c_str(),&a.sin_addr)==1;
#else
    return inet_pton(AF_INET,e.host.c_str(),&a.sin_addr)==1;
#endif
}}
bool MavlinkUdpTransportDriver::initializePlatform(){
#ifdef _WIN32
    WSADATA data{}; return WSAStartup(MAKEWORD(2,2),&data)==0;
#else
    return true;
#endif
}
void MavlinkUdpTransportDriver::shutdownPlatform(){
#ifdef _WIN32
    WSACleanup();
#endif
}
MavlinkUdpTransportDriver::~MavlinkUdpTransportDriver(){close();}
bool MavlinkUdpTransportDriver::open(const MavlinkUdpEndpoint& local){
    close(); if(!initializePlatform())return false;
#ifdef _WIN32
    SOCKET fd=::socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); if(fd==INVALID_SOCKET){shutdownPlatform();return false;} socket_=static_cast<std::uintptr_t>(fd);
#else
    socket_=::socket(AF_INET,SOCK_DGRAM,0); if(socket_<0)return false;
#endif
    const std::string host=local.host.empty()?"127.0.0.1":local.host; sockaddr_in address{};
    if(!resolve_address({host,local.port},address,true)){
#ifdef _WIN32
        ::closesocket(static_cast<SOCKET>(socket_));socket_=invalid_socket;shutdownPlatform();
#else
        ::close(socket_);socket_=invalid_socket;
#endif
        return false;
    }
    if(::bind(
#ifdef _WIN32
        static_cast<SOCKET>(socket_),
#else
        socket_,
#endif
        reinterpret_cast<const sockaddr*>(&address),sizeof(address))!=0){
#ifdef _WIN32
        ::closesocket(static_cast<SOCKET>(socket_));socket_=invalid_socket;shutdownPlatform();
#else
        ::close(socket_);socket_=invalid_socket;
#endif
        return false;
    }
#ifdef _WIN32
    u_long mode=1;if(ioctlsocket(static_cast<SOCKET>(socket_),FIONBIO,&mode)!=0){close();return false;}
#else
    const int flags=fcntl(socket_,F_GETFL,0);if(flags<0||fcntl(socket_,F_SETFL,flags|O_NONBLOCK)!=0){close();return false;}
#endif
    sockaddr_in actual{};
#ifdef _WIN32
    int length=sizeof(actual);if(getsockname(static_cast<SOCKET>(socket_),reinterpret_cast<sockaddr*>(&actual),&length)!=0){close();return false;}
#else
    socklen_t length=sizeof(actual);if(getsockname(socket_,reinterpret_cast<sockaddr*>(&actual),&length)!=0){close();return false;}
#endif
    char buffer[INET_ADDRSTRLEN]{};
#ifdef _WIN32
    InetNtopA(AF_INET,&actual.sin_addr,buffer,sizeof(buffer));
#else
    inet_ntop(AF_INET,&actual.sin_addr,buffer,sizeof(buffer));
#endif
    local_endpoint_=MavlinkUdpEndpoint{buffer,ntohs(actual.sin_port)};initialized_=true;stats_={};return true;
}
void MavlinkUdpTransportDriver::close(){
#ifdef _WIN32
    if(socket_!=invalid_socket){::closesocket(static_cast<SOCKET>(socket_));socket_=invalid_socket;}
#else
    if(socket_!=invalid_socket){::close(socket_);socket_=invalid_socket;}
#endif
    local_endpoint_.reset();if(initialized_)shutdownPlatform();initialized_=false;
}
bool MavlinkUdpTransportDriver::isOpen()const{return socket_!=invalid_socket&&initialized_;}
std::optional<MavlinkUdpEndpoint>MavlinkUdpTransportDriver::localEndpoint()const{return local_endpoint_;}
bool MavlinkUdpTransportDriver::sendTo(const MavlinkUdpEndpoint& remote,const std::vector<std::uint8_t>& frame){
    if(!isOpen()||frame.empty()){++stats_.send_failures;return false;}sockaddr_in address{};
    if(!resolve_address(remote,address,false)){++stats_.send_failures;return false;}
#ifdef _WIN32
    const int sent=::sendto(static_cast<SOCKET>(socket_),reinterpret_cast<const char*>(frame.data()),static_cast<int>(frame.size()),0,reinterpret_cast<const sockaddr*>(&address),sizeof(address));
#else
    const ssize_t sent=::sendto(socket_,frame.data(),frame.size(),0,reinterpret_cast<const sockaddr*>(&address),sizeof(address));
#endif
    if(sent!=static_cast<decltype(sent)>(frame.size())){++stats_.send_failures;return false;}++stats_.sent_packets;return true;
}
std::optional<std::vector<std::uint8_t>>MavlinkUdpTransportDriver::receive(MavlinkUdpEndpoint* source){
    if(!isOpen()){++stats_.receive_failures;return std::nullopt;}
    std::array<std::uint8_t,2048> buffer{};sockaddr_in peer{};
#ifdef _WIN32
    int peer_length=sizeof(peer);const int received=::recvfrom(static_cast<SOCKET>(socket_),reinterpret_cast<char*>(buffer.data()),static_cast<int>(buffer.size()),0,reinterpret_cast<sockaddr*>(&peer),&peer_length);
    if(received==SOCKET_ERROR){const auto error=WSAGetLastError();if(error==WSAEWOULDBLOCK)return std::nullopt;++stats_.receive_failures;return std::nullopt;}
#else
    socklen_t peer_length=sizeof(peer);const ssize_t received=::recvfrom(socket_,buffer.data(),buffer.size(),0,reinterpret_cast<sockaddr*>(&peer),&peer_length);
    if(received<0){if(errno==EAGAIN||errno==EWOULDBLOCK)return std::nullopt;++stats_.receive_failures;return std::nullopt;}
#endif
    if(source){char address_buffer[INET_ADDRSTRLEN]{};
#ifdef _WIN32
        InetNtopA(AF_INET,&peer.sin_addr,address_buffer,sizeof(address_buffer));
#else
        inet_ntop(AF_INET,&peer.sin_addr,address_buffer,sizeof(address_buffer));
#endif
        *source=MavlinkUdpEndpoint{address_buffer,ntohs(peer.sin_port)};}
    ++stats_.received_packets;return std::vector<std::uint8_t>(buffer.begin(),buffer.begin()+received);
}
}