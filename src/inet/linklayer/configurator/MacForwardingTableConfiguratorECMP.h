//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACFORWARDINGTABLECONFIGURATORECMP_H
#define __INET_MACFORWARDINGTABLECONFIGURATORECMP_H

#include "inet/linklayer/configurator/MacForwardingTableConfigurator.h"

namespace inet {

class INET_API MacForwardingTableConfiguratorECMP : public MacForwardingTableConfigurator
{
  protected:
    virtual void initialize(int stage) override;

    virtual void computeConfiguration();
    virtual void extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress);
    virtual void computeMacForwardingTables();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

