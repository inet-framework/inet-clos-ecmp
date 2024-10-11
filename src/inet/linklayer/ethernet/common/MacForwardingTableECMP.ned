//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.ethernet.common.MacForwardingTable;

simple MacForwardingTableECMP extends MacForwardingTable
{
    parameters:
        @display("i=block/table2;color=blue");
        @class(MacForwardingTableECMP);
}

