//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/shortcut/ShortcutMacHeaderSerializer.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/shortcut/ShortcutMacHeader_m.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ShortcutMacHeader, ShortcutMacHeaderSerializer);

void ShortcutMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& header = staticPtrCast<const ShortcutMacHeader>(chunk);
    stream.writeUint16Be(b(header->getChunkLength()).get());
    stream.writeUint16Be(ProtocolGroup::getEthertypeProtocolGroup()->getProtocolNumber(header->getPayloadProtocol()));

    b remainders = header->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ShortcutMacHeader length = %d bit smaller than required %d bits", (int)b(header->getChunkLength()).get(), (int)b(stream.getLength() - startPosition).get());
    uint8_t remainderbits = remainders.get() % 8;
    stream.writeByteRepeatedly('?', B(remainders - b(remainderbits)).get());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ShortcutMacHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ShortcutMacHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadProtocol(ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(stream.readUint16Be()));

    b curLength = stream.getPosition() - startPosition;
    b remainders = headerLength - curLength;
    if (remainders < b(0)) {
        phyHeader->markIncorrect();
    }
    else {
        uint8_t remainderbits = remainders.get() % 8;
        stream.readByteRepeatedly('?', B(remainders - b(remainderbits)).get());
        stream.readBitRepeatedly(false, remainderbits);
    }
    phyHeader->setChunkLength(stream.getPosition() - startPosition);
    return phyHeader;
}

} // namespace physicallayer
} // namespace inet

