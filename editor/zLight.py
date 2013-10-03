import Tkinter          as tk
import tkColorChooser   as tkcolor
import ttk
import pygame

from zEntity import *
from zGUI    import *

DEFAULT_LIGHT = {
    'type':         'POINT',
    'color':        '1.00,1.00,1.00',
    'attenuation':  '0.05,0.01,0.00',
    'brightness':   '1.0'
}

def make2f(s):
    parts = s.strip().split(',')
    return ftos([float(x) for x in parts])

def ftos(f): return ','.join('%0.2f' % x for x in f)

def color2hex(c):
    return '#' + ''.join(['%02x' % int(min(float(x)*255, 255)) \
        for x in c.split(',')
    ])

class LightPropertyWindow(PropertyWindow):
    def __init__(self, light, parent):
        apply(PropertyWindow.__init__, (self, parent, light))

        self.TypeVar    = MakeVar(light.details['type'].upper())
        self.ColorVar   = MakeVar(light.details['color'])
        self.BrtVar     = MakeVar(light.details['brightness'])
        self.MaxAngVar  = MakeVar()
        self.MinAngVar  = MakeVar()

        if self.TypeVar.get() == 'SPOT':
            self.MaxAngVar.set(light.details['maxangle'])
            self.MinAngVar.set(light.details['minangle'])

        self.TypeVar.trace('w', self._Evt_LightType)

        self.Point  = MakeRadioBtn(self, 'Point',       self.TypeVar, 'POINT')
        self.Spot   = MakeRadioBtn(self, 'Spotlight',   self.TypeVar, 'SPOT')
        self.Ambient= MakeRadioBtn(self, 'Ambient',     self.TypeVar, 'AMBIENT')

        self.Bright = tk.Entry(self, textvariable=self.BrtVar)
        self.MaxAng = tk.Entry(self, textvariable=self.MaxAngVar, state=tk.DISABLED)
        self.MinAng = tk.Entry(self, textvariable=self.MinAngVar, state=tk.DISABLED)

        self.Color  = tk.Frame(self, bg='#FFFFFF', width=100, height=100)
        self.Color.bind('<ButtonRelease-1>', self._Evt_ChooseColor)

        PlaceWidget(tk.Label(self,
                             text='All entries that require floating point '
                                  'values should be comma-separated.'
                            ), 0, 0, columnspan=2)

        PlaceWidget(tk.Label(self, text='Lighting Type: '), 2, 0, 'e')
        PlaceWidget(tk.Label(self, text='Color: '), 4, 0, 'e')
        PlaceWidget(tk.Label(self, text='Brightness: '), 5, 0, 'e')
        PlaceWidget(tk.Label(self, text='Max Angle: '), 6, 0, 'e')
        PlaceWidget(tk.Label(self, text='Min Angle: '), 7, 0, 'e')

        PlaceWidget(self.Point,     1, 1, 'w')
        PlaceWidget(self.Spot,      2, 1, 'w')
        PlaceWidget(self.Ambient,   3, 1, 'w')
        PlaceWidget(self.Color,     4, 1, 'w', 10)
        PlaceWidget(self.Bright,    5, 1, 'w')
        PlaceWidget(self.MaxAng,    6, 1, 'w')
        PlaceWidget(self.MinAng,    7, 1, 'w')

        self.Color.config(bg=color2hex(self.ColorVar.get()))
        self._Evt_LightType()

    def Exit(self):
        self.applied.details['type'] = self.TypeVar.get()
        self.applied.details['color'] = self.ColorVar.get()
        self.applied.details['brightness'] = self.BrtVar.get()
        if self.TypeVar.get() == 'SPOT':
            self.applied.details['maxangle'] = \
                360.0 if not self.MaxAngVar.get() else self.MaxAngVar.get()

            self.applied.details['minangle'] = \
                0.0 if not self.MinAngVar.get() else self.MinAngVar.get()

        color = [min(float(x) * 255, 255) for x in self.ColorVar.get().split(',')]
        surf = pygame.Surface((self.applied.surface.get_width(),
                               self.applied.surface.get_height()))
        surf.fill(tuple(color))
        surf.blit(pygame.image.load('light.png'), (0, 0))
        self.applied.surface = surf
        self.destroy()

    def _Evt_ChooseColor(self, evt=None):
        (rgb, hex) = tkcolor.askcolor(color2hex(self.ColorVar.get()),
                                      title='Choose a Light Color...',
                                      parent=self)

        if rgb:
            self.ColorVar.set(','.join(str(x / 255.0) for x in rgb))
            self.Color.config(bg=hex)

    def _Evt_LightType(self, *args):
        if self.TypeVar.get() == 'SPOT':
            self.MaxAng.config(state='normal')
            self.MinAng.config(state='normal')

        else:
            self.MaxAng.config(state='disabled')
            self.MinAng.config(state='disabled')

class Light(Entity):
    def __init__(self, d=DEFAULT_LIGHT):
        apply(Entity.__init__, (self, ))
        self.details = dict(d)
        self.Load(filename='light.png')
        surf = pygame.Surface((self.surface.get_width(),
                               self.surface.get_height()))
        surf.fill((255, 255, 255))
        surf.blit(self.surface, (0, 0))
        self.surface = surf
