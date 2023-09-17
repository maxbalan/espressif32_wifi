typedef struct WifiConfig {
    uint8_t ssid[32];
    uint8_t password[64];
} WifiConfig;

void connectWifi(WifiConfig *wifiConfig);