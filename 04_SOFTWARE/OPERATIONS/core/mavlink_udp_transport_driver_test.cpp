#include "mavlink_udp_transport_driver.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace bluesky::operations;
int main(){
    MavlinkUdpTransportDriver receiver,sender;
    assert(receiver.open({"127.0.0.1",0})); assert(sender.open({"127.0.0.1",0}));
    const auto re=receiver.localEndpoint(); const auto se=sender.localEndpoint(); assert(re&&re->port); assert(se&&se->port);
    const std::vector<std::uint8_t> frame{0xFD,1,0,0,7,1,1,0,0,0,0,0,0};
    assert(sender.sendTo(*re,frame)); const auto received=receiver.receive(); assert(received&&*received==frame);
    assert(sender.stats().sent_packets==1&&receiver.stats().received_packets==1);
    MavlinkUdpEndpoint source; assert(sender.sendTo(*re,frame)); const auto received2=receiver.receive(&source); assert(received2&&*received2==frame&&source.port==se->port);
    assert(!sender.sendTo({"127.0.0.1",0},frame)&&sender.stats().send_failures==1);
    receiver.close(); assert(!receiver.isOpen()); assert(!receiver.sendTo(*se,frame)&&receiver.stats().send_failures==1);
    std::cout<<"mavlink_udp_transport_driver_test: PASS\n";
}