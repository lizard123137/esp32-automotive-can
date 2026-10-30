#pragma once

#include <ESPAsyncWebServer.h>

void init_api(AsyncWebServer& server, AsyncWebSocket& ws);

// Called from loop(): publishes CAN frames and status to WebSocket clients.
void update_api(bool can_ready);
