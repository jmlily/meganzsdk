/**
 * @file ActionPacketStreamProcessor.h
 * @brief Stream processor for ActionPackets to handle large packets without fully loading into memory
 *
 * (c) 2025 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGA SDK.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#pragma once

#include "json.h"
#include "megaclient.h"

namespace mega {

/**
 * @brief Class to handle streaming processing of ActionPackets
 * 
 * This class implements on-the-fly parsing of ActionPackets to avoid loading
 * large packets entirely into memory before processing.
 */
class ActionPacketStreamProcessor
{
public:
    ActionPacketStreamProcessor(MegaClient* client);

    /**
     * @brief Process an ActionPacket in streaming fashion
     * 
     * @param data Pointer to the JSON data of the ActionPacket
     * @return Number of bytes processed
     */
    m_off_t processActionPacketStream(const char* data);

    /**
     * @brief Get the underlying JSONSplitter for direct access if needed
     */
    JSONSplitter* getSplitter();

private:
    MegaClient* mClient;
    JSONSplitter mSplitter;
    bool mProcessingInitialized = false;
};

} // namespace