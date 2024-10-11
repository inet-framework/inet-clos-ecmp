//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.ecmp.FatTreeExample;


import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.Eth1G;
import inet.node.inet.StandardHost;

import inet.networks.base.WiredNetworkBase;


network fattree extends WiredNetworkBase
{
    parameters:
        @statistic[flowDelaySignal](title="flow delay"; source=flowDelaySignal; unit=s; record=histogram,vector,mean; interpolationmode=none);
        @statistic[throughputSignal](title="data throughput"; source=throughputSignal; unit=bps; record=histogram,vector,mean; interpolationmode=none);
        @statistic[packetDelaySignal](title="packet delay"; source=packetDelaySignal; unit=s; record=histogram,vector,mean; interpolationmode=none);
        @statistic[destinationSignal](title="packets destinations"; source=destinationSignal; record=histogram; interpolation=none);
        @statistic[wrongFragmentPkSignal](title="packet fragments received out of order"; source=wrongFragmentPkSignal; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
   		@display("bgb=1070,450");
    submodules:
        //CAPA 0
        switches_l0[4]: EthernetSwitch {            
            @display("p=450,75,r,100");
			gates:
                ethg[];
        }
        
        //CAPA 1
        switches_l1[8]: EthernetSwitch {
            @display("p=250,175,r,100");
            gates:
                ethg[];
        }
        
        //CAPA 2
        switches_l2[8]: EthernetSwitch {
            @display("p=250,250,r,100");
            gates:
                ethg[];
        }
        
        //CAPA HOSTS
        hosts[8]: StandardHost {
            @display("p=250,325,r,100");
            gates:
                ethg[];
        }
    connections:
        //CAPA 0-1
        for i=0..sizeof(switches_l0)-1, if i<2 {
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[0].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[2].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[4].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[6].ethg++;
        }
		for i=0..sizeof(switches_l0)-1, if i>1 {
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[1].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[3].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[5].ethg++;
            switches_l0[i].ethg++ <--> Eth1G <--> switches_l1[7].ethg++;
        }
        
        //CAPA 1-2
        for i=0..sizeof(switches_l1)-1, if i%2==0 {
         	switches_l1[i].ethg++ <--> Eth1G <--> switches_l2[i].ethg++; 
         	switches_l1[i].ethg++ <--> Eth1G <--> switches_l2[i+1].ethg++;     
        }
        for i=0..sizeof(switches_l1)-1, if i%2!=0 {
         	switches_l1[i].ethg++ <--> Eth1G <--> switches_l2[i].ethg++; 
         	switches_l1[i].ethg++ <--> Eth1G <--> switches_l2[i-1].ethg++;     
        }
		
		//CAPA 2-HOSTS
        for i=0..sizeof(switches_l2)-1 {
            switches_l2[i].ethg++ <--> Eth1G <--> hosts[i].ethg++;
        }
}
