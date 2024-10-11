//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACFORWARDINGTABLEECMP_H
#define __INET_MACFORWARDINGTABLEECMP_H

#include "inet/linklayer/ethernet/common/MacForwardingTable.h"

namespace inet {

class INET_API MacForwardingTableECMP : public MacForwardingTable
{
  protected:
    typedef std::multimap<ForwardingTableKey, AddressEntry> ForwardingTableECMP;

    ForwardingTableECMP forwardingTableECMP;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual const char *resolveDirective(char directive) const override;

  public:
    // IMacForwardingTable
    virtual int getUnicastAddressForwardingInterface(const MacAddress& address, unsigned int vid = 0) const override;
    virtual void setUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void removeUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void learnUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;

    virtual std::vector<int> getMulticastAddressForwardingInterfaces(const MacAddress& address, unsigned int vid = 0) const override;
    virtual void addMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void removeMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;

    virtual void removeForwardingInterface(int interfaceId) override;
    virtual void replaceForwardingInterface(int oldInterfaceId, int newInterfaceId) override;

    // ECMP
    virtual std::vector<int> getECMPForwardingInterface(const MacAddress& address, unsigned int vid = 0) const;

  protected:

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) override;

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { initializeTable(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    //@}
};

} // namespace inet

#endif

