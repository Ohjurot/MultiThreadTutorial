#include <Windows.h>
#include <string>
#include <sstream>

struct THREAD_ARGS {
    // Input
    std::wstring message;
    DWORD threadId;

    // Output
    BYTE blob = 0;
};

DWORD WINAPI ThreadMain(THREAD_ARGS* args) {
    WCHAR* threadName = (WCHAR*)L"";
    // Get thread description
    GetThreadDescription(GetCurrentThread(), &threadName);

    // Build message 
    std::wstringstream wss;
    wss << L"Thread ID " << args->threadId << L" (" << threadName << L")";

    // Free name
    if (wcslen(threadName)) {
        LocalFree(threadName);
    }

    // Display message box
    MessageBox(NULL, args->message.c_str(), wss.str().c_str(), MB_OK | MB_ICONINFORMATION);

    // Doing dumb calcs
    args->blob = args->threadId % 255;
    
    return 0;
}

INT WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, PWSTR _In_ cmdArgs, INT _In_ cmdShow) {
    // Create thread args
    THREAD_ARGS args;
    args.message = L"Hello I'm a message for the thread";

    // Create thread
    DWORD threadId = 0;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadMain, &args, CREATE_SUSPENDED, &threadId);
    if (hThread) {
        // Pass on the threads id
        args.threadId = threadId;

        // Set thread name
        SetThreadDescription(hThread, L"MessageBox Thread");

        // Set the core for thread
        UINT threadIndex = 31;
        DWORD_PTR mask = (1ULL << threadIndex);
        SetThreadAffinityMask(hThread, mask);

        // Run the thread
        if (ResumeThread(hThread) == -1) {
            return -1;
        }

        // Wait for the thread
        if (WaitForSingleObject(hThread, 10000) == WAIT_TIMEOUT) {
            // Abort other thred
            TerminateThread(hThread, -1);

            // Display message box
            MessageBox(NULL, L"MessageBox not closed within 10 seconds", L"ERROR", MB_OK | MB_ICONERROR);
        }

        // Thread exit message box
        DWORD exitCode = 0;
        if (GetExitCodeThread(hThread, &exitCode)) {
            std::wstringstream wss;
            wss << L"Thread finished with code: 0x" << std::hex << exitCode << std::dec << std::endl << L"The blob is " << args.blob;
            MessageBox(NULL, wss.str().c_str(), L"FINISH", MB_OK | MB_ICONINFORMATION);
        }

        // Destroy thread
        CloseHandle(hThread);
    }

    return 0;
}
