"""Primitivas deterministas para los planos revisados de Gorran."""
from collections import deque
import gen_gorran_v2 as G
from gen_niveles_av19 import Nivel,_T

class Mapa(Nivel):
    def __init__(self,nombre,titulo,w,h):
        super().__init__(nombre,G.ascii_(titulo),w,h,suelo=_T['roca'],colision=('roca','rio'))
        self.titulo_es=titulo;self.zonas=[];self.notas=[];self.reservas=set()
    def piso(self,x,y,w,h,suelo='losa_oscura'):
        if x<1 or y<1 or x+w>self.w-1 or y+h>self.h-1:raise ValueError('Suelo fuera de '+self.nombre)
        self.rect(x,y,w,h,_T[suelo])
    def sala(self,clave,titulo,x,y,w,h,suelo='losa_oscura'):
        self.piso(x,y,w,h,suelo);self.zonas.append({'id':clave,'nombre':titulo,'rect':[x,y,w,h],'centro':[x+w//2,y+h//2]})
    def camino(self,puntos,ancho=3,suelo='losa_oscura'):
        radio=ancho//2
        for (x,y),(bx,by) in zip(puntos,puntos[1:]):
            if x!=bx and y!=by:raise ValueError('Camino no ortogonal')
            for xx in range(min(x,bx),max(x,bx)+1):
                for yy in range(min(y,by),max(y,by)+1):
                    self.piso(xx-radio,yy-radio,ancho,ancho,suelo)
    def nota(self,x,y,texto):
        self.notas.append([x,y,texto]);self.reservas.add((x,y))
    def reservar(self,*celdas):self.reservas.update(celdas)
    def andable(self,x,y):return 0<=x<self.w and 0<=y<self.h and self.g[y][x] not in self.colision and self.g[y][x]!=0
    def pon(self,oid,x,y,**extra):
        if not self.andable(x,y) or (x,y) in self.ocupades() or (x,y) in self.reservas:
            raise ValueError(f'{self.nombre}: {oid} no cabe en {x},{y}')
        return self.obj(oid,x,y,**extra)
    def portal(self,oid,x,y,destino,llegada,nombre='Paso'):
        if not self.andable(x,y) or (x,y) in self.ocupades():raise ValueError('Portal ocupado '+oid)
        G.puerta(oid,nombre,'felpudo')
        self.reservas.add((x,y))
        return self.obj(oid,x,y,targetLevel=f'assets/levels/{destino}.json',targetPosition={'x':llegada[0],'y':llegada[1]})
    def muro(self,x,y,w,h,huecos=(),familia='pedra_fosca'):
        huecos=set(huecos)
        for yy in range(y,y+h):
            for xx in range(x,x+w):
                if (xx,yy) in huecos:continue
                if (xx,yy) in self.ocupades() or (xx,yy) in self.reservas:raise ValueError('Muro sobre reserva')
                self.g[yy][xx]=0
                # Las caras norte/oeste se dibujan con piezas del catálogo del juego.
                if h==1:self.obj(f'paret_{familia}_llisa',xx,yy)
                elif w==1:self.obj(f'paret_{familia}_llisa_o3',xx,yy)
    def alcanzables(self,catalogo):
        bloqueos={(o['position']['x'],o['position']['y']) for o in self.objetos if catalogo.get(o['objectId'],{}).get('blocksMovement') and not catalogo.get(o['objectId'],{}).get('flotante')}
        visto={self.start};cola=deque([self.start])
        while cola:
            x,y=cola.popleft()
            for q in [(x-1,y),(x+1,y),(x,y-1),(x,y+1)]:
                if q not in visto and self.andable(*q) and q not in bloqueos:visto.add(q);cola.append(q)
        return visto
