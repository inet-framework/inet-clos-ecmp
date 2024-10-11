//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/MacForwardingTableECMP.h"

#include <map>

namespace inet {

#define MAX_LINE    100

Define_Module(MacForwardingTableECMP);

void MacForwardingTableECMP::initialize(int stage)
{
    MacForwardingTable::initialize(stage);
}

void MacForwardingTableECMP::handleParameterChange(const char *name)
{
    MacForwardingTable::handleParameterChange(name);
}

void MacForwardingTableECMP::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacForwardingTableECMP::handleMessageWhenUp(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacForwardingTableECMP::refreshDisplay() const
{
    MacForwardingTable::refreshDisplay();
}

const char *MacForwardingTableECMP::resolveDirective(char directive) const
{
    return MacForwardingTable::resolveDirective(directive);
}

int MacForwardingTableECMP::getUnicastAddressForwardingInterface(const MacAddress& address, unsigned int vid) const
{
    return MacForwardingTable::getUnicastAddressForwardingInterface(address, vid);
}

void MacForwardingTableECMP::setUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("setUnicastAddressForwardingInterface");
    ASSERT(!address.isMulticast());
    ForwardingTableKey key(vid, address);

    //Default behaviour
    auto it = forwardingTable.find(key);
    if (it == forwardingTable.end())
        forwardingTable[key] = AddressEntry(vid, interfaceId, -1);
    else {
        it->second.interfaceId = interfaceId;
        it->second.insertionTime = SimTime::getMaxTime();
    }

    //Extra ECMP behaviour
    auto range = forwardingTableECMP.equal_range(key);

    bool found = false;

    for (auto it = range.first; it != range.second; ++it) {
        if (it->first.first == vid && it->first.second == address && it->second.interfaceId == interfaceId) {
            it->second.interfaceId = interfaceId;
            it->second.insertionTime = SimTime::getMaxTime();
            found = true;
        }
    }

    if (!found) {
        forwardingTableECMP.emplace(key, AddressEntry(vid, interfaceId, -1));
    }
}

void MacForwardingTableECMP::removeUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    MacForwardingTable::removeUnicastAddressForwardingInterface(interfaceId, address, vid);
}

void MacForwardingTableECMP::learnUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    MacForwardingTable::learnUnicastAddressForwardingInterface(interfaceId, address, vid);
}

std::vector<int> MacForwardingTableECMP::getMulticastAddressForwardingInterfaces(const MacAddress& address, unsigned int vid) const
{
    return MacForwardingTable::getMulticastAddressForwardingInterfaces(address, vid);
}

void MacForwardingTableECMP::addMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    MacForwardingTable::addMulticastAddressForwardingInterface(interfaceId, address, vid);
}

void MacForwardingTableECMP::removeMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    MacForwardingTable::removeMulticastAddressForwardingInterface(interfaceId, address, vid);
}

void MacForwardingTableECMP::removeForwardingInterface(int interfaceId)
{
    MacForwardingTable::removeForwardingInterface(interfaceId);
}

void MacForwardingTableECMP::replaceForwardingInterface(int oldInterfaceId, int newInterfaceId)
{
    MacForwardingTable::replaceForwardingInterface(oldInterfaceId, newInterfaceId);
}

void MacForwardingTableECMP::setAgingTime(simtime_t agingTime)
{
    MacForwardingTable::setAgingTime(agingTime);
}

std::vector<int> MacForwardingTableECMP::getECMPForwardingInterface(const MacAddress& address, unsigned int vid) const
{
    Enter_Method("getECMPForwardingInterface");
        ASSERT(!address.isMulticast());
        ForwardingTableKey key(vid, address);

        auto range = forwardingTableECMP.equal_range(key);

        std::vector<int> interfaces;

        for (auto it = range.first; it != range.second; ++it) {
            interfaces.push_back(it->second.interfaceId);
        }

        return interfaces;
        //return interfaces;
}

} // namespace inet

