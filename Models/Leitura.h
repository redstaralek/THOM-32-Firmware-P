//==================================================================================
//===================== Para [mínimos, máximos, instantâneo] =======================
//==================================================================================
class Leitura {

  public: float     tmp,    tmp2,  uva,   uvb,   uvc, sen,    cos,    bat,   gas,  lx, co2, eCo2, tvoc;
  public: uint32_t  prs,    rdis,  rpot;
  public: uint16_t  hum,    hum2;
  public: short     tmpCpu;
  public: int       hal;

};


//==================================================================================
//========================= Dados acumulados p/ média ==============================
//==================================================================================
struct LeituraTS : Leitura {

  public: uint16_t  qtd,    qtdBat,   qtdUVBruto, qtdLx,    qtdR, qtdCo2, qtdTvoc;
  public: bool      bmeOk,  as7331Ok, vemlOk,     as3935Ok, scdOk;
};


//==================================================================================
//============================== Dados compactados =================================
//==================================================================================
struct LeituraCompact {

  public: String data, mot, dsc;
  public: float dados[20];
  public: bool primeiro;
  
};

RTC_NOINIT_ATTR char  _descricaoResetSoftware[100];
RTC_NOINIT_ATTR char  _leiturasAcumuladas[7500];
// RTC_NOINIT_ATTR LeituraCompact _leiturasAcumuladasCompactadas[50];
