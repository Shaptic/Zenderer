import Tkinter as tk
import ttk
import pygame

from zEntity    import *
from zGUI       import *

DEFAULT_SPAWN = {
    'type':  'ENEMY',
    'blacklist': '',
    'whitelist': ''
}

class SpawnPropertyWindow(PropertyWindow):
    def __init__(self, spn, parent, **kw):
        apply(PropertyWindow.__init__, (self, parent, spn))

        self.TypeVar= MakeVar(spn.details['type'].upper())
        self.BLVar  = MakeVar(spn.details['blacklist'])
        self.WLVar  = MakeVar(spn.details['whitelist'])

        self.Enemy  = MakeRadioBtn(self, 'Enemy',   self.TypeVar, 'ENEMY')
        self.Player = MakeRadioBtn(self, 'Player',  self.TypeVar, 'PLAYER')
        self.Item   = MakeRadioBtn(self, 'Item',    self.TypeVar, 'ITEM')

        self.WList  = ttk.Entry(self, textvariable=self.WLVar)
        self.BList  = ttk.Entry(self, textvariable=self.BLVar)

        PlaceWidget(tk.Label(self,
                             text='All entries that require a list of items '
                                  'should have comma-separated values.'
                            ), 0, 0, columnspan=2)

        PlaceWidget(tk.Label(self, text='Spawn Type: '), 2, 0, 'e')
        PlaceWidget(tk.Label(self, text='Whitelist: '), 4, 0, 'e')
        PlaceWidget(tk.Label(self, text='Blacklist: '), 5, 0, 'e')

        PlaceWidget(self.Enemy,     1, 1, 'w')
        PlaceWidget(self.Player,    2, 1, 'w')
        PlaceWidget(self.Item,      3, 1, 'w')
        PlaceWidget(self.WList,     4, 1, 'w')
        PlaceWidget(self.BList,     5, 1, 'w')

    def Exit(self):
        self.applied.details['type'] = self.TypeVar.get()
        self.applied.details['blacklist'] = self.BLVar.get()
        self.applied.details['whitelist'] = self.WLVar.get()

        color = (255, 0, 0) if self.TypeVar.get() == 'ENEMY' else (0, 0, 255)
        surf = pygame.Surface((32, 32))
        surf.fill(color)
        self.applied.Load(surface=surf)
        self.destroy()

class Spawn(Entity):
    def __init__(self, d=DEFAULT_SPAWN):
        apply(Entity.__init__, (self, ))
        self.details = dict(d)

        color = (255, 0, 0)
        if d['type'] == 'PLAYER':
            color = (0, 0, 255)
        elif d['type'] == 'ITEM':
            color = (0, 255, 0)

        surf = pygame.Surface((32, 32))
        surf.fill(color)
        self.Load(surface=surf)
