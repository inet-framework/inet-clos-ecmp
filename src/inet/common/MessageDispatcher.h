//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MESSAGEDISPATCHER_H
#define __INET_MESSAGEDISPATCHER_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

#ifdef INET_WITH_QUEUEING
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#endif // #ifdef INET_WITH_QUEUEING

namespace inet {

/**
 * This class implements the corresponding module. See module documentation for more details.
 */
class INET_API MessageDispatcher :
#ifdef INET_WITH_QUEUEING
    public queueing::PacketProcessorBase, public queueing::IActivePacketSource, public queueing::IPassivePacketSink
#else
    public cSimpleModule
#endif // #ifdef INET_WITH_QUEUEING
{
  public:
    class INET_API Key {
      public:
        int protocolId;
        int servicePrimitive;

      public:
        Key(int protocolId, ServicePrimitive servicePrimitive) : protocolId(protocolId), servicePrimitive(servicePrimitive) {}

        bool operator<(const MessageDispatcher::Key& other) const {
            if (protocolId < other.protocolId)
                return true;
            else if (protocolId > other.protocolId)
                return false;
            else
                return servicePrimitive < other.servicePrimitive;
        }

        friend std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo);
    };

  protected:
    bool forwardServiceRegistration;
    bool forwardProtocolRegistration;
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    std::map<int, int> socketIdToGateIndex;
    std::map<int, int> interfaceIdToGateIndex;
    std::map<Key, int> serviceToGateIndex;
    std::map<Key, int> protocolToGateIndex;
    const Protocol *registeringProtocol = nullptr;
    bool registeringAny = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void arrived(cMessage *message, cGate *gate, const SendOptions& options, simtime_t time) override;
    virtual cGate *handlePacket(Packet *packet, const cGate *inGate);
    virtual cGate *handleMessage(Message *request, cGate *inGate);

    int getGateIndexToConnectedModule(const char *moduleName);

  public:
#ifdef INET_WITH_QUEUEING
    virtual bool supportsPacketSending(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }
    virtual bool supportsPacketPassing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return false; }

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
#endif // #ifdef INET_WITH_QUEUEING
};

std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo) {
    out << "[" << foo.protocolId << ", " << omnetpp::cEnum::get("inet::ServicePrimitive")->getStringFor(foo.servicePrimitive) << "]";
    return out;
}

} // namespace inet

#endif

