//==================================================================================
//=============================== Tempos globais ===================================
//==================================================================================

struct Tempos{
  
  //Todos em milissegundos
  public: unsigned volatile long ultimoEvPluv,  ultimoEvGiro, eventoPluv, eventoGiro, cicloInicio,  reset;
  public: unsigned volatile long eventoSleep, ultimoEventoSleep,  tempoDormido;

};