#include "Windows.h"
#include "Tlhelp32.h"
#include "psapi.h"
#include "client.hpp"


void Client::UpdateSubProcesses()
{
    std::string vanitysearch_exe = "VanitySearch.exe";
    this->subprocesses = FindSubProcesses(this->pid);

    for (uint32_t pid : subprocesses)
    {
        if (!vanitysearch_exe.compare(FindProcessName(pid)))
        {
            this->vanitysearch_pid = pid;
            delete this->vanitysearch_process;
            this->vanitysearch_process = new Vanity(this->vanitysearch_pid);
        }
    }  
}

std::string Client::FindProcessName(const uint32_t& pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        return "";
    }

    TCHAR processName[MAX_PATH] = TEXT("<unknown>");

    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
        GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR));
    }

    CloseHandle(hProcess);

    // TCHAR* -> std::string
    std::wstring w_processName(processName);
    std::string processName_str(w_processName.begin(), w_processName.end());

    return processName_str;
}

void FindSubProcessesRecursive(const uint32_t& parent_pid, std::set<uint32_t>& pids) // seperate recursive function so that end result can be a vector
{
    // ONLY USED IN Client::FindSubProcesses
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ParentProcessID == parent_pid) {
                if (pids.insert(pe.th32ProcessID).second) {
                    // recursively find child processes
                    FindSubProcessesRecursive(pe.th32ProcessID, pids);
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

std::vector<uint32_t> Client::FindSubProcesses(const uint32_t& parent_pid) // returns { 0 } if fails/no subprocesses found
{
    std::set<uint32_t> pids;
    FindSubProcessesRecursive(parent_pid, pids);

    if (pids.empty()) {
        return { 0 };
    }
    else {
        return std::vector<uint32_t>(pids.begin(), pids.end());
    }
}

uint32_t Client::FindPID(const std::string& process_name)
{

    // std::string -> const wchar_t*
    std::wstring wbuf_process_name = std::wstring(process_name.begin(), process_name.end());
    const wchar_t* w_process_name = wbuf_process_name.c_str();

    PROCESSENTRY32 pe;
    int pid = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // snapshot of all processes
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    pe.dwSize = sizeof(PROCESSENTRY32); // requirement for Process32First

    BOOL hResult = Process32First(hSnapshot, &pe);

    while (hResult) {
        if (!wcscmp(w_process_name, pe.szExeFile)) {
            pid = pe.th32ProcessID;
            break;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }

    
    CloseHandle(hSnapshot); // close handle & return pid
    return pid;
}