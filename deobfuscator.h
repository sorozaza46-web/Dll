#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

class Deobfuscator {
public:
    // Java Constant Pool (Sabit Havuzu) içerisindeki gizli UTF-8 verilerini analiz eder
    static void AnalyzeConstantPool(const unsigned char* class_data, int data_len) {
        for (int i = 0; i < data_len - 4; i++) {
            // Java sınıf dosyalarında UTF-8 sabitlerinin başındaki belirteç (Tag: 0x01)
            if (class_data[i] == 0x01) { 
                int length = (class_data[i + 1] << 8) | class_data[i + 2];
                if (length > 0 && (i + 3 + length) < data_len) {
                    std::string potential_string(reinterpret_cast<const char*>(&class_data[i + 3]), length);
                    
                    // Minecraft'ın kalbini oluşturan kritik kelimeleri karartılmış kodun içinde aratıyoruz
                    if (potential_string.find("minecraft") != std::string::npos || 
                        potential_string.find("thePlayer") != std::string::npos ||
                        potential_string.find("C03PacketPlayer") != std::string::npos) {
                        
                        std::cout << "   [!] Eşleşme Bulundu (Gizli Metin): " << potential_string << std::endl;
                    }
                }
            }
        }
    }
};

