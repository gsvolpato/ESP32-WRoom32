#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "AP Bridge"
#define WIFI_PASSWORD "batatafrita"

// PostgreSQL API Configuration
#define POSTGRES_API_URL "http://192.168.50.250:8000/api/rfid-readings"
#define DATABASE_PUBLIC_URL "postgresql://postgres:knNvxYzqbPdCKXKXWYGiWKrySNjlkEiq@tramway.proxy.rlwy.net:22447/railway"
#define DATABASE_URL "postgresql://postgres:knNvxYzqbPdCKXKXWYGiWKrySNjlkEiq@postgres.railway.internal:5432/railway"

// Device Configuration
#define DEVICE_LOCATION "Vila Mariana"

#endif 