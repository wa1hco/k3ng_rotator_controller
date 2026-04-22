#pragma once
#include <cstdint>
#include <cstring>
#include <fstream>

static constexpr int EEPROM_SIZE = 4096;

class EEPROMClass {
public:
    EEPROMClass() {
        memset(data_, 0xFF, sizeof(data_));
        load();
    }

    uint8_t read(int addr) const {
        return (addr>=0 && addr<EEPROM_SIZE) ? data_[addr] : 0xFF;
    }

    void write(int addr, uint8_t val) {
        if (addr>=0 && addr<EEPROM_SIZE) { data_[addr]=val; save(); }
    }

    void update(int addr, uint8_t val) {
        if (read(addr)!=val) write(addr,val);
    }

    int length() const { return EEPROM_SIZE; }

    template<typename T>
    T &get(int addr, T &t) {
        if (addr+int(sizeof(T))<=EEPROM_SIZE) memcpy(&t, data_+addr, sizeof(T));
        return t;
    }

    template<typename T>
    const T &put(int addr, const T &t) {
        if (addr+int(sizeof(T))<=EEPROM_SIZE) { memcpy(data_+addr, &t, sizeof(T)); save(); }
        return t;
    }

private:
    uint8_t data_[EEPROM_SIZE];

    void load() {
        std::ifstream f("eeprom.bin", std::ios::binary);
        if (f) f.read(reinterpret_cast<char*>(data_), EEPROM_SIZE);
    }
    void save() {
        std::ofstream f("eeprom.bin", std::ios::binary);
        if (f) f.write(reinterpret_cast<const char*>(data_), EEPROM_SIZE);
    }
};

extern EEPROMClass EEPROM;
