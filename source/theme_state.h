#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>
extern volatile LONG themeState;
inline DWORD state(){return (DWORD)InterlockedCompareExchange(&themeState,0,0);}
inline bool enabled(){return (state()&0x80000000u)!=0;}
inline bool iconsEnabled(){return (state()&0xc0000000u)==0xc0000000u;}
inline bool mapBlue(int r,int g,int b,int&nr,int&ng,int&nb){
 DWORD s=state();if(!(s&0x80000000u))return false;
 double hi=std::max({r,g,b})/255.,lo=std::min({r,g,b})/255.,d=hi-lo;if(d<0.045||b<std::max(r,g))return false;
 double hue=60*(4+(r-g)/255./d);if(hue<190||hue>250)return false;
 double tr=((s>>16)&255)/255.,tg=((s>>8)&255)/255.,tb=(s&255)/255.;double mx=std::max({tr,tg,tb}),mn=std::min({tr,tg,tb}),td=mx-mn,tl=(mx+mn)/2;
 double th=0,ts=0;if(td>0.0001){ts=td/(1-std::abs(2*tl-1));if(mx==tr)th=60*std::fmod((tg-tb)/td+6,6);else if(mx==tg)th=60*((tb-tr)/td+2);else th=60*((tr-tg)/td+4);}
 double l=std::clamp((hi+lo)/2+(tl-.55)*.55,0.06,0.96),sat=std::min(ts,0.88);double c=(1-std::abs(2*l-1))*sat,x=c*(1-std::abs(std::fmod(th/60,2)-1)),m=l-c/2;double a=0,z=0,k=0;
 if(th<60){a=c;z=x;}else if(th<120){a=x;z=c;}else if(th<180){z=c;k=x;}else if(th<240){z=x;k=c;}else if(th<300){a=x;k=c;}else{a=c;k=x;}
 nr=int((a+m)*255+.5);ng=int((z+m)*255+.5);nb=int((k+m)*255+.5);return true;
}
