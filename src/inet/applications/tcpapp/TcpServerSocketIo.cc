//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpServerSocketIo.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpServerSocketIo);

void TcpServerSocketIo::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        trafficSink.reference(gate("trafficOut"), true);
}

void TcpServerSocketIo::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method("acceptSocket");
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn"))
        socket->send(check_and_cast<Packet *>(message));
    else if (message == readDelayTimer) {
        if (socket->isOpen())
            socket->read(par("readSize"));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpServerSocketIo::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    ASSERT(socket == this->socket);
    packet->removeTag<SocketInd>();
    trafficSink.pushPacket(packet);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpServerSocketIo::socketEstablished(TcpSocket *socket)
{
    ASSERT(socket == this->socket);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpServerSocketIo::sendOrScheduleReadCommandIfNeeded()
{
    if (!socket->getAutoRead() && socket->isOpen()) {
        simtime_t delay = par("readDelay");
        if (delay >= SIMTIME_ZERO) {
            if (readDelayTimer == nullptr) {
                readDelayTimer = new cMessage("readDelayTimer");
                readDelayTimer->setContextPointer(this);
            }
            scheduleAfter(delay, readDelayTimer);
        }
        else {
            // send read message to TCP
            socket->read(par("readSize"));
        }
    }
}

cGate *TcpServerSocketIo::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("trafficIn")) {
        if (type == typeid(IPassivePacketSink))
            return gate;
    }
    else if (gate->isName("socketIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && socketInd->getSocketId() == socket->getSocketId())
                return gate;
            auto packetServiceTag = dynamic_cast<const PacketServiceTag *>(arguments);
            if (packetServiceTag != nullptr && packetServiceTag->getProtocol() == &Protocol::tcp)
                return gate;
        }
    }
    return nullptr;
}

} // namespace inet

