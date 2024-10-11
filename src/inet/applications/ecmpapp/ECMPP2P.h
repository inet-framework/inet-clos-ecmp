//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ECMPP2P_H
#define __INET_ECMPP2P_H

#include <map>
#include <vector>
#include <queue>

#include "inet/applications/udpapp/UdpBasicBurst.h"

namespace inet {

class INET_API ECMPP2P : public UdpBasicBurst
{
  protected:
    int mtuSize;
    double loadStress;
    long interfaceBitRate;
    bool flowLevelBalance;

    unsigned int localFragmentId = 0;

    typedef std::map<unsigned int, unsigned int> FlowTracker;
    FlowTracker activeProcessingFlows;

    typedef std::map<unsigned int, double> FlowTimeTracker;
    FlowTimeTracker activeFlowTimes;

    std::queue<Packet*> packetFragmentQueue;

    int numWrongFragment;
    int byteRecievedCount = 0;

    static simsignal_t wrongFragmentPkSignal;
    static simsignal_t throughputSignal;
    static simsignal_t flowDelaySignal;
    static simsignal_t packetDelaySignal;
    static simsignal_t destinationSignal;

  protected:
    virtual void createPacketQueue();
    virtual void processPacket(Packet *msg);
    virtual void generateBurst();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    ECMPP2P() {}
    ~ECMPP2P();
};

} // namespace inet

#endif

