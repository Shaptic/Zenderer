import Tkinter as tk
import ttk
import pygame

import zGUI

DEFAULT_ENTITY = {
    'stretch':  'false',
    'depth': '1',
    'attributes': '0x00',
    'texture': ''
}

class EntityPropertyWindow(zGUI.PropertyWindow):
    def __init__(self, ent, parent, **kw):
        apply(zGUI.PropertyWindow.__init__, (self, parent, ent))
        
        self.StretchVar = tk.StringVar()
        self.TextureVar = tk.StringVar()

        self.Stretch = ttk.Checkbutton(self, text='Stretch Texture',
                                       variable=self.StretchVar,
                                       onvalue='true', offvalue='false')
        self.Texture = ttk.Entry(self, textvariable=self.TextureVar)
        ttk.Label(self, text='Texture').grid(row=1, column=0)

        self.Stretch.grid(row=0, column=0)
        self.Texture.grid(row=1, column=1)
        
        self.StretchVar.set(ent.details['stretch'])
        self.TextureVar.set(ent.details['texture'] \
                         if ent.details['texture'] else ent.filename)

    def Exit(self):
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
    
    def Load(self, filename=None, surface=None):
        assert not(filename and surface), 'not both'
        self.surface = pygame.image.load(filename) if filename else surface
        self.filename = filename if filename else ''
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
