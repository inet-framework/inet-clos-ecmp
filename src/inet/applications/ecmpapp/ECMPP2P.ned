//
// Copyright (C) 2006 Alfonso Ariza
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ecmpapp;

import inet.applications.udpapp.UdpBasicBurst;

simple ECMPP2P extends UdpBasicBurst
{
    parameters:
        @class(ECMPP2P);
        
        int mtuSize = default(1500);
        double loadStress = default(1.0);			//100%
        int interfaceBitRate @unit(Gbps) = default(1Gbps);
        bool flowLevelBalance = default(true);
        
        @signal[wrongFragmentPkSignal](type=inet::Packet);
        @signal[throughputSignal](type=double);
        @signal[flowDelaySignal](type=double);
        @signal[packetDelaySignal](type=double);
        @signal[destinationSignal](type=int);
        @statistic[wrongFragmentPkSignal](title="packet fragments received out of order"; source=wrongFragmentPkSignal; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughputSignal](title="data throughput"; source=throughputSignal; unit=bps; record=histogram,vector; interpolationmode=none);
        @statistic[flowDelaySignal](title="flow delay"; source=flowDelaySignal; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[packetDelaySignal](title="packet delay"; source=packetDelaySignal; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[destinationSignal](title="packets destinations"; source=destinationSignal; record=histogram; interpolation=none);
}

