typedef struct WifiConfig {
    char ssid[30];
    char password[30];
} WifiConfig;

void connect(WifiConfig config);