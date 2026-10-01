#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <vector>
#include <winver.h>
std::wstring loadedPath;
uintptr_t module(DWORD pid,const wchar_t* name){
 HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);if(s==INVALID_HANDLE_VALUE)return 0;
 MODULEENTRY32W e{};e.dwSize=sizeof(e);uintptr_t r=0;
 if(Module32FirstW(s,&e))do{if(!_wcsicmp(e.szModule,name)){r=(uintptr_t)e.modBaseAddr;loadedPath=e.szExePath;break;}}while(Module32NextW(s,&e));CloseHandle(s);return r;
}
DWORD call(HANDLE p,uintptr_t fn,void* arg){
 HANDLE t=CreateRemoteThread(p,0,0,(LPTHREAD_START_ROUTINE)fn,arg,0,0);if(!t)throw GetLastError();
 DWORD wait=WaitForSingleObject(t,10000),result=999;if(wait==WAIT_OBJECT_0)GetExitCodeThread(t,&result);CloseHandle(t);
 if(wait!=WAIT_OBJECT_0)throw DWORD(WAIT_TIMEOUT);return result;
}

bool compatible(DWORD pid){HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);if(snap==INVALID_HANDLE_VALUE)return false;MODULEENTRY32W e{};e.dwSize=sizeof(e);bool ok=false;if(Module32FirstW(snap,&e))do{if(!_wcsicmp(e.szModule,L"Qt5Core.dll")){DWORD ignored,n=GetFileVersionInfoSizeW(e.szExePath,&ignored);std::vector<unsigned char>v(n);if(n&&GetFileVersionInfoW(e.szExePath,0,n,v.data())){VS_FIXEDFILEINFO*info;UINT len;if(VerQueryValueW(v.data(),L"\\",(void**)&info,&len))ok=info->dwFileVersionMS==MAKELONG(15,5)&&info->dwFileVersionLS==MAKELONG(0,18);}}}while(Module32NextW(snap,&e));CloseHandle(snap);return ok;}
int wmain(int argc,wchar_t**argv){
 if(argc!=3||(_wcsicmp(argv[2],L"apply")&&_wcsicmp(argv[2],L"restore"))){std::cerr<<"Usage: theme-helper.exe PID apply|restore\n";return 1;}
 DWORD pid=wcstoul(argv[1],0,10);
 HANDLE p=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ,FALSE,pid);
 if(!p){DWORD err=GetLastError();std::cerr<<"OpenProcess error "<<err;return 2;}
 wchar_t exe[32768];DWORD len=32768;
 if(!QueryFullProcessImageNameW(p,0,exe,&len)||_wcsicmp(wcsrchr(exe,L'\\')?wcsrchr(exe,L'\\')+1:exe,L"westlake.exe")){CloseHandle(p);std::cerr<<"Unexpected target";return 3;}
 if(!module(pid,L"Qt5Core.dll")||!module(pid,L"Qt5Widgets.dll")){CloseHandle(p);return 4;}
 if(!compatible(pid)){CloseHandle(p);return 30;}
 for(auto name:{L"full_rose.dll",L"full_rose2.dll",L"rose_accent.dll",L"white_rose.dll",L"qt_style_bridge.dll"})if(module(pid,name)){CloseHandle(p);return 31;}
 wchar_t path[32768];GetModuleFileNameW(0,path,32768);std::wstring dll(path);dll=dll.substr(0,dll.find_last_of(L"\\/")+1)+L"theme_engine.dll";
 HMODULE local=LoadLibraryExW(dll.c_str(),0,DONT_RESOLVE_DLL_REFERENCES);if(!local){CloseHandle(p);return 5;}
 auto entry=GetProcAddress(local,!_wcsicmp(argv[2],L"restore")?"Restore":"Apply");if(!entry){FreeLibrary(local);CloseHandle(p);return 5;}auto offset=(uintptr_t)entry-(uintptr_t)local;FreeLibrary(local);
 try{
  auto base=module(pid,L"theme_engine.dll");if(base&&_wcsicmp(loadedPath.c_str(),dll.c_str())){CloseHandle(p);return 32;}
  if(!base&&!_wcsicmp(argv[2],L"restore")){CloseHandle(p);return 0;}
  if(!base){
   auto load=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");HMODULE owner;
   GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)load,&owner);
   wchar_t ownerPath[32768];GetModuleFileNameW(owner,ownerPath,32768);std::wstring ownerName(ownerPath);ownerName=ownerName.substr(ownerName.find_last_of(L"\\/")+1);
   uintptr_t remote=module(pid,ownerName.c_str());if(!remote)throw DWORD(126);
   size_t bytes=(dll.size()+1)*sizeof(wchar_t);void* buf=VirtualAllocEx(p,0,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!buf)throw GetLastError();
   SIZE_T written;if(!WriteProcessMemory(p,buf,dll.c_str(),bytes,&written)||written!=bytes)throw GetLastError();
   call(p,remote+(uintptr_t)load-(uintptr_t)owner,buf);VirtualFreeEx(p,buf,0,MEM_RELEASE);
   base=module(pid,L"theme_engine.dll");if(!base)throw DWORD(126);
  }
  DWORD result=call(p,base+offset,nullptr);std::cout<<"Qt style request result: "<<result<<"\n";CloseHandle(p);return result;
 }catch(DWORD e){std::cerr<<"Error: "<<e<<"\n";CloseHandle(p);return 6;}
}



