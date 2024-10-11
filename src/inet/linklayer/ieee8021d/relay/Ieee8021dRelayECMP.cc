//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelayECMP.h"
#include "inet/linklayer/ethernet/common/MacForwardingTableECMP.h"
#include "inet/applications/base/ApplicationPacket_m.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"

namespace inet {

Define_Module(Ieee8021dRelayECMP);

void Ieee8021dRelayECMP::initialize(int stage)
{
    Ieee8021dRelay::initialize(stage);
}

void Ieee8021dRelayECMP::handleLowerPacket(Packet *incomingPacket)
{
    numReceivedNetworkFrames++;
    auto protocol = incomingPacket->getTag<PacketProtocolTag>()->getProtocol();
    auto macAddressInd = incomingPacket->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    auto interfaceInd = incomingPacket->getTag<InterfaceInd>();
    int incomingInterfaceId = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(incomingInterfaceId);
    unsigned int vlanId = 0;
    if (auto vlanInd = incomingPacket->findTag<VlanInd>())
        vlanId = vlanInd->getVlanId();
    EV_INFO << "Processing packet from network" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
    //updatePeerAddress(incomingInterface, sourceAddress, vlanId);

    const auto& incomingInterfaceData = incomingInterface->findProtocolData<Ieee8021dInterfaceData>();
    // BPDU Handling
    if ((!incomingInterfaceData || incomingInterfaceData->getRole() != Ieee8021dInterfaceData::DISABLED)
            && (destinationAddress == bridgeAddress
                || in_range(registeredMacAddresses, destinationAddress)
                || incomingInterface->matchesMacAddress(destinationAddress))
            && !destinationAddress.isBroadcast())
    {
        EV_DETAIL << "Deliver to upper layer" << endl;
        sendUp(incomingPacket); // deliver to the STP/RSTP module
    }
    else if (incomingInterfaceData && !incomingInterfaceData->isForwarding()) {
        EV_INFO << "Dropping packet because the incoming interface is currently not forwarding" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << endl;
        numDroppedFrames++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, incomingPacket, &details);
        delete incomingPacket;
    }
    else {
        auto outgoingPacket = incomingPacket->dup();
        outgoingPacket->trim();
        outgoingPacket->clearTags();
        outgoingPacket->addTag<PacketProtocolTag>()->setProtocol(protocol);
        if (auto vlanInd = incomingPacket->findTag<VlanInd>())
            outgoingPacket->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
        if (auto userPriorityInd = incomingPacket->findTag<UserPriorityInd>())
            outgoingPacket->addTag<UserPriorityReq>()->setUserPriority(userPriorityInd->getUserPriority());
        auto& macAddressReq = outgoingPacket->addTag<MacAddressReq>();
        macAddressReq->setSrcAddress(sourceAddress);
        macAddressReq->setDestAddress(destinationAddress);

        // TODO revise next "if"s: 2nd drops all packets for me if not forwarding port; 3rd sends up when dest==STP_MULTICAST_ADDRESS; etc.
        // reordering, merge 1st and 3rd, ...

        if (destinationAddress.isBroadcast())
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
        else if (destinationAddress.isMulticast()) {
            auto outgoingInterfaceIds = macForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress);
            if (outgoingInterfaceIds.size() == 0)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                    auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                    if (outgoingInterfaceId != incomingInterfaceId) {
                        if (isForwardingInterface(outgoingInterface))
                            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                        else {
                            EV_WARN << "Discarding packet because output interface is currently not forwarding" << EV_FIELD(outgoingInterface) << EV_FIELD(outgoingPacket) << endl;
                            numDroppedFrames++;
                            PacketDropDetails details;
                            details.setReason(NO_INTERFACE_FOUND);
                            emit(packetDroppedSignal, outgoingPacket, &details);
                        }
                    }
                    else {
                        EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(NO_INTERFACE_FOUND);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                    }
                }
                delete outgoingPacket;
            }
        }
        else {
            auto flowLevelBalance = incomingPacket->par("flowLevelBalance");
            FlowKey key(incomingPacket->par("flowId"), destinationAddress);
            auto it = flowCounter.find(key);

            if(flowLevelBalance){
                if (it == flowCounter.end())
                    flowCounter[key] = counter++;
            }

            IMacForwardingTable* macForwardingTableRef = macForwardingTable.get();
            MacForwardingTableECMP* macForwardingTableECMP = (MacForwardingTableECMP*)macForwardingTableRef;
            auto outgoingInterfacesIds = macForwardingTableECMP->getECMPForwardingInterface(destinationAddress);

            if (outgoingInterfacesIds.size() == 0)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                int selectedInterface;

                if(flowLevelBalance){
                    selectedInterface = outgoingInterfacesIds[flowCounter[key] % outgoingInterfacesIds.size()];
                    if(incomingPacket->par("fragmentEnd"))
                        flowCounter.erase(it);
                }
                else{
                    msgid_t msgId = incomingPacket->getId();
                    selectedInterface = outgoingInterfacesIds[msgId % outgoingInterfacesIds.size()];
                }

                auto outgoingInterface =  interfaceTable->getInterfaceById(selectedInterface);

                if (selectedInterface != incomingInterfaceId) {
                    if (isForwardingInterface(outgoingInterface))
                        sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                    else {
                        EV_WARN << "Discarding packet because output interface is currently not forwarding" << EV_FIELD(outgoingInterface) << EV_FIELD(outgoingPacket) << endl;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(NO_INTERFACE_FOUND);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                    }
                }
                else {
                    EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                    numDroppedFrames++;
                    PacketDropDetails details;
                    details.setReason(NO_INTERFACE_FOUND);
                    emit(packetDroppedSignal, outgoingPacket, &details);
                }
                delete outgoingPacket;
            }
        }
        delete incomingPacket;
    }
    updateDisplayString();
}

void Ieee8021dRelayECMP::handleUpperPacket(Packet *packet)
{
    Ieee8021dRelay::handleUpperPacket(packet);
}

bool Ieee8021dRelayECMP::isForwardingInterface(NetworkInterface *networkInterface) const
{
    return Ieee8021dRelay::isForwardingInterface(networkInterface);
}

void Ieee8021dRelayECMP::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId)
{
    Ieee8021dRelay::updatePeerAddress(incomingInterface, sourceAddress, vlanId);
}

void Ieee8021dRelayECMP::handleStartOperation(LifecycleOperation *operation)
{
    Ieee8021dRelay::handleStartOperation(operation);
}

void Ieee8021dRelayECMP::handleStopOperation(LifecycleOperation *operation)
{
    Ieee8021dRelay::handleStopOperation(operation);
}

void Ieee8021dRelayECMP::handleCrashOperation(LifecycleOperation *operation)
{
    Ieee8021dRelay::handleCrashOperation(operation);
}

void Ieee8021dRelayECMP::finish()
{
    Ieee8021dRelay::finish();
}

} // namespace inet

