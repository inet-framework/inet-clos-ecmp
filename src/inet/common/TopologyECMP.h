//
// Copyright (C) 1992-2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOPOLOGYECMP_H
#define __INET_TOPOLOGYECMP_H

//#include "inet/common/Topology.h"
#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API TopologyECMP : public inet::NetworkConfiguratorBase::Topology
{
  public:

    explicit TopologyECMP(const char *name = nullptr);
    /**
     * Creates and returns an exact copy of this object.
     * See cObject for more details.
     */
    virtual TopologyECMP *dup() const override { return new TopologyECMP(*this); }

    /**
     * Produces a one-line description of the object's contents.
     * See cObject for more details.
     */
    virtual std::string str() const override;

    /**
     * Serializes the object into an MPI send buffer.
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void parsimPack(cCommBuffer *buffer) const override;

    /**
     * Deserializes the object from an MPI receive buffer
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /**
     * Apply the Dijkstra algorithm to find all shortest paths to the given
     * graph node. The paths found can be extracted via Node's methods.
     * Uses weights in nodes and links.
     */
    void calculateWeightedSingleShortestPathsTo(Node *target) const;
    //@}
};

} // namespace inet

#endif
