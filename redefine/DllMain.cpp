#include "D:\jdk-17.0.12\include\jvmti.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEPARATOR '\\'
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

static jvmtiEnv* g_jvmti = nullptr;
static JavaVM* g_jvm = nullptr;
static std::string g_output_dir = "./dumped_classes/";

bool create_directories(const std::string& path) {
    if (path.empty()) return false;

    std::string current_path;
    size_t pos = 0;

    if (path[0] == '/' || path[0] == '\\') {
        pos = 1;
        current_path = path.substr(0, 1);
    }

    while (pos < path.length()) {
        size_t next_pos = path.find_first_of("/\\", pos);
        if (next_pos == std::string::npos) {
            next_pos = path.length();
        }

        if (next_pos > pos) {
            if (!current_path.empty() && current_path.back() != '/' && current_path.back() != '\\') {
                current_path += PATH_SEPARATOR;
            }
            current_path += path.substr(pos, next_pos - pos);
            mkdir(current_path.c_str(), 0755);
        }

        pos = next_pos + 1;
    }

    return true;
}

std::string class_name_to_path(const std::string& class_name) {
    std::string path = class_name;
    for (char& c : path) {
        if (c == '.') {
            c = PATH_SEPARATOR;
        }
    }

    return path + ".class";
}

void save_class_to_file(const char* class_name, const unsigned char* class_data, int class_data_len) {
    if (!class_name || !class_data || class_data_len <= 0) {
        return;
    }

    std::string filename = g_output_dir + class_name_to_path(std::string(class_name));
    size_t last_separator = filename.find_last_of("/\\");
    if (last_separator != std::string::npos) {
        std::string dir_path = filename.substr(0, last_separator);
        create_directories(dir_path);
    }

    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(class_data), class_data_len);
        file.close();
        std::cout << "[DUMPER] Saved: " << filename << " (" << class_data_len << " bytes)" << std::endl;
    }
    else {
        std::cerr << "[DUMPER] Failed to save: " << filename << std::endl;
    }
}

void JNICALL class_file_load_hook(jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jclass class_being_redefined,
    jobject loader,
    const char* name,
    jobject protection_domain,
    jint class_data_len,
    const unsigned char* class_data,
    jint* new_class_data_len,
    unsigned char** new_class_data) {

    if (name && class_data && class_data_len > 0) {
        std::string class_name_str(name);

         if (class_name_str.find("java/") == 0 || class_name_str.find("sun/") == 0) {
             return; 
        }

        save_class_to_file(name, class_data, class_data_len);
    }
}

void JNICALL class_prepare(jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jclass klass) {

    char* class_signature = nullptr;
    char* generic_ptr = nullptr;
    jvmtiError error = jvmti_env->GetClassSignature(klass, &class_signature, &generic_ptr);

    if (error == JVMTI_ERROR_NONE && class_signature) {
        std::cout << "[DUMPER] Class prepared: " << class_signature << std::endl;

        if (class_signature) {
            jvmti_env->Deallocate((unsigned char*)class_signature);
        }
        if (generic_ptr) {
            jvmti_env->Deallocate((unsigned char*)generic_ptr);
        }
    }
}

void dump_loaded_classes() {
    if (!g_jvmti) return;

    jint class_count;
    jclass* classes;
    jvmtiError error = g_jvmti->GetLoadedClasses(&class_count, &classes);

    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to get loaded classes: " << error << std::endl;
        return;
    }

    std::cout << "[DUMPER] Found " << class_count << " loaded classes" << std::endl;

    for (jint i = 0; i < class_count; i++) {
        char* class_signature = nullptr;
        char* generic_ptr = nullptr;
        error = g_jvmti->GetClassSignature(classes[i], &class_signature, &generic_ptr);

        if (error == JVMTI_ERROR_NONE && class_signature) {
            std::string class_name = class_signature;
            if (class_name.length() > 2 && class_name[0] == 'L' && class_name[class_name.length() - 1] == ';') {
                class_name = class_name.substr(1, class_name.length() - 2);
            }

            std::cout << "[DUMPER] Found loaded class: " << class_name << std::endl;

            if (class_signature) {
                g_jvmti->Deallocate((unsigned char*)class_signature);
            }
            if (generic_ptr) {
                g_jvmti->Deallocate((unsigned char*)generic_ptr);
            }
        }
    }

    g_jvmti->Deallocate((unsigned char*)classes);
}

void JNICALL vm_init(jvmtiEnv* jvmti_env, JNIEnv* jni_env, jthread thread) {
    std::cout << "[DUMPER] VM initialized, dumping already loaded classes..." << std::endl;
    dump_loaded_classes();
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    std::cout << "[DUMPER] Agent loading..." << std::endl;

    g_jvm = vm;

    jint result = vm->GetEnv((void**)&g_jvmti, JVMTI_VERSION_1_2);
    if (result != JNI_OK || g_jvmti == nullptr) {
        std::cerr << "[DUMPER] Failed to get JVMTI environment, result: " << result << std::endl;
        return JNI_ERR;
    }

    if (options && strlen(options) > 0) {
        g_output_dir = std::string(options);
        if (g_output_dir.back() != '/' && g_output_dir.back() != '\\') {
            g_output_dir += PATH_SEPARATOR;
        }
        std::cout << "[DUMPER] Output directory: " << g_output_dir << std::endl;
    }

    create_directories(g_output_dir);
    jvmtiCapabilities capabilities;
    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.can_generate_all_class_hook_events = 1;
    capabilities.can_get_bytecodes = 1;
    jvmtiError error = g_jvmti->AddCapabilities(&capabilities);
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to add capabilities: " << error << std::endl;
        return JNI_ERR;
    }

    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ClassFileLoadHook = class_file_load_hook;
    callbacks.ClassPrepare = class_prepare;
    callbacks.VMInit = vm_init;
    error = g_jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to set callbacks: " << error << std::endl;
        return JNI_ERR;
    }
    error = g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to enable ClassFileLoadHook: " << error << std::endl;
        return JNI_ERR;
    }

    error = g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, nullptr);
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to enable ClassPrepare: " << error << std::endl;
        return JNI_ERR;
    }

    error = g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[DUMPER] Failed to enable VMInit: " << error << std::endl;
        return JNI_ERR;
    }

    std::cout << "[DUMPER] Agent loaded successfully" << std::endl;
    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm) {
    std::cout << "[DUMPER] Agent unloading..." << std::endl;
    std::cout << "[DUMPER] Agent unloaded" << std::endl;
}

extern "C" JNIEXPORT void JNICALL Java_ClassDumper_dumpLoadedClasses(JNIEnv* env, jclass clazz) {
    std::cout << "[DUMPER] Manual dump requested" << std::endl;
    dump_loaded_classes();
}