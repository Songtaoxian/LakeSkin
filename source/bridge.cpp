#include "theme_state.h"
#include <string>
#include <sstream>
volatile LONG themeState=0;
struct Arg{const void*data;const char*name;};
using Instance=void*(*)();using Utf8=void*(*)(void*,const char*,int);using Destroy=void(*)(void*);
using Invoke=bool(*)(void*,const char*,int,Arg,Arg,Arg,Arg,Arg,Arg,Arg,Arg,Arg,Arg);
using Getter=void*(*)(const void*,void*);
static HMODULE self;static void*originalStyle=nullptr;static bool captured=false;static volatile LONG busy=0;
int installAccent();int installIcons(HMODULE);
static std::wstring directory(){wchar_t b[32768];GetModuleFileNameW(self,b,32768);std::wstring p(b);return p.substr(0,p.find_last_of(L"\\/")+1);}
static bool readFile(const wchar_t*name,std::string&out){auto p=directory()+name;HANDLE h=CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);if(h==INVALID_HANDLE_VALUE)return false;DWORD n=GetFileSize(h,0),got=0;if(n>1024*1024){CloseHandle(h);return false;}out.assign(n,'\0');BOOL ok=ReadFile(h,out.data(),n,&got,0);CloseHandle(h);return ok&&got==n;}
static DWORD execute(bool restore){
 auto core=GetModuleHandleW(L"Qt5Core.dll"),widgets=GetModuleHandleW(L"Qt5Widgets.dll");
 if(!core){core=GetModuleHandleW(L"Qt5Core_conda.dll");widgets=GetModuleHandleW(L"Qt5Widgets_conda.dll");}if(!core||!widgets)return 10;
 auto instance=(Instance)GetProcAddress(core,"?instance@QCoreApplication@@SAPEAV1@XZ");auto utf8=(Utf8)GetProcAddress(core,"?fromUtf8@QString@@SA?AV1@PEBDH@Z");auto destroy=(Destroy)GetProcAddress(core,"??1QString@@QEAA@XZ");
 auto invoke=(Invoke)GetProcAddress(core,"?invokeMethod@QMetaObject@@SA_NPEAVQObject@@PEBDW4ConnectionType@Qt@@VQGenericArgument@@333333333@Z");auto getter=(Getter)GetProcAddress(widgets,"?styleSheet@QApplication@@QEBA?AVQString@@XZ");if(!instance||!utf8||!destroy||!invoke||!getter)return 11;auto app=instance();if(!app)return 12;
 if(!captured){getter(app,&originalStyle);captured=true;}
 Arg empty{nullptr,nullptr};void*value=nullptr;
 if(restore){InterlockedExchange(&themeState,0);Arg arg{&originalStyle,"QString"};return invoke(app,"setStyleSheet",2,arg,empty,empty,empty,empty,empty,empty,empty,empty,empty)?0:16;}
 std::string cfg,css;if(!readFile(L"theme.cfg",cfg)||!readFile(L"theme.qss",css))return 13;
 unsigned long color=0;int iconFlag=-1;std::istringstream input(cfg);input>>std::hex>>color>>std::dec>>iconFlag;if(input.fail()||color>0xffffff||(iconFlag!=0&&iconFlag!=1))return 14;
 if(installIcons(self)<0)return 21;if(installAccent()<=0)return 20;
 DWORD old=state();InterlockedExchange(&themeState,(LONG)(0x80000000u|(iconFlag?0x40000000u:0)|color));
 utf8(&value,css.data(),(int)css.size());Arg arg{&value,"QString"};bool ok=invoke(app,"setStyleSheet",2,arg,empty,empty,empty,empty,empty,empty,empty,empty,empty);destroy(&value);if(!ok)InterlockedExchange(&themeState,old);return ok?0:16;
}
extern "C" __declspec(dllexport) DWORD WINAPI Apply(void*){if(InterlockedCompareExchange(&busy,1,0))return 22;DWORD r=execute(false);InterlockedExchange(&busy,0);return r;}
extern "C" __declspec(dllexport) DWORD WINAPI Restore(void*){if(InterlockedCompareExchange(&busy,1,0))return 22;DWORD r=execute(true);InterlockedExchange(&busy,0);return r;}
extern "C" __declspec(dllexport) DWORD WINAPI ConfigureTest(void*p){InterlockedExchange(&themeState,(LONG)(uintptr_t)p);return 0;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){self=h;DisableThreadLibraryCalls(h);}return TRUE;}
