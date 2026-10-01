#include "theme_state.h"
#include <map>
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
struct alignas(16) Obj{unsigned char data[128];};
using Int=int(*)(const void*);using Key=long long(*)(const void*);using Destroy=void(*)(void*);
static Int pw,ph,stride;static Key key;static Destroy imageDestroy;
using ToImage=void*(*)(const void*,void*);static ToImage toImage;
using Convert=void*(*)(const void*,void*,int,int);static Convert convert;
using Bits=unsigned char*(*)(void*);static Bits bits;
using FromImage=void*(*)(void*,const void*,int);static FromImage fromImage;
using Ratio=double(*)(const void*);static Ratio ratio;
using SetRatio=void(*)(void*,double);static SetRatio setRatio;
static std::unordered_set<uint64_t>*masks;
static std::map<std::pair<long long,DWORD>,Obj*>*cache;
static bool ready=false;
static CRITICAL_SECTION cacheLock;
struct CacheGuard{CacheGuard(){EnterCriticalSection(&cacheLock);}~CacheGuard(){LeaveCriticalSection(&cacheLock);}};
static std::unordered_set<uint64_t>*pixelAssets,*assetSizes;
static uint64_t maskHash(unsigned char*p,int w,int h,int step){uint64_t v=14695981039346656037ull;v=(v^w)*1099511628211ull;v=(v^h)*1099511628211ull;for(int y=0;y<h;y++)for(int x=0;x<w;x++)v=(v^(p[y*step+x*4+3]>32?1:0))*1099511628211ull;return v;}

static uint64_t pixelHash(unsigned char*p,int w,int h,int step){uint64_t v=14695981039346656037ull;v=(v^w)*1099511628211ull;v=(v^h)*1099511628211ull;for(int y=0;y<h;y++)for(int x=0;x<w*4;x++)v=(v^p[y*step+x])*1099511628211ull;return v;}
static bool pixelPink(unsigned char*c){int r,g,b;if(!c[3]||!mapBlue(c[2],c[1],c[0],r,g,b))return false;c[2]=r;c[1]=g;c[0]=b;return true;}

static const void* icon(const void*pix){
 if(!ready||!iconsEnabled())return pix;CacheGuard guard;int w=pw(pix),h=ph(pix);if(w<1||h<1||w>4096||h>4096)return pix;bool small=w>=8&&w<=96&&h>=8&&h<=128;if(!small&&!assetSizes->count((uint64_t(w)<<32)|uint32_t(h)))return pix;
 auto id=std::make_pair(key(pix),state());auto found=cache->find(id);if(found!=cache->end())return found->second?(void*)found->second:pix;
 if(cache->size()>=8192)return pix;
 Obj original{},image{};toImage(pix,&original);convert(&original,&image,5,0);imageDestroy(&original);
 unsigned char*p=bits(&image);int step=stride(&image);bool exact=p&&pixelAssets->count(pixelHash(p,w,h,step));if(!p||(!exact&&(!small||!masks->count(maskHash(p,w,h,step))))){imageDestroy(&image);(*cache)[id]=nullptr;return pix;}
 int changed=0;
 for(int y=0;y<h;y++)for(int x=0;x<w;x++){auto c=p+y*step+x*4;int b=c[0],g=c[1],r=c[2];if(exact){if(pixelPink(c))changed++;}else if(c[3]&&b>r+20&&g>r&&b>=g){DWORD rgb=state();c[2]=(rgb>>16)&255;c[1]=(rgb>>8)&255;c[0]=rgb&255;changed++;}}
 Obj*replacement=nullptr;
 if(changed){replacement=new Obj{};fromImage(replacement,&image,0);setRatio(replacement,ratio(pix));}
 imageDestroy(&image);(*cache)[id]=replacement;return replacement?(void*)replacement:pix;
}
using Draw3=void(*)(void*,const void*,const void*);static Draw3 point,pointF,rect;
static void drawPoint(void*p,const void*d,const void*i){point(p,d,icon(i));}
static void drawPointF(void*p,const void*d,const void*i){pointF(p,d,icon(i));}
static void drawRect(void*p,const void*d,const void*i){rect(p,d,icon(i));}
using Draw4=void(*)(void*,const void*,const void*,const void*);static Draw4 rectCrop,rectFCrop,pointCrop,pointFCrop;
static void drawRectCrop(void*p,const void*d,const void*i,const void*s){rectCrop(p,d,icon(i),s);}
static void drawRectFCrop(void*p,const void*d,const void*i,const void*s){rectFCrop(p,d,icon(i),s);}
static void drawPointCrop(void*p,const void*d,const void*i,const void*s){pointCrop(p,d,icon(i),s);}
static void drawPointFCrop(void*p,const void*d,const void*i,const void*s){pointFCrop(p,d,icon(i),s);}
using DrawXY=void(*)(void*,int,int,const void*);static DrawXY xy;
static void drawXY(void*p,int x,int y,const void*i){xy(p,x,y,icon(i));}
using DrawXYWH=void(*)(void*,int,int,int,int,const void*);static DrawXYWH xywh;
static void drawXYWH(void*p,int x,int y,int w,int h,const void*i){xywh(p,x,y,w,h,icon(i));}

static Int iw,ih;static Key imageKey;static Destroy pixDestroy;
static std::map<std::pair<long long,DWORD>,Obj*>*imageCache;
static const void* roseImage(const void*input){if(!ready||!iconsEnabled())return input;CacheGuard guard;int w=iw(input),h=ih(input);if(w<1||h<1||w>4096||h>4096)return input;if((w>96||h>128)&&!assetSizes->count((uint64_t(w)<<32)|uint32_t(h)))return input;if(!imageCache)imageCache=new std::map<std::pair<long long,DWORD>,Obj*>;auto id=std::make_pair(imageKey(input),state());auto it=imageCache->find(id);if(it!=imageCache->end())return it->second?(void*)it->second:input;if(imageCache->size()>2048)return input;Obj pix{};fromImage(&pix,input,0);const void*result=icon(&pix);Obj*changed=nullptr;if(result!=&pix){changed=new Obj{};toImage(result,changed);}pixDestroy(&pix);(*imageCache)[id]=changed;return changed?(void*)changed:input;}
using Image3=void(*)(void*,const void*,const void*);static Image3 imagePoint,imageRect,imageRectF;
static void imageAtPoint(void*p,const void*d,const void*i){imagePoint(p,d,roseImage(i));}
static void imageAtRect(void*p,const void*d,const void*i){imageRect(p,d,roseImage(i));}
static void imageAtRectF(void*p,const void*d,const void*i){imageRectF(p,d,roseImage(i));}
using Image5=void(*)(void*,const void*,const void*,const void*,int);static Image5 imageCrop,imageCropF;
static void croppedImage(void*p,const void*d,const void*i,const void*s,int f){imageCrop(p,d,roseImage(i),s,f);}
static void croppedImageF(void*p,const void*d,const void*i,const void*s,int f){imageCropF(p,d,roseImage(i),s,f);}

static Draw4 tile,tileF;
static void tiled(void*p,const void*d,const void*i,const void*s){tile(p,d,icon(i),s);}
static void tiledF(void*p,const void*d,const void*i,const void*s){tileF(p,d,icon(i),s);}
struct Hook{const char*name;void*replacement;void**original;};
static Hook hooks[]={
 {"?drawTiledPixmap@QPainter@@QEAAXAEBVQRect@@AEBVQPixmap@@AEBVQPoint@@@Z",(void*)tiled,(void**)&tile},
 {"?drawTiledPixmap@QPainter@@QEAAXAEBVQRectF@@AEBVQPixmap@@AEBVQPointF@@@Z",(void*)tiledF,(void**)&tileF},
 {"?drawImage@QPainter@@QEAAXAEBVQPoint@@AEBVQImage@@@Z",(void*)imageAtPoint,(void**)&imagePoint},
 {"?drawImage@QPainter@@QEAAXAEBVQRect@@AEBVQImage@@@Z",(void*)imageAtRect,(void**)&imageRect},
 {"?drawImage@QPainter@@QEAAXAEBVQRectF@@AEBVQImage@@@Z",(void*)imageAtRectF,(void**)&imageRectF},
 {"?drawImage@QPainter@@QEAAXAEBVQRect@@AEBVQImage@@0V?$QFlags@W4ImageConversionFlag@Qt@@@@@Z",(void*)croppedImage,(void**)&imageCrop},
 {"?drawImage@QPainter@@QEAAXAEBVQRectF@@AEBVQImage@@0V?$QFlags@W4ImageConversionFlag@Qt@@@@@Z",(void*)croppedImageF,(void**)&imageCropF},
 {"?drawPixmap@QPainter@@QEAAXAEBVQPoint@@AEBVQPixmap@@@Z",(void*)drawPoint,(void**)&point},
 {"?drawPixmap@QPainter@@QEAAXAEBVQPointF@@AEBVQPixmap@@@Z",(void*)drawPointF,(void**)&pointF},
 {"?drawPixmap@QPainter@@QEAAXAEBVQRect@@AEBVQPixmap@@@Z",(void*)drawRect,(void**)&rect},
 {"?drawPixmap@QPainter@@QEAAXAEBVQRect@@AEBVQPixmap@@0@Z",(void*)drawRectCrop,(void**)&rectCrop},
 {"?drawPixmap@QPainter@@QEAAXAEBVQRectF@@AEBVQPixmap@@0@Z",(void*)drawRectFCrop,(void**)&rectFCrop},
 {"?drawPixmap@QPainter@@QEAAXAEBVQPoint@@AEBVQPixmap@@AEBVQRect@@@Z",(void*)drawPointCrop,(void**)&pointCrop},
 {"?drawPixmap@QPainter@@QEAAXAEBVQPointF@@AEBVQPixmap@@AEBVQRectF@@@Z",(void*)drawPointFCrop,(void**)&pointFCrop},
 {"?drawPixmap@QPainter@@QEAAXHHAEBVQPixmap@@@Z",(void*)drawXY,(void**)&xy},
 {"?drawPixmap@QPainter@@QEAAXHHHHAEBVQPixmap@@@Z",(void*)drawXYWH,(void**)&xywh}
};
static bool ours(void*address){HMODULE owner=nullptr;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&owner))return false;return owner==GetModuleHandleW(L"rose_accent.dll")||owner==GetModuleHandleW(L"white_rose.dll")||owner==GetModuleHandleW(L"full_rose.dll");}

static int patch(const wchar_t*name){auto b=(unsigned char*)GetModuleHandleW(name);if(!b)return 0;auto dos=(IMAGE_DOS_HEADER*)b;auto nt=(IMAGE_NT_HEADERS64*)(b+dos->e_lfanew);auto dir=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];if(!dir.VirtualAddress)return 0;int count=0;
 for(auto imp=(IMAGE_IMPORT_DESCRIPTOR*)(b+dir.VirtualAddress);imp->Name;imp++){if(_stricmp((char*)b+imp->Name,"Qt5Gui.dll")||!imp->OriginalFirstThunk)continue;auto n=(IMAGE_THUNK_DATA64*)(b+imp->OriginalFirstThunk);auto slot=(IMAGE_THUNK_DATA64*)(b+imp->FirstThunk);for(;n->u1.AddressOfData;n++,slot++){if(IMAGE_SNAP_BY_ORDINAL64(n->u1.Ordinal))continue;auto sym=(IMAGE_IMPORT_BY_NAME*)(b+n->u1.AddressOfData);for(auto&h:hooks)if(!strcmp((char*)sym->Name,h.name)){if((void*)slot->u1.Function==h.replacement){count++;break;}if((void*)slot->u1.Function!=*h.original&&!ours((void*)slot->u1.Function))continue;DWORD old;if(!VirtualProtect(&slot->u1.Function,8,PAGE_READWRITE,&old))continue;InterlockedExchangePointer((void*volatile*)&slot->u1.Function,h.replacement);DWORD unused;VirtualProtect(&slot->u1.Function,8,old,&unused);count++;break;}}}return count;}
int installIcons(HMODULE self){if(ready)return 1;auto gui=GetModuleHandleW(L"Qt5Gui.dll");if(!gui)gui=GetModuleHandleW(L"Qt5Gui_conda.dll");if(!gui)return -1;
#define LOAD(var,type,symbol) var=(type)GetProcAddress(gui,symbol);if(!var)return -2;
 LOAD(pw,Int,"?width@QPixmap@@QEBAHXZ");LOAD(ph,Int,"?height@QPixmap@@QEBAHXZ");LOAD(key,Key,"?cacheKey@QPixmap@@QEBA_JXZ");
 LOAD(toImage,ToImage,"?toImage@QPixmap@@QEBA?AVQImage@@XZ");
 LOAD(convert,Convert,"?convertToFormat@QImage@@QEBA?AV1@W4Format@1@V?$QFlags@W4ImageConversionFlag@Qt@@@@@Z");
 LOAD(bits,Bits,"?bits@QImage@@QEAAPEAEXZ");LOAD(stride,Int,"?bytesPerLine@QImage@@QEBAHXZ");LOAD(imageDestroy,Destroy,"??1QImage@@UEAA@XZ");
 LOAD(fromImage,FromImage,"?fromImage@QPixmap@@SA?AV1@AEBVQImage@@V?$QFlags@W4ImageConversionFlag@Qt@@@@@Z");LOAD(ratio,Ratio,"?devicePixelRatio@QPixmap@@QEBANXZ");LOAD(setRatio,SetRatio,"?setDevicePixelRatio@QPixmap@@QEAAXN@Z");
 
 LOAD(iw,Int,"?width@QImage@@QEBAHXZ");LOAD(ih,Int,"?height@QImage@@QEBAHXZ");LOAD(imageKey,Key,"?cacheKey@QImage@@QEBA_JXZ");LOAD(pixDestroy,Destroy,"??1QPixmap@@UEAA@XZ");
 for(auto&h:hooks){*h.original=(void*)GetProcAddress(gui,h.name);if(!*h.original)return -3;}
 wchar_t buf[32768];GetModuleFileNameW(self,buf,32768);std::wstring path(buf);path=path.substr(0,path.find_last_of(L"\\/")+1)+L"icon-masks.txt";HANDLE f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);if(f==INVALID_HANDLE_VALUE)return -4;DWORD size=GetFileSize(f,0),got=0;if(size>12000000){CloseHandle(f);return -5;}std::string text(size,'\0');ReadFile(f,text.data(),size,&got,0);CloseHandle(f);if(got!=size)return -6;
 masks=new std::unordered_set<uint64_t>;cache=new std::map<std::pair<long long,DWORD>,Obj*>;std::istringstream stream(text);uint64_t v;while(stream>>std::hex>>v)masks->insert(v);if(masks->empty())return -7;
 pixelAssets=new std::unordered_set<uint64_t>;assetSizes=new std::unordered_set<uint64_t>;
 path=std::wstring(buf);path=path.substr(0,path.find_last_of(L"\\/")+1)+L"pixel-assets.txt";
 f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);if(f==INVALID_HANDLE_VALUE)return -8;size=GetFileSize(f,0);if(size>16000000){CloseHandle(f);return -9;}text.assign(size,'\0');got=0;ReadFile(f,text.data(),size,&got,0);CloseHandle(f);if(got!=size)return -10;std::istringstream pixels(text);int w,h;while(pixels>>std::dec>>w>>h>>std::hex>>v){pixelAssets->insert(v);assetSizes->insert((uint64_t(w)<<32)|uint32_t(h));}
 InitializeCriticalSection(&cacheLock);ready=true;int count=patch(L"WeComCore.dll")+patch(L"FunView.dll")+patch(L"Qt5Widgets.dll");return count?count:1;
}
extern "C" __declspec(dllexport) const void* TestIcon(const void*p){HMODULE self;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)TestIcon,&self);if(installIcons(self)<0)return nullptr;return icon(p);}

