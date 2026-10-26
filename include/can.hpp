#pragma once

#include <mcp_can.h>

#include <array>
#include <vector>

// The purpose of this is to give the entire application access to our MCP_CAN
#define SPI_CS_PIN 3
extern MCP_CAN CAN;

struct CANFrame {
    uint32_t id;
    uint8_t dlc;
    std::array<uint8_t, 8> data;
};

class CANManager {
public:
    // TODO If this is too slow replace with a circular buffer.
    std::vector<CANFrame> history;

    static constexpr size_t MAX_HISTORY_SIZE = 100;

    void readFrame() {
        if (CAN.checkReceive() != CAN_MSGAVAIL) {
            return;
        }
        
        unsigned long id;
        uint8_t dlc;
        uint8_t data[8];
        
        if (CAN.readMsgBuf(&id, &dlc, data) != CAN_OK) {
            return;
        }

        CANFrame frame;
        frame.id = id;
        frame.dlc = dlc;
        std::copy(data, data + dlc, frame.data.begin());

        if (history.size() >= MAX_HISTORY_SIZE) {
            history.erase(history.begin());
        }
        history.push_back(frame);
    }
};

extern CANManager can_manager;