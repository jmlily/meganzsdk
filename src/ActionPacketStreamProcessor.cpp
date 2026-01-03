/**
 * @file ActionPacketStreamProcessor.cpp
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

#include "mega/ActionPacketStreamProcessor.h"
#include "mega/megaclient.h"

namespace mega {

ActionPacketStreamProcessor::ActionPacketStreamProcessor(MegaClient* client)
    : mClient(client)
{
}

m_off_t ActionPacketStreamProcessor::processActionPacketStream(const char* data)
{
    LOG_debug << "ActionPacketStreamProcessor::processActionPacketStream with " << strlen(data) << " bytes";
    LOG_debug << "Data starts with: " << std::string(data, std::min(strlen(data), size_t(100)));

    // If the splitter has finished processing the previous data, clear it to start fresh
    if (mSplitter.hasFinished() || mSplitter.hasFailed()) {
        LOG_debug << "Resetting JSONSplitter state (finished: " << mSplitter.hasFinished() 
                  << ", failed: " << mSplitter.hasFailed() << ")";
        mSplitter.clear();
    }

    // Check splitter state before processing
    LOG_debug << "JSONSplitter state - Starting: " << mSplitter.isStarting() 
              << ", Finished: " << mSplitter.hasFinished() 
              << ", Failed: " << mSplitter.hasFailed();

    // Set up filters for the JSONSplitter to handle specific paths in action packets
    std::map<std::string, std::function<bool(JSON *)>> filters;
    
    // Callback for processing 'w' elements (webhook URLs)
    filters["{\"w"] = [this](JSON* j) { 
        LOG_debug << "Processing 'w' element";
        std::string url;
        j->storeobject(&url);
        if (this->mClient) {
            this->mClient->scnotifyurl = url;
        }
        return true; 
    };
    
    // Callback for processing 'ir' elements (is not last - spoonfeeding)
    filters["{\"ir"] = [this](JSON* j) { 
        LOG_debug << "Processing 'ir' element";
        if (this->mClient) {
            this->mClient->insca_notlast = (j->getint() == 1);
        }
        return true; 
    };
    

        // Callback for processing 'sn' elements (sequence number)
    filters["{\"sn"] = [this](JSON* j)
    {
        LOG_debug << "Processing 'sn' element";
        if (this->mClient)
        {
            this->mClient->scsn.setScsn(j);
            LOG_debug << "Updated SCSN to: " << this->mClient->scsn.text();
        }
        return true;
    };

    
    // Callback for processing 't' elements (tree/node updates) - this is where large data could be
    filters["{[t"] = [this](JSON* j) { 
        LOG_debug << "Processing 't' element";
        if (this->mClient) {
            this->mClient->readtree(j);
        }
        return true; 
    };
    
    // Callback for processing 'u' elements (user updates)
    filters["{u"] = [this](JSON* j) { 
        LOG_debug << "Processing 'u' element";
        if (this->mClient) {
            this->mClient->readusers(j, true);
        }
        return true; 
    };
    
    // Callback for processing the entire 'a' array (action commands within action packets)
    // This should capture the complete action array [{"a":"ua",...},{"a":"t",...}] when the "a" array completes
    filters["{[a"] = [this](JSON* j) {
        LOG_debug << "Processing complete 'a' array (calling processActionArray)";
        if (this->mClient) {
            LOG_debug << "Client is valid, calling processActionArray";
            // Process the action array using the existing logic from procsc
            this->mClient->processActionArray(j);
            LOG_debug << "processActionArray completed";
        } else {
            LOG_warn << "mClient is NULL, cannot call processActionArray";
        }
        return true;
    };
    
    // Special callbacks to help understand path matching
    filters["{"] = [this](JSON* j) { 
        LOG_debug << "End of object reached " << j->getname();
        return true; 
    };
    
    filters["["] = [this](JSON* j) { 
        LOG_debug << "End of array reached " << j->getname();
        return true; 
    };
    
    // Error callback
    filters["E"] = [this](JSON* j) {
        LOG_err << "Error processing ActionPacket stream: " << j->pos;
        return false;
    };
    
    LOG_debug << "Calling JSONSplitter::processChunk with " << filters.size() << " filters";
    LOG_debug << "Filters defined: " << (filters.count("{a") ? "{a," : "") 
              << (filters.count("{a[") ? "{a[," : "")
              << (filters.count("{a[{a") ? "{a[{a," : "")
              << (filters.count("{t") ? "{t," : "") 
              << (filters.count("{u") ? "{u," : "") 
              << (filters.count("{w") ? "{w," : "") 
              << (filters.count("{ir") ? "{ir," : "") 
              << (filters.count("{sn") ? "{sn," : "") 
              << (filters.count("{") ? "{," : "") 
              << (filters.count("[") ? "[," : "") 
              << (filters.count("E") ? "E" : "");
    
    // Process the JSON data using the splitter
    m_off_t processed = mSplitter.processChunk(&filters, data);
    
    LOG_debug << "JSONSplitter::processChunk returned: " << processed;
    LOG_debug << "JSONSplitter final state - Starting: " << mSplitter.isStarting() 
              << ", Finished: " << mSplitter.hasFinished() 
              << ", Failed: " << mSplitter.hasFailed();
    
    return processed;
}

JSONSplitter* ActionPacketStreamProcessor::getSplitter()
{
    return &mSplitter;
}

} // namespace