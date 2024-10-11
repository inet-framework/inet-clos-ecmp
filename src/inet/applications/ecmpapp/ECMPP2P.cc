//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/ecmpapp/ECMPP2P.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

EXECUTE_ON_STARTUP(
        cEnum * e = cEnum::find("inet::ChooseDestAddrMode");
        if (!e)
            omnetpp::internal::enums.getInstance()->add(e = new cEnum("inet::ChooseDestAddrMode"));
        e->insert(ECMPP2P::ONCE, "once");
        e->insert(ECMPP2P::PER_BURST, "perBurst");
        e->insert(ECMPP2P::PER_SEND, "perSend");
        );

Define_Module(ECMPP2P);

simsignal_t ECMPP2P::wrongFragmentPkSignal = registerSignal("wrongFragmentPkSignal");
simsignal_t ECMPP2P::throughputSignal = registerSignal("throughputSignal");
simsignal_t ECMPP2P::flowDelaySignal = registerSignal("flowDelaySignal");
simsignal_t ECMPP2P::packetDelaySignal = registerSignal("packetDelaySignal");
simsignal_t ECMPP2P::destinationSignal = registerSignal("destinationSignal");

ECMPP2P::~ECMPP2P()
{
    while(!packetFragmentQueue.empty()){
        cancelAndDelete(packetFragmentQueue.front());;
        packetFragmentQueue.pop();
    }
}

void ECMPP2P::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        counter = 0;
        numSent = 0;
        numReceived = 0;
        numDeleted = 0;
        numWrongFragment = 0;

        delayLimit = par("delayLimit");
        startTime = par("startTime");
        stopTime = par("stopTime");

        if (stopTime >= SIMTIME_ZERO && stopTime <= startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");

        messageLengthPar = &par("messageLength");
        burstDurationPar = &par("burstDuration");
        sleepDurationPar = &par("sleepDuration");
        sendIntervalPar = &par("sendInterval");
        nextSleep = startTime;
        nextBurst = startTime;
        nextPkt = startTime;
        dontFragment = par("dontFragment");

        destAddrRNG = par("destAddrRNG");
        const char *addrModeStr = par("chooseDestAddrMode");
        int addrMode = cEnum::get("inet::ChooseDestAddrMode")->lookup(addrModeStr);
        if (addrMode == -1)
            throw cRuntimeError("Invalid chooseDestAddrMode: '%s'", addrModeStr);
        chooseDestAddrMode = static_cast<ChooseDestAddrMode>(addrMode);

        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numDeleted);
        WATCH(numWrongFragment);

        localPort = par("localPort");
        destPort = par("destPort");

        mtuSize = par("mtuSize");
        loadStress = par("loadStress");
        interfaceBitRate = par("interfaceBitRate");
        flowLevelBalance = par("flowLevelBalance");

        timerNext = new cMessage("ECMPP2PTimer");
    }
}

void ECMPP2P::createPacketQueue()
{
    long msgByteLenght = *messageLengthPar;
    for(int i = msgByteLenght; i > 0; i -= mtuSize){
    char msgName[32];
    sprintf(msgName, "ECMPBasicAppData:%d-%d", counter, localFragmentId);

    Packet *pk = new Packet(msgName);
    const auto& payload = makeShared<ApplicationPacket>();
    if(i <= mtuSize){
        payload->setChunkLength(B(i));
        pk->addPar("fragmentEnd") = true;
    }
    else{
        payload->setChunkLength(B(mtuSize));
        pk->addPar("fragmentEnd") = false;
    }

    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    pk->insertAtBack(payload);
    pk->addPar("sourceId") = getId();
    pk->addPar("msgId") = numSent;
    pk->addPar("fragmentId") = localFragmentId++;
    pk->addPar("flowId") = counter;
    pk->addPar("flowLevelBalance") = flowLevelBalance;
    pk->addPar("creationTime") = simTime().dbl();

    packetFragmentQueue.push(pk);
    }
    counter++;
    localFragmentId = 0;
}

void ECMPP2P::handleMessageWhenUp(cMessage *msg)
{
    UdpBasicBurst::handleMessageWhenUp(msg);
}

void ECMPP2P::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    UdpBasicBurst::socketDataArrived(socket, packet);
}

void ECMPP2P::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    UdpBasicBurst::socketErrorArrived(socket, indication);
}

void ECMPP2P::socketClosed(UdpSocket *socket)
{
    UdpBasicBurst::socketClosed(socket);
}

void ECMPP2P::refreshDisplay() const
{
    UdpBasicBurst::refreshDisplay();
}

void ECMPP2P::processPacket(Packet *pk)
{
    if (pk->getKind() == UDP_I_ERROR) {
        EV_WARN << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId")) {
        // duplicate control
        int moduleId = pk->par("sourceId");
        int msgId = pk->par("msgId");
        int fragmentId = pk->par("fragmentId");
        int flowId = pk->par("flowId");
        auto it = sourceSequence.find(moduleId);
        auto flow = activeProcessingFlows.find(pk->par("flowId"));
        if (flow == activeProcessingFlows.end()){
            if (fragmentId != 0){
                EV_DEBUG << "First fragment not 0: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
                emit(wrongFragmentPkSignal, pk);
                delete pk;
                numWrongFragment++;
                return;
            }
            activeProcessingFlows[flowId] = 0;
        }
        else if(flow->second+1 != fragmentId){
            EV_DEBUG << "Out of order fragment: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
            emit(wrongFragmentPkSignal, pk);
            delete pk;
            numWrongFragment++;
            return;
        }

        if (it != sourceSequence.end()) {
            it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
        flow->second++;
    }

    if (delayLimit > 0) {
        if (simTime() - pk->getTimestamp() > delayLimit) {
            EV_DEBUG << "Old packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
            PacketDropDetails details;
            details.setReason(CONGESTION);
            emit(packetDroppedSignal, pk, &details);
            delete pk;
            numDeleted++;
            return;
        }
    }

    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    bool fragmentEnd = pk->par("fragmentEnd");
    int fragmentId = pk->par("fragmentId");
    if(fragmentEnd){
        auto it = activeProcessingFlows.find(pk->par("flowId"));
        activeProcessingFlows.erase(it);

        double flowSeconds = simTime().dbl() - activeFlowTimes[pk->par("flowId")];
        auto it2 = activeFlowTimes.find(pk->par("flowId"));
        activeFlowTimes.erase(it2);
        emit(flowDelaySignal, flowSeconds);
    }

    double creationTime = pk->par("creationTime");

    if(fragmentId == 0)
        activeFlowTimes[pk->par("flowId")] = creationTime;

    emit(packetDelaySignal, simTime().dbl() - creationTime);

    byteRecievedCount += pk->getByteLength();
    emit(packetReceivedSignal, pk);
    numReceived++;
    delete pk;
}

void ECMPP2P::generateBurst()
{
    simtime_t now = simTime();

    if(packetFragmentQueue.empty()){
        createPacketQueue();
    }

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = *sendIntervalPar;
    if (sendInterval < 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");

    if (activeBurst && nextBurst <= now) { // new burst
        double burstDuration = *burstDurationPar;
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = *sleepDurationPar;

        if (burstDuration == 0.0)
            activeBurst = false;
        else {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    Packet *payload = packetFragmentQueue.front();
    packetFragmentQueue.pop();

    int fragmentId = payload->par("fragmentId");

    if (chooseDestAddrMode == PER_SEND && fragmentId == 0)
            destAddr = chooseDestAddr();

    if (dontFragment)
        payload->addTag<FragmentationReq>()->setDontFragment(true);
    payload->setTimestamp();
    emit(packetSentSignal, payload);
    socket.sendTo(payload, destAddr, destPort);

    int lastPartIP = destAddr.toIpv4().getDByte(3);
    emit(destinationSignal, lastPartIP);

    numSent++;

    bool fragmentEnd = payload->par("fragmentEnd");
    if(!fragmentEnd){
        double seconds = payload->getByteLength() / (loadStress * (interfaceBitRate*1000000000) / 8.0);
        simtime_t convertedTime = SimTime(seconds);
        nextPkt += convertedTime;
    }
    else{
        sendInterval = *sendIntervalPar;
        nextPkt += sendInterval;
        // Next timer
        if (activeBurst && nextPkt >= nextSleep)
            nextPkt = nextBurst;

        if (stopTime >= SIMTIME_ZERO && nextPkt >= stopTime){
            timerNext->setKind(STOP);
            nextPkt = stopTime;
        }
    }
    scheduleAt(nextPkt, timerNext);
}

void ECMPP2P::finish()
{
    auto time = simTime().dbl() - getSimulation()->getWarmupPeriod().dbl();
    emit(throughputSignal, (double)(byteRecievedCount*8.0)/time);
    UdpBasicBurst::finish();
}

void ECMPP2P::handleStartOperation(LifecycleOperation *operation)
{
    UdpBasicBurst::handleStartOperation(operation);
}

void ECMPP2P::handleStopOperation(LifecycleOperation *operation)
{
    UdpBasicBurst::handleStopOperation(operation);
}

void ECMPP2P::handleCrashOperation(LifecycleOperation *operation)
{
    UdpBasicBurst::handleCrashOperation(operation);
}

void ECMPP2P::processStart()
{
    UdpBasicBurst::processStart();
}

void ECMPP2P::processSend()
{
    UdpBasicBurst::processSend();
}

void ECMPP2P::processStop()
{
    UdpBasicBurst::processStop();
}

} // namespace inet

