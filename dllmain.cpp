#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <shlobj.h> // Masaüstü yolunu otomatik bulmak için gerekli
#include <jvmti.h>
#include <jni.h>
#include "deobfuscator.h"
#include "pattern_scanner.h"

std::string g_LogFilePath = "";

// Kullanıcının Masaüstü yolunu dinamik olarak bulan fonksiyon
void InitLogFile() {
    char desktopPath[MAX_PATH];
    // Meşru Windows API'si ile kullanıcının Desktop klasörünü çekiyoruz
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        g_LogFilePath = std::string(desktopPath) + "\\otopilot_veri_log.txt";
        
        // İlk açılışta eski dosya varsa temizleyip başlık atıyoruz
        std::ofstream file(g_LogFilePath, std::ios::trunc);
        if (file.is_open()) {
            file << "=== SONOYUNCU OTOPILOT CANLI VERI LOG SISTEMI ===\n";
            file << "Zaman tüneli ve yakalanan sınıflar aşağıda listelenmektedir.\n";
            file << "================================================\n\n";
            file.close();
        }
    }
}

// Dosyaya veri yazma fonksiyonu
void LogToFile(const std::string& text) {
    if (g_LogFilePath.empty()) return;
    
    std::ofstream file(g_LogFilePath, std::ios::app); // append modu: sonuna ekler
    if (file.is_open()) {
        file << text << "\n";
        file.close();
    }
}

void SetConsoleColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void InitConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);
    SetConsoleTitleA("SonOyuncu Canlı Otopilot Veri Tarayıcı v4.0");
    
    SetConsoleColor(11);
    std::cout << "======================================================================" << std::endl;
    std::cout << "[+] CANLI VERI TARAYICI AKTIF - VERILER MASAUUSTUNE KAYDEDILIYOR" << std::endl;
    std::cout << "======================================================================" << std::endl;
    SetConsoleColor(7);
}

// JVM Sınıf Yükleme Kancası
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
        
        if (c_name.length() <= 12 || c_name.find("/") == std::string::npos) {
            
            // 1. KOORDİNAT KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Position)) {
                SetConsoleColor(10);
                std::cout << "[BULUNDU - KOORDİNAT] -> Sınıf: " << c_name << std::endl;
                LogToFile("[KOORDİNAT] Eşleşen Sınıf: " + c_name);
            }

            // 2. HIZ / IVME KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Velocity)) {
                SetConsoleColor(14);
                std::cout << "[BULUNDU - HIZ/VELOCITY] -> Sınıf: " << c_name << std::endl;
                LogToFile("[HIZ/VELOCITY] Eşleşen Sınıf: " + c_name);
            }

            // 3. YERDE TEMAS KONTROLÜ (ONGROUND)
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::OnGround)) {
                SetConsoleColor(13);
                std::cout << "[BULUNDU - ONGROUND] -> Sınıf: " << c_name << std::endl;
                LogToFile("[ONGROUND] Eşleşen Sınıf: " + c_name);
            }

            // 4. CAN VE AÇLIK DURUMU KONTROLÜ
            if (PatternScanner::ScanClassBytecode(class_data, class_data_len, Signatures::Status)) {
                SetConsoleColor(12);
                std::cout << "[BULUNDU - CAN/AÇLIK] -> Sınıf: " << c_name << std::endl;
                LogToFile("[CAN_ACLIK] Eşleşen Sınıf: " + c_name);
            }
        }
    }
}

// DLL Giriş Noktası
extern "C" JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
    InitConsole();
    InitLogFile(); // Masaüstü kayıt sistemini başlat
    
    jvmtiEnv *jvmti;
    if (vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_0) != JNI_OK) {
        return JNI_ERR;
    }

    jvmtiCapabilities caps = { 0 };
    caps.can_generate_all_class_hook_events = 1;
    jvmti->AddCapabilities(&caps);

    jvmtiEventCallbacks callbacks = { 0 };
    callbacks.ClassFileLoadHook = &UltraClassHook;
    jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));

    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, NULL);
    
    return JNI_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}


