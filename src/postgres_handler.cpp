#define DISABLE_ALL_LIBRARY_WARNINGS
#include "postgres_handler.h"
#include "config.h"

PostgresHandler::PostgresHandler(const char* wifiSSID, const char* wifiPassword, const char* apiURL) {
    ssid = wifiSSID;
    password = wifiPassword;
    apiUrl = apiURL;
    dbUrl = DATABASE_PUBLIC_URL;
    wifiConnected = false;
    useDirectConnection = false;
}

bool PostgresHandler::connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

bool PostgresHandler::isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

void PostgresHandler::checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        connectToWiFi();
    }
}

String PostgresHandler::getLocalIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

bool PostgresHandler::parseDatabaseUrl(const char* url, String& host, int& port, String& database, String& user, String& password) {
    // Parse postgresql://user:password@host:port/database
    String urlStr = String(url);
    
    if (!urlStr.startsWith("postgresql://")) {
        return false;
    }
    
    urlStr = urlStr.substring(13); // Remove "postgresql://"
    
    int atPos = urlStr.indexOf('@');
    if (atPos == -1) return false;
    
    String auth = urlStr.substring(0, atPos);
    int colonPos = auth.indexOf(':');
    if (colonPos == -1) return false;
    
    user = auth.substring(0, colonPos);
    password = auth.substring(colonPos + 1);
    
    String hostPortDb = urlStr.substring(atPos + 1);
    int slashPos = hostPortDb.indexOf('/');
    if (slashPos == -1) return false;
    
    database = hostPortDb.substring(slashPos + 1);
    String hostPort = hostPortDb.substring(0, slashPos);
    
    int portPos = hostPort.indexOf(':');
    if (portPos == -1) {
        host = hostPort;
        port = 5432; // Default PostgreSQL port
    } else {
        host = hostPort.substring(0, portPos);
        port = hostPort.substring(portPos + 1).toInt();
    }
    
    return true;
}

String PostgresHandler::escapeString(String str) {
    String escaped = "";
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == '\'') {
            escaped += "''";
        } else if (c == '\\') {
            escaped += "\\\\";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

bool PostgresHandler::connectToDatabase(String host, int port, String database, String user, String password) {
    if (client.connected()) {
        return true;
    }
    
    // Close any existing connection
    if (client) {
        client.stop();
        delay(100);
    }
    
    Serial.print("Connecting to PostgreSQL: ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);
    
    if (!client.connect(host.c_str(), port)) {
        Serial.println("TCP connection failed");
        return false;
    }
    
    Serial.println("TCP connected, starting PostgreSQL handshake...");
    
    // PostgreSQL startup message format:
    // 4 bytes: message length (including these 4 bytes + protocol version + parameters)
    // 4 bytes: protocol version (0x00 0x03 0x00 0x00 = 3.0)
    // Parameters: key\x00value\x00...\x00\x00
    
    String startupMsg = "user\x00";
    startupMsg += user;
    startupMsg += "\x00database\x00";
    startupMsg += database;
    startupMsg += "\x00\x00";
    
    // Total length = 4 (protocol version) + startupMsg length + 4 (length field itself)
    uint32_t len = 4 + startupMsg.length() + 4;
    
    // Write message length (big-endian)
    client.write((uint8_t)(len >> 24));
    client.write((uint8_t)(len >> 16));
    client.write((uint8_t)(len >> 8));
    client.write((uint8_t)(len));
    
    // Write protocol version 3.0 (major=3, minor=0)
    client.write((uint8_t)0x00);
    client.write((uint8_t)0x03);
    client.write((uint8_t)0x00);
    client.write((uint8_t)0x00);
    
    // Write parameters
    client.write((uint8_t*)startupMsg.c_str(), startupMsg.length());
    
    delay(300);
    
    // Read authentication response
    unsigned long startTime = millis();
    bool authenticated = false;
    
    while (millis() - startTime < 5000) {
        if (!client.connected()) {
            Serial.println("Connection lost during handshake");
            break;
        }
        
        if (!client.available()) {
            delay(50);
            continue;
        }
        
        uint8_t msgType = client.read();
        
        if (msgType == 'R') {
            // Authentication request
            uint8_t lenBytes[4];
            if (client.readBytes(lenBytes, 4) == 4) {
                uint32_t authLen = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];
                
                if (authLen > 4) {
                    uint8_t authData[4];
                    client.readBytes(authData, 4);
                    uint32_t authMethod = (authData[0] << 24) | (authData[1] << 16) | (authData[2] << 8) | authData[3];
                    
                    Serial.print("Auth method: ");
                    Serial.println(authMethod);
                    
                    if (authMethod == 0 || authMethod == 3) {
                        // Cleartext password
                        String passwordMsg = password;
                        passwordMsg += "\x00";
                        
                        len = passwordMsg.length() + 5;
                        client.write('p');
                        client.write((uint8_t)(len >> 24));
                        client.write((uint8_t)(len >> 16));
                        client.write((uint8_t)(len >> 8));
                        client.write((uint8_t)(len));
                        client.write((uint8_t*)passwordMsg.c_str(), passwordMsg.length());
                    } else {
                        Serial.print("Unsupported auth method: ");
                        Serial.println(authMethod);
                        client.stop();
                        return false;
                    }
                }
            }
        } else if (msgType == 'Z') {
            // Ready for query
            uint8_t lenBytes[4];
            client.readBytes(lenBytes, 4);
            authenticated = true;
            Serial.println("PostgreSQL authenticated!");
            break;
        } else if (msgType == 'E') {
            // Error
            uint8_t lenBytes[4];
            if (client.readBytes(lenBytes, 4) == 4) {
                uint32_t msgLen = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];
                String errorMsg = "";
                for (uint32_t i = 0; i < msgLen - 4 && i < 200; i++) {
                    char c = client.read();
                    if (c >= 32 && c <= 126) errorMsg += c;
                }
                Serial.print("PostgreSQL error: ");
                Serial.println(errorMsg);
            }
            client.stop();
            return false;
        } else {
            // Skip other messages
            uint8_t lenBytes[4];
            if (client.readBytes(lenBytes, 4) == 4) {
                uint32_t msgLen = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];
                for (uint32_t i = 0; i < msgLen - 4 && i < 1000; i++) {
                    client.read();
                }
            }
        }
    }
    
    if (!authenticated) {
        Serial.println("PostgreSQL authentication timeout");
        client.stop();
    }
    
    return authenticated;
}

bool PostgresHandler::sendViaDirectConnection(String uid, String cardType, String location, String plate, String vehicle, String department) {
    String host, database, dbUser, dbPassword;
    int port;
    
    if (!parseDatabaseUrl(dbUrl, host, port, database, dbUser, dbPassword)) {
        Serial.println("Failed to parse database URL");
        return false;
    }
    
    if (!connectToDatabase(host, port, database, dbUser, dbPassword)) {
        Serial.println("Failed to connect to database");
        return false;
    }
    
    // Build metadata JSON
    StaticJsonDocument<256> metadataDoc;
    if (location.length() > 0) metadataDoc["location"] = location;
    if (plate.length() > 0) metadataDoc["plate"] = plate;
    if (vehicle.length() > 0) metadataDoc["vehicle"] = vehicle;
    if (department.length() > 0) metadataDoc["department"] = department;
    
    String metadataJson;
    serializeJson(metadataDoc, metadataJson);
    
    // Build SQL query
    String sql = "INSERT INTO rfid_readings (card_uid, card_type, reader_name, metadata) VALUES ('";
    sql += escapeString(uid);
    sql += "', '";
    sql += escapeString(cardType.length() > 0 ? cardType : "");
    sql += "', '";
    sql += escapeString(location.length() > 0 ? location : "PN532");
    sql += "', '";
    sql += escapeString(metadataJson);
    sql += "'::jsonb);";
    
    // Send query
    Serial.println("Sending INSERT query...");
    uint32_t len = sql.length() + 5;
    client.write('Q');
    client.write((uint8_t)(len >> 24));
    client.write((uint8_t)(len >> 16));
    client.write((uint8_t)(len >> 8));
    client.write((uint8_t)(len));
    client.write((uint8_t*)sql.c_str(), sql.length());
    
    delay(200);
    
    // Read response
    bool success = false;
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) {
        if (!client.connected()) {
            Serial.println("Connection lost during query");
            break;
        }
        
        if (!client.available()) {
            delay(50);
            continue;
        }
        
        uint8_t msgType = client.read();
        
        if (msgType == 'C') {
            // Command complete
            uint8_t lenBytes[4];
            client.readBytes(lenBytes, 4);
            success = true;
            Serial.println("INSERT successful!");
        } else if (msgType == 'Z') {
            // Ready for query
            uint8_t lenBytes[4];
            client.readBytes(lenBytes, 4);
            if (success) break;
        } else if (msgType == 'E') {
            // Error
            uint8_t lenBytes[4];
            if (client.readBytes(lenBytes, 4) == 4) {
                uint32_t msgLen = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];
                String errorMsg = "";
                for (uint32_t i = 0; i < msgLen - 4 && i < 200; i++) {
                    char c = client.read();
                    if (c >= 32 && c <= 126) errorMsg += c;
                }
                Serial.print("PostgreSQL query error: ");
                Serial.println(errorMsg);
            }
            break;
        } else {
            // Skip other messages
            uint8_t lenBytes[4];
            if (client.readBytes(lenBytes, 4) == 4) {
                uint32_t msgLen = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];
                for (uint32_t i = 0; i < msgLen - 4 && i < 1000; i++) {
                    client.read();
                }
            }
        }
    }
    
    if (!success) {
        Serial.println("Query timeout or failed");
        client.stop();
    }
    
    return success;
}

bool PostgresHandler::sendCardData(String uid, String cardType, String location, 
                                   String plate, String vehicle, String department) {
    if (!isWiFiConnected()) {
        return false;
    }
    
    checkWiFiConnection();
    if (!isWiFiConnected()) {
        return false;
    }
    
    // Try API first (if configured)
    if (apiUrl && strlen(apiUrl) > 0 && !useDirectConnection) {
        HTTPClient http;
        WiFiClientSecure clientSecure;
        
        // HTTPS requires secure client
        if (String(apiUrl).startsWith("https://")) {
            clientSecure.setInsecure(); // Skip certificate validation for Railway
            http.begin(clientSecure, apiUrl);
        } else {
            http.begin(apiUrl);
        }
        
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(3000);
        
        StaticJsonDocument<512> doc;
        doc["card_uid"] = uid;
        doc["card_type"] = cardType.length() > 0 ? cardType : "";
        doc["reader_name"] = location.length() > 0 ? location : "PN532";
        
        JsonObject metadata = doc.createNestedObject("metadata");
        if (location.length() > 0) metadata["location"] = location;
        if (plate.length() > 0) metadata["plate"] = plate;
        if (vehicle.length() > 0) metadata["vehicle"] = vehicle;
        if (department.length() > 0) metadata["department"] = department;
        
        String jsonPayload;
        serializeJson(doc, jsonPayload);
        
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode == 200 || httpResponseCode == 201) {
            http.end();
            return true;
        }
        
        // If API fails, try direct connection once
        if (httpResponseCode == -1 || httpResponseCode == -11) {
            http.end();
            // Only try direct connection if API is completely unreachable
            // Railway requires SSL, so direct connection may not work
            // But we'll try once
            if (!useDirectConnection) {
                useDirectConnection = true;
                bool directResult = sendViaDirectConnection(uid, cardType, location, plate, vehicle, department);
                if (!directResult) {
                    // If direct also fails, go back to trying API next time
                    useDirectConnection = false;
                }
                return directResult;
            }
        }
        
        http.end();
    }
    
    // Use direct PostgreSQL connection if API not configured or already using direct
    if (useDirectConnection || !apiUrl || strlen(apiUrl) == 0) {
        return sendViaDirectConnection(uid, cardType, location, plate, vehicle, department);
    }
    
    return false;
}

