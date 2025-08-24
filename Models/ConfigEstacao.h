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