#include <windows.h>
#include <iostream>
#include <string>
#include <jvmti.h>
#include <jni.h>
#include "deobfuscator.h"
#include "pattern_scanner.h"

// Konsol metinlerini renklendirmek için yardımcı fonksiyon
void SetConsoleColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Canlı Takip Konsolunu Başlatma
void InitConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);
    SetConsoleTitleA("SonOyuncu Otopilot Deobfuscator & Memory Finder v3.0");
    
    SetConsoleColor(11); // Açık Mavi / Camgöbeği
    std::cout << "======================================================================" << std::endl;
    std::cout << "[+] SONOYUNCU CANLI OTOPİLOT VERİ VE DURUM TARAYICI AKTİF" << std::endl;
    std::cout << "======================================================================" << std::endl;
    SetConsoleColor(7); // Standart Beyaz
}

// Her sınıf JVM'e yüklenirken tetiklenen Kanca (Hook) Fonksiyonu
void JNICALL UltraClassHook(
    jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jclass class_being_redefined,
    jobject loader,
    const char* name,
    jobject protection_domain,
    jint class_data_len,
    const unsigned char* class_data,
    jint* new_class_data_len,
    unsigned char** new_class_data) 
{
    if (name != nullptr) {
        std::string c_name(name);
        
        // Sadece şüpheli kısa/karartılmış paketleri filtrele (Performans kaybını önlemek için)
        if (c_name.length() <= 12 || c_name.find("/") == std::string::npos) {
            
            // 1. KOORDİNAT KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Position)) {
                SetConsoleColor(10); // Yeşil
                std::cout << "[BULUNDU - KOORDİNAT] -> Sınıf: " << c_name << " (Otopilot konum verisi)" << std::endl;
            }

            // 2. HIZ / VELOCITY KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Velocity)) {
                SetConsoleColor(14); // Sarı
                std::cout << "[BULUNDU - HIZ/VELOCITY] -> Sınıf: " << c_name << " (İvme/Hareket verisi)" << std::endl;
            }

            // 3. YERDE TEMAS KONTROLÜ (ONGROUND)
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::OnGround)) {
                SetConsoleColor(13); // Pembe
                std::cout << "[BULUNDU - ONGROUND] -> Sınıf: " << c_name << " (Düşme/Zıplama kontrolü)" << std::endl;
            }

            // 4. CAN VE AÇLIK DURUMU KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Status)) {
                SetConsoleColor(12); // Kırmızı
                std::cout << "[BULUNDU - CAN/AÇLIK] -> Sınıf: " << c_name << " (Sağlık/Yemek durumu)" << std::endl;
            }

            // Sabit havuzundaki çözülebilen gizli metinleri gri renkte bas
            SetConsoleColor(8); // Gri
            Deobfuscator::AnalyzeConstantPool(class_data, class_data_len);
            SetConsoleColor(7); // Rengi normale döndür
        }
    }
}

// DLL Enjektörün ile Oyuna Gönderildiğinde Çalışacak Giriş Noktası (Native Agent)
extern "C" JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
    InitConsole();
    
    jvmtiEnv *jvmti;
    if (vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_0) != JNI_OK) {
        return JNI_ERR;
    }

    // JVM Sınıf yakalama yeteneklerini aktif et
    jvmtiCapabilities caps = { 0 };
    caps.can_generate_all_class_hook_events = 1;
    jvmti->AddCapabilities(&caps);

    // Fonksiyonu JVM olaylarına bağla
    jvmtiEventCallbacks callbacks = { 0 };
    callbacks.ClassFileLoadHook = &UltraClassHook;
    jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));

    // Bildirim modunu aç
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, NULL);
    
    return JNI_OK;
}

// Standart Windows DllMain girişi
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

