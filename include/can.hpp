#pragma once

#include <mcp_can.h>

// The purpose of this is to give the entire application access to our MCP_CAN
#define SPI_CS_PIN 3
extern MCP_CAN CAN;