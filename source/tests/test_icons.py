import os,ctypes,pathlib,re
os.environ['QT_QPA_PLATFORM']='offscreen'
from PyQt5.QtWidgets import QApplication
from PyQt5.QtSvg import QSvgRenderer
from PyQt5.QtGui import QImage,QPainter,QPixmap,QColor
from PyQt5.QtCore import QByteArray
from PyQt5 import sip
# Reuse only the resource reader portion, not mask generation.
import os,struct,zlib,pathlib,re
os.environ['QT_QPA_PLATFORM']='offscreen'
from PyQt5.QtWidgets import QApplication
from PyQt5.QtSvg import QSvgRenderer
from PyQt5.QtGui import QImage,QPainter
from PyQt5.QtCore import Qt,QByteArray
app=QApplication([])
b=pathlib.Path('D:/WXWorkLocal/westlake/3.5.0.4433/res/res.rcc').read_bytes()
_,ver,tree,data,names=struct.unpack('>4s4I',b[:20]); icons=[]
def walk(i,path=''):
 p=tree+i*22;no,flags=struct.unpack_from('>IH',b,p);n=struct.unpack_from('>H',b,names+no)[0];name=b[names+no+6:names+no+6+2*n].decode('utf-16be');path=path+'/'+name if i else ''
 if flags&2:
  count,first=struct.unpack_from('>II',b,p+6)
  for j in range(first,first+count):walk(j,path)
 elif path.endswith('.svg') and (('general_gender_badge_male' in path) or ('/glyph/' in path and re.search(r'/(bubble_fill|mail_fill|calendar_fill|document|home|apps|briefcase|organization|contacts|workbench|workplace|building|grid|person|address_book)',path))):
  off=struct.unpack_from('>I',b,p+10)[0]+data;size=struct.unpack_from('>I',b,off)[0];s=b[off+4:off+4+size];s=zlib.decompress(s[4:]) if flags&1 else s;icons.append((path,s))
walk(0)

dll=ctypes.WinDLL(str(pathlib.Path(__file__).resolve().parents[2]/'engine'/'theme_engine.dll'))
dll.TestIcon.argtypes=[ctypes.c_void_p];dll.TestIcon.restype=ctypes.c_void_p
svg=next(s for p,s in icons if p.endswith('general_gender_badge_male.svg'))
im=QImage(16,16,QImage.Format_ARGB32);im.fill(0);p=QPainter(im);QSvgRenderer(QByteArray(svg)).render(p);p.end();pix=QPixmap.fromImage(im)
dll.ConfigureTest.argtypes=[ctypes.c_void_p]
for rgb in [0xcd4880,0x26997a,0x8257d6,0x626977,0xcd4880]:
 dll.ConfigureTest(0xc0000000|rgb)
 ptr=dll.TestIcon(sip.unwrapinstance(pix));assert ptr and ptr!=sip.unwrapinstance(pix)
 result=sip.wrapinstance(ptr,QPixmap).toImage();c=result.pixelColor(6,5)
 target=QColor('#%06x'%rgb)
 assert abs(c.hslHue()-target.hslHue())<=3,(hex(rgb),c.name())
 assert abs(c.hslSaturation()-target.hslSaturation())<=5,(hex(rgb),c.name())
 print('PASS icon color/cache',c.name())
for state in [0,0x80000000|0xcd4880]:
 dll.ConfigureTest(state);assert dll.TestIcon(sip.unwrapinstance(pix))==sip.unwrapinstance(pix)
print('PASS restore and icon toggle')
dll.ConfigureTest(0xc0000000|0xcd4880)
plain=QPixmap(32,32);plain.fill(QColor('#4b95f3'));assert dll.TestIcon(sip.unwrapinstance(plain))==sip.unwrapinstance(plain);print('PASS: non-icon blue image unchanged')
large=QPixmap(150,150);large.fill(QColor('#4b95f3'));assert dll.TestIcon(sip.unwrapinstance(large))==sip.unwrapinstance(large);print('PASS: large image unchanged')
