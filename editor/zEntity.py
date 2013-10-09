import Tkinter as tk
import ttk
import pygame

from zGUI import *

DEFAULT_ENTITY = {
    'stretch':  'false',
    'depth': '1',
    'attributes': '0x00',
    'texture': ''
}

ATTR = {
    'PHYSICAL': 0x01,
    'ANIMABLE': 0x02
}

class EntityPropertyWindow(PropertyWindow):
    def __init__(self, ent, parent, **kw):
        apply(PropertyWindow.__init__, (self, parent, ent))

        self.CollideVar = MakeVar(int(ent.details['attributes'], 16) & ATTR['PHYSICAL'], tk.IntVar)
        self.StretchVar = MakeVar(ent.details['stretch'])
        self.TextureVar = MakeVar(
            ent.details['texture'] if ent.details['texture'] else ent.filename
        )

        self.Collide = ttk.Checkbutton(self, variable=self.CollideVar)
        self.Stretch = ttk.Checkbutton(self, variable=self.StretchVar,
                                       onvalue='true', offvalue='false')
        self.Texture = ttk.Entry(self, textvariable=self.TextureVar)

        PlaceWidget(ttk.Label(self, text='Physical'), 0, 0)
        PlaceWidget(self.Collide, 0, 1)
        PlaceWidget(ttk.Label(self, text='Stretch'), 1, 0)
        PlaceWidget(self.Stretch, 1, 1)
        PlaceWidget(ttk.Label(self, text='Texture'), 2, 0)
        PlaceWidget(self.Texture, 2, 1)

    def Exit(self):
        self.applied.details['attributes'] = '0x%02x' % (1 & (ATTR['PHYSICAL'] if self.CollideVar.get() else 1))
        self.applied.details['stretch'] = self.StretchVar.get()
        self.applied.details['texture'] = self.TextureVar.get()
        self.destroy()

class Entity:
    def __init__(self):
        self.surface = None
        self.start = (0, 0)
        self.end = tuple(self.start)
        self.details = dict(DEFAULT_ENTITY)
        self.on = False
        self.filename = ''

    def Load(self, filename=None, surface=None):
        assert not(filename and surface), 'not both'
        self.surface = pygame.image.load(filename) if filename else surface
        self.filename = filename if filename else ''
        self.details['texture'] = self.filename
        return self

    def Collides(self, rect):
        return pygame.Rect(
            self.start[0], self.start[1],
            max(abs(self.end[0] - self.start[0]), self.surface.get_width()),
            max(abs(self.end[1] - self.start[1]), self.surface.get_height())
        ).colliderect(rect)

    def Move(self, pos):
        self.end = (self.end[0] + (pos[0] - self.start[0]),
                    self.end[1] + (pos[1] - self.start[1]))
        self.start = pos

    def Start(self, pos):
        self.start = self.end = pos
        self.on = True

    def Stop(self, pos):
        self.end = pos
        self.on = False

    def Update(self, scr):
        if self.surface:
            d = (self.surface.get_width(), self.surface.get_height())

            # Calculate how many should be rendered.
            count_x = abs(self.start[0] - self.end[0]) / d[0]
            count_y = abs(self.start[1] - self.end[1]) / d[1]

            # Render count_x * count_y copies.
            for x in xrange(count_x + 1):
                for y in xrange(count_y + 1):
                    coord = (self.start[0] + (x * d[0]),
                            (self.start[1] + (y * d[1])))

                    scr.blit(self.surface, coord)

        return self
