class EepromUtils{

    public: static void iniciaEEPROM(){ EEPROM.begin(EEPROM_SIZE); }

    public: static void escreveDadoEEPROM(int addr, const String &str){
        
        byte len = str.length();
        EEPROM.write(addr, len);
        for (uint16_t i = 0; i < len; i++)
            EEPROM.write(addr + 1 + i, str[i]); 

    }


    public: static String getDadoEEPROM(int addr) {

        uint16_t strLen = EEPROM.read(addr);
        char data[strLen + 1];
        for (uint16_t i = 0; i < strLen; i++)
            data[i] = EEPROM.read(addr + 1 + i); 
        data[strLen] = '\0';
        EEPROM.commit();
        return String(data);

    }

};