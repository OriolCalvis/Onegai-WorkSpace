# Generador de guiones: funciones pequenas que emiten ordenes del guion GLFW.
class G:
    def __init__(self): self.l=[]
    def w(self,n=8): self.l.append(f"wait {n}"); return self
    def sl(self,ms): self.l.append(f"sleep {ms}"); return self
    def k(self,key,n=1,gap=6,hold=2):
        for _ in range(n): self.l.append(f"key {key} {hold}"); self.l.append(f"wait {gap}")
        return self
    def shot(self,name,pre=6): self.l.append(f"wait {pre}"); self.l.append(f"shot {name}"); return self
    def log(self,t): self.l.append(f"log {t}"); return self
    def raw(self,t): self.l.append(t); return self
    def save(self,path): open(path,'w').write("\n".join(self.l)+"\nquit\n"); return path

def crear_personaje(g, shots=None):
    """Titulo -> Nueva partida -> 14 pasos con eleccion por defecto."""
    g.w(20).k("ENTER",gap=8)
    g.k("ENTER",3,gap=8)
    for i in range(4):
        g.k("RIGHT",3,gap=4)
        if i<3: g.k("S",gap=4)
    g.k("ENTER",gap=8)
    g.k("ENTER",9,gap=8)
    g.k("ENTER",gap=8)
    return g

def prologo(g, n=14):
    """Pasa los dialogos del prologo y llega a ciudad_centro."""
    for i in range(n):
        g.k("ENTER",gap=12)
    g.sl(1200)
    g.k("ENTER",4,gap=12)   # llegada + encargo
    g.sl(600)
    return g

def andar(g, keys, ms=200):
    for k in keys:
        g.raw(f"key {k} 2"); g.sl(ms)
    return g
