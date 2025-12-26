//==================================================================================
//============================ Configuração de Rede ================================
//==================================================================================
class ConfigEstacao {

  public:
    String url, token, modeloBiruta;

    String ssid[MAX_WIFIS];
    String user[MAX_WIFIS];
    String senha[MAX_WIFIS];

    String apn, userSIM, senhaSIM;

    int qtdRedes = 0; // número de redes carregadas
};


struct BSSIDInfo {
    String bssid;
    String ssid;
    String user;
    String senha;
    int rssi;
    bool hasInternet;
};

// Helper struct and enum for detailed ping results
enum PingStatus {
    PING_SUCCESS,
    PING_TIMEOUT,      // No response (poor signal)
    PING_NO_INTERNET   // Destination unreachable (no internet)
};

struct PingResult {
    PingStatus status;
    float packetLoss;
};
