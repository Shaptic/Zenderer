import Tkinter as tk

class ContextMenu(tk.Menu):
    def __init__(self, parent, **kw):
        apply(tk.Menu.__init__, (self, parent), kw)
        self.vars = []
        self.registered = None

    def add_checkbutton(self, **kw):
        v = tk.IntVar()
        v.set(0)
        self.vars.append(v)
        kw["variable"] = v
        tk.Menu.add_checkbutton(self, **kw)
        return len(self.vars) - 1

    def SetVar(self, i, val):
        if(i >= len(self.vars)): return
        val = int(val)
        self.vars[i].set(val)

    def ToggleVar(self, i):
        if(i >= len(self.vars)):    return
        if(self.GetVar(i) == 0):    self.vars[i].set(1)
        else:                       self.vars[i].set(0)

    def GetVar(self, i): return int(self.vars[i].get())

class PropertyWindow(tk.Toplevel):
    def __init__(self, parent, primary, **kw):
        apply(tk.Toplevel.__init__, (self, parent), kw)
        
        self.geometry('%dx%d+%d+%d' % (
            self.winfo_width(), self.winfo_height(),
            parent.winfo_rootx() + int(primary.start[0]),
            parent.winfo_rooty() + int(primary.start[1])))
        self.wm_geometry("")

        self.protocol('WM_DELETE_WINDOW', self.Exit)
        self.transient(parent)
        self.grab_set()
        self.focus()
        self.applied = primary

    def Exit(self): self.destroy()