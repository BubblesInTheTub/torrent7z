/*  #########################################################################  */

#include "StdAfx.h"

#ifdef _WIN32
#include <tlhelp32.h>
#include "t7z.h"

/*  #########################################################################  */

namespace torrent7z
{

/*  #########################################################################  */

struct PEH
{
    union
    {
        DWORD hModule;
        BYTE*DosStub;
        WORD*Signature;
    };
    union
    {
        void*peh;
        IMAGE_NT_HEADERS32*pe32h;
        IMAGE_NT_HEADERS64*pe64h;
    };
};
#define GetDosStubSize(P) (*((DWORD*)((P).hModule+0x3C)))
#define GetPEHeader(P) ((void*)((P).hModule+GetDosStubSize(P)))

/*  #########################################################################  */

HMODULE GetFirstModuleHandle(DWORD dwPID)
{
    HMODULE EAX=0;
    MODULEENTRY32 me32;
    HANDLE hModuleSnap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,dwPID);
    if(hModuleSnap!=INVALID_HANDLE_VALUE)
    {
        me32.dwSize=sizeof(MODULEENTRY32);
        if(Module32First(hModuleSnap,&me32))
        {
            EAX=me32.hModule;
        }
        CloseHandle(hModuleSnap);
    }
    return EAX;
}

/*  #########################################################################  */

PROCESSENTRY32*GetCurrentProcessInfo()
{
    PROCESSENTRY32*EAX=0;
    static PROCESSENTRY32 pe32;
    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(hSnapshot!=INVALID_HANDLE_VALUE)
    {
        pe32.dwSize=sizeof(PROCESSENTRY32);
        if(Process32First(hSnapshot,&pe32))
        {
            do
            {
                if(pe32.th32ProcessID==GetCurrentProcessId())
                {
                    EAX=&pe32;
                }
            }while(!EAX && Process32Next(hSnapshot,&pe32));
        }
        CloseHandle(hSnapshot);
    }
    return EAX;
}

/*  #########################################################################  */

int IsParentGui()
{
    int EAX=0;
    if(buffer)
    {
        PROCESSENTRY32&pe32=*GetCurrentProcessInfo();
        HANDLE hProcess=OpenProcess(STANDARD_RIGHTS_REQUIRED|PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pe32.th32ParentProcessID);
        if(hProcess!=INVALID_HANDLE_VALUE)
        {
            HMODULE ef=GetFirstModuleHandle(pe32.th32ParentProcessID);
            SIZE_T numberOfBytesRead;
            if(ReadProcessMemory(hProcess,(LPCVOID)ef,buffer,4*1024,&numberOfBytesRead))
            {
                PEH pe;
                pe.hModule=*((DWORD*)(&buffer));
                if(pe.Signature[0]==IMAGE_DOS_SIGNATURE)
                {
                    if((GetDosStubSize(pe)+sizeof(IMAGE_NT_HEADERS64))<4*1024)
                    {
                        pe.peh=GetPEHeader(pe);
                        if(pe.pe32h->Signature==IMAGE_NT_SIGNATURE)
                        {
                            if(pe.pe32h->OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR32_MAGIC)
                            {
                                EAX=(pe.pe32h->OptionalHeader.Subsystem==IMAGE_SUBSYSTEM_WINDOWS_GUI);
                            }
                            if(pe.pe64h->OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR64_MAGIC)//not sure if we can even get there on 64bit windows
                            {
                                EAX=(pe.pe64h->OptionalHeader.Subsystem==IMAGE_SUBSYSTEM_WINDOWS_GUI);
                            }
                        }
                    }
                }
            }
            CloseHandle(hProcess);
        }
    }
    return EAX;
}

/*  #########################################################################  */

}
#endif