#include "theme_state.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>
struct alignas(16) Color { unsigned char bytes[32]; };
using Component=int(*)(const void*);
using ColorCtor=void*(*)(void*,int,int,int,int);
static Component red,green,blue,alpha; static ColorCtor make;
static bool mapped(const void*c,Color&out){int r,g,b;if(!mapBlue(red(c),green(c),blue(c),r,g,b))return false;make(&out,r,g,b,alpha(c));return true;}

using BrushCtor=void*(*)(void*,const void*,int);static BrushCtor brushCtor;
static void* newBrush(void* b,const void*c,int style){Color p;return brushCtor(b,mapped(c,p)?&p:c,style);}
using Gradient=void(*)(void*,double,const void*);static Gradient gradient;
static void gradientColor(void*g,double pos,const void*c){Color p;gradient(g,pos,mapped(c,p)?&p:c);}
using Pair=void(*)(void*,const void*);static Pair penColor;
static void newPenColor(void*p,const void*c){Color q;penColor(p,mapped(c,q)?&q:c);}
using RectFill=void(*)(void*,const void*,const void*);static RectFill rectFill,rectFFill;
static void fillRect(void*p,const void*r,const void*c){Color q;rectFill(p,r,mapped(c,q)?&q:c);}
static void fillRectF(void*p,const void*r,const void*c){Color q;rectFFill(p,r,mapped(c,q)?&q:c);}
using IntFill=void(*)(void*,int,int,int,int,const void*);static IntFill intFill;
static void fillInts(void*p,int x,int y,int w,int h,const void*c){Color q;intFill(p,x,y,w,h,mapped(c,q)?&q:c);}

struct alignas(16) PaintObject{unsigned char data[128];};
using GetColor=void*(*)(const void*,void*);static GetColor getPenColor;
using Copy=void*(*)(void*,const void*);static Copy copyPen,copyBrush;
using DestroyPaint=void(*)(void*);static DestroyPaint destroyPen,destroyBrush;
static Pair setPenObject,setBrushObject,changePenColor,changeBrushColor;
using BrushColor=const void*(*)(const void*);static BrushColor getBrushColor;
static Component brushStyle;
static void newPenObject(void*p,const void*pen){Color c,q;getPenColor(pen,&c);if(!mapped(&c,q)){setPenObject(p,pen);return;}PaintObject v;copyPen(&v,pen);changePenColor(&v,&q);setPenObject(p,&v);destroyPen(&v);}

using GradPtr=const void*(*)(const void*);static GradPtr getGradient;
using Stops=void*(*)(const void*,void*);static Stops getStops,getTransform;
using GradientBrush=void*(*)(void*,const void*);static GradientBrush fromGradient;
using SetStop=void(*)(void*,double,const void*);static SetStop setStop;
using Dealloc=void(*)(void*,size_t,size_t);static Dealloc freeArray;
static Pair setTransform;
static bool mappedBrush(const void*brush,PaintObject&v){
 if(!enabled())return false;
 if(brushStyle(brush)==1){Color q;if(!mapped(getBrushColor(brush),q))return false;copyBrush(&v,brush);changeBrushColor(&v,&q);return true;}
 const void*g=getGradient(brush);if(!g)return false;void*vec=nullptr;getStops(g,&vec);if(!vec)return false;
 int n=*(int*)((char*)vec+4);intptr_t offset=*(intptr_t*)((char*)vec+16);bool changed=false;
 if(n>0&&n<4096){char*data=(char*)vec+offset;for(int i=0;i<n;i++){Color q;if(mapped(data+i*24+8,q)){if(!changed){fromGradient(&v,g);PaintObject t;getTransform(brush,&t);setTransform(&v,&t);changed=true;}setStop((void*)getGradient(&v),*(double*)(data+i*24),&q);}}}
 auto ref=(volatile LONG*)vec;if(*ref!=-1&&InterlockedDecrement(ref)==0)freeArray(vec,24,8);return changed;
}
static void newBrushObject(void*p,const void*brush){PaintObject v;if(!mappedBrush(brush,v)){setBrushObject(p,brush);return;}setBrushObject(p,&v);destroyBrush(&v);}
static RectFill fillBrush,fillBrushF;
static void newFillBrush(void*p,const void*r,const void*b){PaintObject v;if(mappedBrush(b,v)){fillBrush(p,r,&v);destroyBrush(&v);}else fillBrush(p,r,b);}
static void newFillBrushF(void*p,const void*r,const void*b){PaintObject v;if(mappedBrush(b,v)){fillBrushF(p,r,&v);destroyBrush(&v);}else fillBrushF(p,r,b);}


struct Hook {const char*name;void*replacement;void**original;};
static Hook hooks[]={
 {"?fillRect@QPainter@@QEAAXAEBVQRect@@AEBVQBrush@@@Z",(void*)newFillBrush,(void**)&fillBrush},
 {"?fillRect@QPainter@@QEAAXAEBVQRectF@@AEBVQBrush@@@Z",(void*)newFillBrushF,(void**)&fillBrushF},
 {"?setPen@QPainter@@QEAAXAEBVQPen@@@Z",(void*)newPenObject,(void**)&setPenObject},
 {"?setBrush@QPainter@@QEAAXAEBVQBrush@@@Z",(void*)newBrushObject,(void**)&setBrushObject},
 {"?setPen@QPainter@@QEAAXAEBVQColor@@@Z",(void*)newPenColor,(void**)&penColor},
 {"?fillRect@QPainter@@QEAAXAEBVQRect@@AEBVQColor@@@Z",(void*)fillRect,(void**)&rectFill},
 {"?fillRect@QPainter@@QEAAXAEBVQRectF@@AEBVQColor@@@Z",(void*)fillRectF,(void**)&rectFFill},
 {"?fillRect@QPainter@@QEAAXHHHHAEBVQColor@@@Z",(void*)fillInts,(void**)&intFill}
};
static int initializeColors(){auto gui=GetModuleHandleW(L"Qt5Gui.dll");if(!gui)gui=GetModuleHandleW(L"Qt5Gui_conda.dll");if(!gui)return -1;
 red=(Component)GetProcAddress(gui,"?red@QColor@@QEBAHXZ");green=(Component)GetProcAddress(gui,"?green@QColor@@QEBAHXZ");blue=(Component)GetProcAddress(gui,"?blue@QColor@@QEBAHXZ");alpha=(Component)GetProcAddress(gui,"?alpha@QColor@@QEBAHXZ");make=(ColorCtor)GetProcAddress(gui,"??0QColor@@QEAA@HHHH@Z");
 if(!red||!green||!blue||!alpha||!make)return -2;
 
#define GET(var,type,name) var=(type)GetProcAddress(gui,name);if(!var)return -4;
 GET(getPenColor,GetColor,"?color@QPen@@QEBA?AVQColor@@XZ");GET(copyPen,Copy,"??0QPen@@QEAA@AEBV0@@Z");GET(copyBrush,Copy,"??0QBrush@@QEAA@AEBV0@@Z");
 GET(destroyPen,DestroyPaint,"??1QPen@@QEAA@XZ");GET(destroyBrush,DestroyPaint,"??1QBrush@@QEAA@XZ");
 GET(changePenColor,Pair,"?setColor@QPen@@QEAAXAEBVQColor@@@Z");GET(changeBrushColor,Pair,"?setColor@QBrush@@QEAAXAEBVQColor@@@Z");
 GET(getBrushColor,BrushColor,"?color@QBrush@@QEBAAEBVQColor@@XZ");GET(brushStyle,Component,"?style@QBrush@@QEBA?AW4BrushStyle@Qt@@XZ");
 
 GET(getGradient,GradPtr,"?gradient@QBrush@@QEBAPEBVQGradient@@XZ");GET(getStops,Stops,"?stops@QGradient@@QEBA?AV?$QVector@U?$QPair@NVQColor@@@@@@XZ");
 GET(fromGradient,GradientBrush,"??0QBrush@@QEAA@AEBVQGradient@@@Z");GET(setStop,SetStop,"?setColorAt@QGradient@@QEAAXNAEBVQColor@@@Z");
 GET(getTransform,Stops,"?transform@QBrush@@QEBA?AVQTransform@@XZ");GET(setTransform,Pair,"?setTransform@QBrush@@QEAAXAEBVQTransform@@@Z");
 auto core=GetModuleHandleW(L"Qt5Core.dll");if(!core)core=GetModuleHandleW(L"Qt5Core_conda.dll");freeArray=(Dealloc)GetProcAddress(core,"?deallocate@QArrayData@@SAXPEAU1@_K1@Z");if(!freeArray)return -5;
 for(auto&h:hooks){*h.original=(void*)GetProcAddress(gui,h.name);if(!*h.original)return -3;}return 0;
}
static bool ours(void*address){HMODULE owner=nullptr;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&owner))return false;return owner==GetModuleHandleW(L"rose_accent.dll")||owner==GetModuleHandleW(L"white_rose.dll")||owner==GetModuleHandleW(L"full_rose.dll");}

static int patchModule(const wchar_t* name){auto base=(unsigned char*)GetModuleHandleW(name);if(!base)return 0;
 auto dos=(IMAGE_DOS_HEADER*)base;auto nt=(IMAGE_NT_HEADERS64*)(base+dos->e_lfanew);auto dir=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];if(!dir.VirtualAddress)return 0;int count=0;
 for(auto imp=(IMAGE_IMPORT_DESCRIPTOR*)(base+dir.VirtualAddress);imp->Name;imp++){
  if(_stricmp((char*)base+imp->Name,"Qt5Gui.dll")||!imp->OriginalFirstThunk)continue;
  auto names=(IMAGE_THUNK_DATA64*)(base+imp->OriginalFirstThunk);auto slots=(IMAGE_THUNK_DATA64*)(base+imp->FirstThunk);
  for(;names->u1.AddressOfData;names++,slots++){
   if(IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal))continue;
   auto symbol=(IMAGE_IMPORT_BY_NAME*)(base+names->u1.AddressOfData);
   for(auto&h:hooks)if(!strcmp((char*)symbol->Name,h.name)){
    if((void*)slots->u1.Function==h.replacement){count++;break;}
    if((void*)slots->u1.Function!=*h.original&&!ours((void*)slots->u1.Function))continue;
    DWORD old;if(!VirtualProtect(&slots->u1.Function,sizeof(void*),PAGE_READWRITE,&old))continue;
    InterlockedExchangePointer((void*volatile*)&slots->u1.Function,h.replacement);
    DWORD ignored;VirtualProtect(&slots->u1.Function,sizeof(void*),old,&ignored);count++;break;
   }
  }
 }return count;
}
int installAccent(){static int installed=0;if(installed)return installed;int result=initializeColors();if(result<0)return result;installed=patchModule(L"WeComCore.dll")+patchModule(L"FunView.dll")+patchModule(L"Qt5Widgets.dll")+patchModule(L"Qt5Svg.dll");return installed;}
extern "C" __declspec(dllexport) DWORD WINAPI TestAccent(void*){if(initializeColors())return 1;Color a,p;make(&a,45,140,240,180);if(!mapped(&a,p)||red(&p)<=blue(&p)||alpha(&p)!=180)return 2;make(&a,230,50,50,255);if(mapped(&a,p))return 3;make(&a,240,240,240,255);if(mapped(&a,p))return 4;return 0;}

extern "C" __declspec(dllexport) DWORD TestPen(void*painter,void*pen){if(initializeColors())return 1;newPenObject(painter,pen);return 0;}
extern "C" __declspec(dllexport) DWORD TestBrush(void*p,void*b){if(initializeColors())return 1;newBrushObject(p,b);return 0;}
