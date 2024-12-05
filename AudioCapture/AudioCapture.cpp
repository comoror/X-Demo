// AudioCapture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include "LoopbackCapture.h"

void usage()
{
    std::wcout <<
        L"Usage: AudioCapture <outputfilename>\n"
        L"\n"
        L"  <outputfilename> - The name of the output file to write the captured audio to.\n";
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 2)
    {
        usage();
        return 0;
    }

    PCWSTR outputFile = argv[1];

    CLoopbackCapture loopbackCapture;
    HRESULT hr = loopbackCapture.StartCaptureAsync(outputFile);
    if (FAILED(hr))
    {
        wil::unique_hlocal_string message;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PWSTR)&message, 0, nullptr);
        std::wcout << L"Failed to start capture\n0x" << std::hex << hr << L": " << message.get() << L"\n";
    }
    else
    {
        std::wcout << L"Capturing 10 seconds of audio." << std::endl;
        Sleep(10000);

        loopbackCapture.StopCaptureAsync();

        std::wcout << L"Finished.\n";
    }

    return 0;
}
