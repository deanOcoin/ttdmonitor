#include "vanity.hpp"

uint32_t Vanity::FindSearchThread()
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnapshot, &te32)) {
        CloseHandle(hThreadSnapshot);
        return 0;
    }

    do {
        if (te32.th32OwnerProcessID == this->pid) {
            return te32.th32ThreadID; // First found will be the search thread
        }
    } while (Thread32Next(hThreadSnapshot, &te32));

    CloseHandle(hThreadSnapshot);

    return 0;
}

double Vanity::ReadXmm(const uint32_t& register_index)
{
    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_ALL;

    HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, this->search_thread_id);
    if (!hThread) {
        return -1;
    }

    if (!GetThreadContext(hThread, &context)) {
        CloseHandle(hThread);
        return -1;
    }

    double value = 0.0;
    M128A xmm_value;

    switch (register_index) {
    case 7:
        xmm_value = context.Xmm7;
        break;
    case 9:
        xmm_value = context.Xmm9;
        break;
    default:
        CloseHandle(hThread);
        return -1;
    }

    memcpy(&value, &xmm_value, sizeof(double)); // copy lower 64 bits of the 128 bit register, casting Xmm.Low to a double will not cast the bits properly

    CloseHandle(hThread);
    return value;
}

bool Vanity::ChangeUpdateDelay(const uint32_t& ms)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, this->pid);
    uintptr_t base_address = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->pid);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnap, &moduleEntry))
        {
            do
            {
                if (!_wcsicmp(moduleEntry.szModule, L"VanitySearch.exe"))
                {
                    base_address = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &moduleEntry));
        }
    }
    else { return false; }

    CloseHandle(hSnap);

    const uintptr_t update_time_address = base_address + this->update_time_offset;

    // convert 32 bit ms into a byte array so that it can be placed in process memory
    uint8_t byte_array[sizeof(uint32_t)];
    for (size_t i = 0; i < sizeof(int32_t); ++i)
    {
        byte_array[i] = static_cast<uint8_t>((ms >> (i * 8)) & 0xFF);
    }

    DWORD old_protection = 0;
    size_t bytes_written = 0;

    if (VirtualProtectEx(hProcess, (LPVOID)update_time_address, sizeof(byte_array), PAGE_EXECUTE_READWRITE, &old_protection) == 0 ||
        WriteProcessMemory(hProcess, (LPVOID)update_time_address, byte_array, sizeof(byte_array), &bytes_written) == 0 ||
        VirtualProtectEx(hProcess, (LPVOID)update_time_address, sizeof(byte_array), old_protection, &old_protection) == 0 )
    {
        return false;
    }

    return true;
}