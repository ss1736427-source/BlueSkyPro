#include "mavlink_session_runtime.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
static void crc(std::uint8_t d, std::uint16_t& c){auto t=static_cast<std::uint8_t>(d^static_cast<std::uint8_t>(c&255));auto t2=static_cast<std::uint8_t>(t^static_cast<std::uint8_t>(t<<4));c=static_cast<std::uint16_t>((c>>8)^(static_cast<std::uint16_t>(t2)<<8)^(static_cast<std::uint16_t>(t2)<<3)^(static_cast<std::uint16_t>(t2)>>4));}
static std::vector<std::uint8_t> hb(std::uint8_t seq,std::uint8_t health=3){
 std::vector<std::uint8_t> f{0xFD,9,0,0,seq,1,1,0,0,0,0,0,0,0,0,0,0,health,0,0,0};
 std::uint16_t c=0xffff; for(std::size_t i=1;i<f.size()-2;++i) crc(f[i],c); crc(50,c);
 f[f.size()-2]=static_cast<std::uint8_t>(c); f[f.size()-1]=static_cast<std::uint8_t>(c>>8); return f;
}
static bluesky::operations::MavlinkSessionIngestResult in(bluesky::operations::MavlinkSessionRuntime& r,const std::string& s,std::uint8_t q,std::int64_t ts){
 return r.ingestRawFrame(s,bluesky::operations::MavlinkDialect::ArduPilot,"UAV-028","ArduPilot:MAVLink2:1:1",ts,hb(q));
}
int main(){
 using namespace bluesky::operations;
 MavlinkSessionRuntime r(3000);
 auto a=in(r,"A",10,1000); assert(a.accepted&&a.new_session&&a.snapshot.link_state==MavlinkLinkState::Healthy);
 auto d=in(r,"A",10,1100); assert(!d.accepted&&d.sequence_result==MavlinkSequenceResult::Duplicate);
 auto g=in(r,"A",13,1200); assert(g.accepted&&g.sequence_result==MavlinkSequenceResult::Gap&&g.snapshot.packets_lost==2);
 auto o=in(r,"A",14,1300); assert(o.accepted&&o.snapshot.link_state==MavlinkLinkState::Healthy);
 assert(r.tick("A",4301).link_state==MavlinkLinkState::Lost);
 auto rec=r.reconnect("A","UAV-028","ArduPilot:MAVLink2:1:1",1,1); assert(rec.link_state==MavlinkLinkState::Recovering&&!rec.sequence_initialized);
 auto rr=in(r,"A",20,5000); assert(rr.accepted&&rr.snapshot.link_state==MavlinkLinkState::Healthy);
 auto b=r.ingestRawFrame("B",MavlinkDialect::PX4,"UAV-029","PX4:MAVLink2:2:1",1000,hb(200)); assert(b.accepted&&b.snapshot.last_sequence==200);
 auto bad=hb(1); bad[12]^=1; auto x=r.ingestRawFrame("X",MavlinkDialect::ArduPilot,"UAV-X","ArduPilot:MAVLink2:1:1",1000,bad); assert(!x.accepted&&!r.snapshot("X").has_value());
 std::cout<<"mavlink_session_runtime_test: PASS\n";
}