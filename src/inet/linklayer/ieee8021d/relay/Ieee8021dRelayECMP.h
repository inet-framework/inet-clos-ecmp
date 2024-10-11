//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021DRELAYECMP_H
#define __INET_IEEE8021DRELAYECMP_H

#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"

namespace inet {

//
// This module forward frames (~EtherFrame) based on their destination MAC addresses to appropriate interfaces.
// See the NED definition for details.
//
class INET_API Ieee8021dRelayECMP : public Ieee8021dRelay
{
  protected:
    typedef std::pair<unsigned int, MacAddress> FlowKey;
    typedef std::map<FlowKey, unsigned int> FlowCounter;
    FlowCounter flowCounter;
    int counter = 1;

    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

    virtual void updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId) override;

    virtual bool isForwardingInterface(NetworkInterface *networkInterface) const override;

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    //@}
};

} // namespace inet

#endif

