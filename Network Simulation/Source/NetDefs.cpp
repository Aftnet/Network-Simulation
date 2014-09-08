#include "NetDefs.h"

const u_short NetDefs::cServerPort = 16678;
const UINT NetDefs::cMagicNo = 2879954678;
const char NetDefs::cMsgConnect = 'C';
const char NetDefs::cMsgConnAck = 'K';
const char NetDefs::cMsgStatCom = 'S';
const char NetDefs::cMsgStillAlive = 'A';

NetDefs::MsgConnectStr::MsgConnectStr():
PacketType(NetDefs::cMsgConnect),
MagicNo(NetDefs::cMagicNo)
{
}

NetDefs::MsgStatComHeaderStr::MsgStatComHeaderStr():
PacketType(NetDefs::cMsgStatCom),
TimeVal(0.0f),
NumEnts(0)
{
}

NetDefs::MsgStillAliveStr::MsgStillAliveStr():
PacketType(NetDefs::cMsgStillAlive),
MagicNo(NetDefs::cMagicNo)
{
}