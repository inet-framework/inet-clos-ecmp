//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/MacForwardingTableConfiguratorECMP.h"
#include "inet/common/TopologyECMP.h"

/*#include "inet/common/MatchableObject.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"
#include "inet/networklayer/arp/ipv4/GlobalArp.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"*/

namespace inet {

Define_Module(MacForwardingTableConfiguratorECMP);

void MacForwardingTableConfiguratorECMP::initialize(int stage)
{
    MacForwardingTableConfigurator::initialize(stage);
}

void MacForwardingTableConfiguratorECMP::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new TopologyECMP();
    TIME(extractTopology(*topology))
    TIME(computeMacForwardingTables());
    printElapsedTime("initialize", initializeStartTime);
}

void MacForwardingTableConfiguratorECMP::computeMacForwardingTables()
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        Node *destinationNode = (Node *)topology->getNode(i);
        if (!isBridgeNode(destinationNode)) {
            for (auto destinationInterface : destinationNode->interfaces) {
                if (!destinationInterface->networkInterface->isLoopback() &&
                    !destinationInterface->networkInterface->getMacAddress().isUnspecified())
                {
                    extendConfiguration(destinationNode, destinationInterface, destinationInterface->networkInterface->getMacAddress());
                }
            }
        }
    }
}

void MacForwardingTableConfiguratorECMP::extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress)
{
    for (int j = 0; j < destinationNode->getNumInLinks(); j++) {
        auto link = (Link *)destinationNode->getLinkIn(j);
        link->setWeight(link->destinationInterface != destinationInterface ? std::numeric_limits<double>::infinity() : 1);
    }
    topology->calculateWeightedSingleShortestPathsTo(destinationNode);
    for (int j = 0; j < topology->getNumNodes(); j++) {
        Node *sourceNode = (Node *)topology->getNode(j);
        if (sourceNode != destinationNode && isBridgeNode(sourceNode) && sourceNode->getNumPaths() != 0) {
            for (int k = 0; k < sourceNode->getNumPaths(); k++) {
                auto kthLink = static_cast<Link*>(sourceNode->getPath(k));
                auto firstInterface = static_cast<Interface *>(kthLink->sourceInterface);
                auto interfaceName = firstInterface->networkInterface->getInterfaceName();
                auto moduleId = sourceNode->module->getSubmodule("macTable")->getId();
                auto it = configurations.find(moduleId);
                if (it == configurations.end()){
                    configurations[moduleId] = new cValueArray();
                    it = configurations.find(moduleId);
                }
                if (findForwardingRule(it->second, macAddress, interfaceName) != nullptr)
                    continue;
                else {
                    auto rule = new cValueMap();
                    rule->set("address", macAddress.str());
                    rule->set("interface", interfaceName);
                    it->second->add(rule);
                }
            }
        }
    }
}

void MacForwardingTableConfiguratorECMP::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    MacForwardingTableConfigurator::receiveSignal(source, signal, object, details);
}

} // namespace inet

