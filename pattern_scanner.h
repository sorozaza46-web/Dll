#pragma once
#include <windows.h>
#include <vector>

class PatternScanner {
public:
    // Sınıfın ham byte verisi içinde imza araması yapar (AOB)
    static bool ScanClassBytecode(const unsigned char* class_data, int data_len, const std::vector<unsigned char>& pattern) {
        if (data_len < pattern.size()) return false;

        for (int i = 0; i <= data_len - pattern.size(); i++) {
            bool match = true;
            for (size_t j = 0; j < pattern.size(); j++) {
                if (class_data[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        return false;
    }
};

namespace Signatures {
    // 1. Koordinat Değişim İmzası (X, Y, Z)
    const std::vector<unsigned char> Position = { 0xB4, 0x00, 0x01, 0x24, 0x62, 0x57 }; 

    // 2. Hız / İvme İmzası (motionX, motionY, motionZ)
    const std::vector<unsigned char> Velocity = { 0x38, 0x0F, 0x57, 0x24, 0x66, 0x62 }; 

    // 3. Karakterin Yerde Olma Durumu İmzası (onGround - Düşme/Zıplama kontrolü)
    const std::vector<unsigned char> OnGround = { 0x12, 0x02, 0xB6, 0x00, 0x04, 0x99 };

    // 4. Can ve Açlık Seviyesi İmzası (Health & Food Level paketleri)
    const std::vector<unsigned char> Status = { 0xB6, 0x00, 0x0A, 0x1E, 0xB6, 0x00, 0x0B };
}

