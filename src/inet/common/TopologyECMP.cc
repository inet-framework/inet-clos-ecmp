//
// Copyright (C) 1992-2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/TopologyECMP.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <list>
#include <sstream>

#include "inet/common/PatternMatcher.h"
#include "inet/common/stlutils.h"

namespace inet {

Register_Class(TopologyECMP);

TopologyECMP::TopologyECMP(const char *name)
{
}

std::string TopologyECMP::str() const
{
    return Topology::str();
}

void TopologyECMP::parsimPack(cCommBuffer *buffer) const
{
    throw cRuntimeError(this, "parsimPack() not implemented");
}

void TopologyECMP::parsimUnpack(cCommBuffer *buffer)
{
    throw cRuntimeError(this, "parsimUnpack() not implemented");
}

void TopologyECMP::calculateWeightedSingleShortestPathsTo(Node *_target) const
{
    if (!_target)
                throw cRuntimeError(this, "..ShortestPathTo(): target node is nullptr");
            auto target = _target;

            // clean path infos
            for (auto& elem : nodes) {
                elem->dist = INFINITY;
                elem->outPaths.clear();
            }

            target->dist = 0;

            std::list<Node *> q;

            q.push_back(target);

            while (!q.empty()) {
                Node *dest = q.front();
                q.pop_front();

                ASSERT(dest->getWeight() >= 0.0);

                // for each w adjacent to v...
                for (int i = 0; i < dest->getNumInLinks(); i++) {
                    if (!(dest->getLinkIn(i)->isEnabled()))
                        continue;

                    Node *src = dest->getLinkIn(i)->getLinkInRemoteNode();
                    if (!src->isEnabled())
                        continue;

                    double linkWeight = dest->getLinkIn(i)->getWeight();

                    // links with linkWeight == 0 might induce circles
                    ASSERT(linkWeight > 0.0);

                    double newdist = dest->dist + linkWeight;
                    if (dest != target)
                        newdist += dest->getWeight(); // dest is not the target, uses weight of dest node as price of routing (infinity means dest node doesn't route between interfaces)
                    if (newdist != INFINITY && src->dist > newdist) { // it's a valid shorter path from src to target node
                        if (src->dist != INFINITY)
                            q.remove(src); // src is in the queue
                        src->dist = newdist;
                        src->outPaths.clear(); // Clear the previous paths
                        src->outPaths.push_back(dest->inLinks[i]); // Add the new shortest path

                        // insert src node to ordered list
                        auto it = q.begin();
                        for (; it != q.end(); ++it)
                            if ((*it)->dist > newdist)
                                break;

                        q.insert(it, src);
                    }
                    else if (src->dist == newdist && !contains(src->outPaths, dest->inLinks[i])) {
                        src->outPaths.push_back(dest->inLinks[i]); // Add another path with the same distance
                    }
                }
            }
}

} // namespace inet
