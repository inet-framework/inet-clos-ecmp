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

package inet.examples.ecmp.macDebugTest;


import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.Eth1G;
import inet.node.inet.StandardHost;

import inet.networks.base.WiredNetworkBase;


network samplenetwork extends WiredNetworkBase
{
    @display("bgb=1070.11,449.74");
    submodules:
        //CAPA 0
        switch: EthernetSwitch {
            @display("p=600,125");
            gates:
                ethg[];
        }
        //CAPA HOSTS
        host1: StandardHost {
            @display("p=450,300");
            gates:
                ethg[];
        }
        host2: StandardHost {
            @display("p=750,300");
            gates:
                ethg[];
        }
    connections:
        switch.ethg++ <--> Eth1G <--> host1.ethg++;
        switch.ethg++ <--> Eth1G <--> host2.ethg++;
}
