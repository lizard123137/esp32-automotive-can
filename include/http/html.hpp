#pragma once

const char* html = R"rawliteral(
    <!DOCTYPE html>
    <html>
        <head>
            <title>ESP32 Automotive CAN</title>
            <script>
                const ws = new WebSocket("ws://" + location.host + "/ws");
                ws.onmessage = (event) => {
                    console.log("Received message from server:", event.data);
                };

                const scheduler_response = await fetch("/api/scheduler");

                if (!scheduler_response.ok) {
                    console.error("Failed to fetch scheduler data:", scheduler_response.statusText);
                }

                const scheduler_data = await scheduler_response.json();
                console.log("Scheduler data:", scheduler_data);
            </script>
        </head>
        <body>
            <h1>ESP32 Automotive CAN</h1>
            <p>Welcome to the ESP32 Automotive CAN web interface!</p>
        </body>
    </html>
)rawliteral";