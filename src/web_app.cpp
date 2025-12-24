#include "web_app.h"
#include "web_html.h"
#include "config.h"

WebAppHandler::WebAppHandler(int port) {
    serverPort = port;
    server = new WebServer(port);
    rfidHandler = nullptr;
    currentOperation = "idle";
    statusMessage = "System ready";
}

void WebAppHandler::initialize(RFIDHandler* rfid) {
    Serial.println("🔄 Initializing web server...");
    rfidHandler = rfid;
    
    Serial.println("🔄 Setting up HTTP routes...");
    server->on("/", [this]() { handleRoot(); });
    Serial.println("✅ Route registered: GET /");
    
    server->on("/read", [this]() { handleReadCard(); });
    Serial.println("✅ Route registered: GET /read");
    
    server->on("/write", [this]() { handleWriteCard(); });
    Serial.println("✅ Route registered: GET /write");
    
    server->on("/format", [this]() { handleFormatCard(); });
    Serial.println("✅ Route registered: GET /format");
    
    server->on("/status", [this]() { handleGetStatus(); });
    Serial.println("✅ Route registered: GET /status");
    
    server->on("/writedata", HTTP_POST, [this]() { handleWriteData(); });
    Serial.println("✅ Route registered: POST /writedata");
    
    server->onNotFound([this]() { handleNotFound(); });
    Serial.println("✅ 404 handler registered");
    
    Serial.println("🔄 Starting HTTP server...");
    server->begin();
    Serial.println("✅ Web server started on port " + String(serverPort));
    Serial.println("🌐 Access the web interface at: http://" + WiFi.localIP().toString());
    Serial.println("🌐 Available endpoints:");
    Serial.println("   - http://" + WiFi.localIP().toString() + "/ (Main interface)");
    Serial.println("   - http://" + WiFi.localIP().toString() + "/read (Read card)");
    Serial.println("   - http://" + WiFi.localIP().toString() + "/format (Format card)");
    Serial.println("   - http://" + WiFi.localIP().toString() + "/status (System status)");
}

void WebAppHandler::handleClients() {
    if (server) {
        server->handleClient();
    }
}

void WebAppHandler::handleRoot() {
    server->send(200, "text/html", generateHTML());
}

void WebAppHandler::handleReadCard() {
    if (!rfidHandler) {
        server->send(500, "application/json", "{\"error\":\"RFID handler not initialized\"}");
        return;
    }
    
    currentOperation = "reading";
    setStatus("Place card near reader...");
    
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        CardInfo cardInfo = rfidHandler->readCard();
        if (cardInfo.isValid) {
            lastCardData = buildCardDataJSON(cardInfo);
            
            setStatus("Card read successfully!");
            currentOperation = "idle";
            server->send(200, "application/json", "{\"success\":true,\"data\":" + lastCardData + "}");
            return;
        }
        delay(100);
    }
    
    setStatus("No card detected within 5 seconds");
    currentOperation = "idle";
    server->send(200, "application/json", "{\"success\":false,\"message\":\"No card detected\"}");
}

void WebAppHandler::handleWriteCard() {
    server->send(200, "text/html", generateHTML());
}

void WebAppHandler::handleWriteData() {
    if (!rfidHandler) {
        server->send(500, "application/json", "{\"error\":\"RFID handler not initialized\"}");
        return;
    }
    
    if (!server->hasArg("plate") || !server->hasArg("vehicle") || !server->hasArg("department")) {
        server->send(400, "application/json", "{\"error\":\"Missing required fields\"}");
        return;
    }
    
    String plate = server->arg("plate");
    String vehicle = server->arg("vehicle");
    String department = server->arg("department");
    
    currentOperation = "writing";
    setStatus("Place card near reader to write data...");
    
    bool success = performWriteOperation(plate, vehicle, department);
    
    String response = success ? 
        "{\"success\":true,\"message\":\"Data written successfully\"}" :
        "{\"success\":false,\"message\":\"Failed to write data\"}";
    
    server->send(200, "application/json", response);
}

void WebAppHandler::handleFormatCard() {
    if (!rfidHandler) {
        server->send(500, "application/json", "{\"error\":\"RFID handler not initialized\"}");
        return;
    }
    
    currentOperation = "formatting";
    setStatus("Place card near reader to format...");
    
    bool success = performFormatOperation();
    
    String response = success ? 
        "{\"success\":true,\"message\":\"Card formatted successfully\"}" :
        "{\"success\":false,\"message\":\"Failed to format card\"}";
    
    server->send(200, "application/json", response);
}

void WebAppHandler::handleGetStatus() {
    server->send(200, "application/json", getStatusJSON());
}

void WebAppHandler::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}

String WebAppHandler::generateHTML() {
    String html = String(WEB_HTML);
    html.replace("%LOCATION%", String(DEVICE_LOCATION));
    html.replace("%SERVER_IP%", WiFi.localIP().toString());
    return html;
}

String WebAppHandler::getStatusJSON() {
    return "{\"operation\":\"" + currentOperation + "\",\"status\":\"" + statusMessage + "\"}";
}

void WebAppHandler::setStatus(String message) {
    statusMessage = message;
    Serial.println("Web Status: " + message);
}

String WebAppHandler::getServerIP() {
    return WiFi.localIP().toString();
}

bool WebAppHandler::isRunning() {
    return server != nullptr;
}

bool WebAppHandler::writeStringToBlock(int blockNumber, String data) {
    if (!rfidHandler) return false;
    
    Serial.println("Writing to block " + String(blockNumber) + ": " + data);
    
    return rfidHandler->writeBlockAsText(blockNumber, data);
}

String WebAppHandler::buildCardDataJSON(CardInfo cardInfo) {
    String json = "{";
    json += "\"uid\":\"" + cardInfo.uid + "\",";
    json += "\"type\":\"" + cardInfo.type + "\",";
    json += "\"plate\":\"" + cardInfo.plate + "\",";
    json += "\"vehicle\":\"" + cardInfo.vehicle + "\",";
    json += "\"department\":\"" + cardInfo.department + "\"";
    json += "}";
    return json;
}

bool WebAppHandler::performWriteOperation(String plate, String vehicle, String department) {
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {
        CardInfo cardInfo = rfidHandler->readCard();
        if (cardInfo.isValid) {
            bool success = true;
            
            if (plate.length() > 0) {
                success &= writeStringToBlock(4, plate);
            }
            if (vehicle.length() > 0) {
                success &= writeStringToBlock(5, vehicle);
            }
            if (department.length() > 0) {
                success &= writeStringToBlock(6, department);
            }
            
            setStatus(success ? "Data written successfully!" : "Failed to write data to card");
            currentOperation = "idle";
            return success;
        }
        delay(100);
    }
    
    setStatus("No card detected within 10 seconds");
    currentOperation = "idle";
    return false;
}

bool WebAppHandler::performFormatOperation() {
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {
        CardInfo cardInfo = rfidHandler->readCard();
        if (cardInfo.isValid) {
            bool success = true;
            
            success &= writeStringToBlock(4, "");
            success &= writeStringToBlock(5, "");
            success &= writeStringToBlock(6, "");
            
            setStatus(success ? "Card formatted successfully!" : "Failed to format card");
            currentOperation = "idle";
            return success;
        }
        delay(100);
    }
    
    setStatus("No card detected within 10 seconds");
    currentOperation = "idle";
    return false;
}
