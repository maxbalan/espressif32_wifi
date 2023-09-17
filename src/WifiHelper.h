typedef struct WifiConfig {
    char ssid[30];
    char passowrd[30];
} WifiConfig;

void connect(WifiConfig config);