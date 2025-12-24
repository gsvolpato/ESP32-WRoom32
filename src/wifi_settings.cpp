#include "wifi_settings.h"

WiFiHandler::WiFiHandler(const char* wifiSSID, const char* wifiPassword) {
    ssid = wifiSSID;
    password = wifiPassword;
    wifiConnected = false;
    lastConnectionAttempt = 0;
    connectionAttempts = 0;
    
    Serial.println("🔄 WiFi Handler initialized");
    Serial.print("📶 Target SSID: ");
    Serial.println(ssid);
}

bool WiFiHandler::connect() {
    Serial.println("🔄 Starting WiFi connection process...");
    Serial.print("📶 WiFi mode: ");
    Serial.println(WiFi.getMode());
    
    WiFi.mode(WIFI_STA);
    Serial.println("📶 WiFi mode set to STA");
    
    Serial.print("🔗 Attempting to connect to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && connectionAttempts < MAX_CONNECTION_ATTEMPTS) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        if (connectionAttempts % 5 == 0) {
            Serial.print(" [" + String(connectionAttempts) + "/" + String(MAX_CONNECTION_ATTEMPTS) + "] ");
            printWiFiStatus();
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\n✅ WiFi connected successfully!");
        printConnectionDetails();
        return true;
    } else {
        wifiConnected = false;
        Serial.println("\n❌ WiFi connection failed after " + String(MAX_CONNECTION_ATTEMPTS) + " attempts!");
        Serial.print("❌ Final status: ");
        printWiFiStatus();
        return false;
    }
}

bool WiFiHandler::isConnected() {
    bool currentStatus = (WiFi.status() == WL_CONNECTED);
    if (wifiConnected != currentStatus) {
        wifiConnected = currentStatus;
        if (!currentStatus) {
            Serial.println("⚠️ WiFi connection lost!");
        }
    }
    return wifiConnected;
}

void WiFiHandler::checkConnection() {
    if (!isConnected()) {
        unsigned long currentTime = millis();
        if (currentTime - lastConnectionAttempt >= RECONNECT_INTERVAL) {
            Serial.println("🔄 WiFi disconnected. Attempting to reconnect...");
            reconnect();
            lastConnectionAttempt = currentTime;
        }
    }
}

void WiFiHandler::reconnect() {
    Serial.println("🔄 Reconnecting to WiFi...");
    WiFi.disconnect();
    delay(1000);
    connect();
}

void WiFiHandler::disconnect() {
    Serial.println("🔄 Disconnecting from WiFi...");
    WiFi.disconnect(true);
    wifiConnected = false;
    Serial.println("✅ WiFi disconnected");
}

String WiFiHandler::getLocalIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

String WiFiHandler::getGatewayIP() {
    if (isConnected()) {
        return WiFi.gatewayIP().toString();
    }
    return "Not Connected";
}

String WiFiHandler::getSubnetMask() {
    if (isConnected()) {
        return WiFi.subnetMask().toString();
    }
    return "Not Connected";
}

String WiFiHandler::getDNSIP() {
    if (isConnected()) {
        return WiFi.dnsIP().toString();
    }
    return "Not Connected";
}

String WiFiHandler::getMACAddress() {
    return WiFi.macAddress();
}

int WiFiHandler::getSignalStrength() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiHandler::printNetworkInfo() {
    if (!isConnected()) {
        Serial.println("❌ Not connected to WiFi");
        return;
    }
    
    Serial.println("📡 === WiFi Network Information ===");
    Serial.print("📶 SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("🌐 IP Address: ");
    Serial.println(getLocalIP());
    Serial.print("🚪 Gateway: ");
    Serial.println(getGatewayIP());
    Serial.print("🎭 Subnet Mask: ");
    Serial.println(getSubnetMask());
    Serial.print("🌍 DNS Server: ");
    Serial.println(getDNSIP());
    Serial.print("🏷️ MAC Address: ");
    Serial.println(getMACAddress());
    Serial.print("📊 Signal Strength: ");
    Serial.print(getSignalStrength());
    Serial.println(" dBm");
    Serial.print("📈 Channel: ");
    Serial.println(WiFi.channel());
    Serial.println("📡 ================================");
}

void WiFiHandler::scanNetworks() {
    Serial.println("🔍 Scanning for WiFi networks...");
    int networkCount = WiFi.scanNetworks();
    
    if (networkCount == 0) {
        Serial.println("❌ No networks found");
    } else {
        Serial.println("✅ Found " + String(networkCount) + " networks:");
        for (int i = 0; i < networkCount; i++) {
            Serial.print("  ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(" dBm) ");
            Serial.print(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Encrypted");
            if (WiFi.SSID(i) == ssid) {
                Serial.print(" ← Target network");
            }
            Serial.println();
        }
    }
    Serial.println("🔍 Network scan complete");
}

wl_status_t WiFiHandler::getStatus() {
    return WiFi.status();
}

String WiFiHandler::getStatusString() {
    return getStatusString(WiFi.status());
}

String WiFiHandler::getStatusString(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS: return "IDLE";
        case WL_NO_SSID_AVAIL: return "NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED: return "SCAN_COMPLETED";
        case WL_CONNECTED: return "CONNECTED";
        case WL_CONNECT_FAILED: return "CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "CONNECTION_LOST";
        case WL_DISCONNECTED: return "DISCONNECTED";
        default: return "UNKNOWN";
    }
}

void WiFiHandler::printWiFiStatus() {
    wl_status_t status = WiFi.status();
    Serial.print("Status: ");
    Serial.print(getStatusString(status));
    
    switch (status) {
        case WL_NO_SSID_AVAIL:
            Serial.print(" - Check SSID name");
            break;
        case WL_CONNECT_FAILED:
            Serial.print(" - Check password");
            break;
        case WL_CONNECTION_LOST:
            Serial.print(" - Connection unstable");
            break;
        case WL_DISCONNECTED:
            Serial.print(" - Disconnected");
            break;
    }
    Serial.println();
}

void WiFiHandler::printConnectionDetails() {
    Serial.print("✅ IP address: ");
    Serial.println(getLocalIP());
    Serial.print("✅ Gateway: ");
    Serial.println(getGatewayIP());
    Serial.print("✅ Subnet: ");
    Serial.println(getSubnetMask());
    Serial.print("✅ DNS: ");
    Serial.println(getDNSIP());
    Serial.print("✅ MAC address: ");
    Serial.println(getMACAddress());
    Serial.print("✅ Signal strength: ");
    Serial.print(getSignalStrength());
    Serial.println(" dBm");
}
