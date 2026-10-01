import os,ctypes,pathlib
os.environ['QT_QPA_PLATFORM']='offscreen'
from PyQt5.QtWidgets import QApplication
from PyQt5.QtGui import QColor,QPen,QBrush,QLinearGradient,QImage,QPainter,QPixmap
from PyQt5 import sip
app=QApplication([]);d=ctypes.WinDLL(str(pathlib.Path(__file__).resolve().parents[2]/'engine'/'theme_engine.dll'))
d.ConfigureTest.argtypes=[ctypes.c_void_p];d.TestPen.argtypes=[ctypes.c_void_p,ctypes.c_void_p];d.TestBrush.argtypes=[ctypes.c_void_p,ctypes.c_void_p]
canvas=QImage(30,30,QImage.Format_ARGB32);canvas.fill(0);p=QPainter(canvas)
original=QColor('#2488ef');pen=QPen(original);g=QLinearGradient(0,0,30,30);g.setColorAt(0,original);g.setColorAt(1,QColor('#77baff'));brush=QBrush(g)
for rgb in [0xcd4880,0x21966f,0x8758d6,0x555555]:
 d.ConfigureTest(0xc0000000|rgb)
 assert d.TestPen(sip.unwrapinstance(p),sip.unwrapinstance(pen))==0
 assert d.TestBrush(sip.unwrapinstance(p),sip.unwrapinstance(brush))==0
 out=p.brush().gradient().stops()[0][1]
 assert out!=original,(hex(rgb),out.name())
 assert brush.gradient().stops()[0][1]==original,'Original gradient mutated'
 print('PASS color',hex(rgb),'pen',p.pen().color().name(),'gradient',out.name())
d.ConfigureTest(0)
d.TestPen(sip.unwrapinstance(p),sip.unwrapinstance(pen));d.TestBrush(sip.unwrapinstance(p),sip.unwrapinstance(brush))
assert p.pen().color()==original and p.brush().gradient().stops()[0][1]==original
p.end();print('PASS restore bypass, originals intact')
app.setStyleSheet('QWidget { color: #123456; }');assert d.Restore(None)==0;app.processEvents();assert '#123456' in app.styleSheet();print('PASS original QSS retained')
