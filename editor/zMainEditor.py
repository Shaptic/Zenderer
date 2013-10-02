# Zenderer level editor.
import os

import Tkinter as tk
import tkFileDialog as tkfile
import ttk

import pygame

from zEntity    import *
from zLight     import *
from zGUI       import *
from zFile      import Exporter

import zGeometry as geometry

__version__ = '0.0.1-dev'
__author__ = 'George Kudrayvtsev'

VALID_IMAGES = ['.png', '.tga']

class Main:
    def __init__(self):
        self.quit = False
        self.window = tk.Tk()

        self.window.geometry('1000x600')
        self.window.title('Zenderer Level Editor')
        self.window.protocol('WM_DELETE_WINDOW', self.Exit)

        self._Setup()

        # Enable the Pygame window as the Tkinter frame
        os.environ["SDL_WINDOWID"] = str(self.PyGame.winfo_id())
        os.environ["SDL_WINDOWDRIVER"] = "windib"

        pygame.display.init()
        self.screen = pygame.display.set_mode((800, 600), pygame.NOFRAME, 32)

        self.entities = []
        self.lights   = []
        self.verts    = []
        self.indices  = []
        self.polys    = []
        self.origin   = (0, 0)

        self.BaseEntity = None
        self.BaseLight  = None

        # Populate entity list by recursively searching for images
        # in the directory provided by the settings module.
        for root, dirs, files in os.walk(os.getcwd()):
            self.EntityList['values'] = tuple(
                list(self.EntityList['values']) + [f for f in files \
                    if os.path.splitext(f)[1] in VALID_IMAGES and f != 'light.png'
                ]
            )

        # Set up menus.
        file = tk.Menu(tearoff=0)
        file.add_command(label='Import...')
        file.add_command(label='Export...', command=self._Export)
        file.add_separator()
        file.add_command(label='Quit', command=self.Exit)

        main = tk.Menu()
        main.add_cascade(menu=file, label='File')
        self.window.config(menu=main)

    def Run(self):
        while not self.quit:
            for evt in pygame.event.get():
                if evt.type == pygame.QUIT: self.Exit()
                elif evt.type == pygame.MOUSEBUTTONDOWN:

                    ent = self._GetEntityAt(evt.pos)
                    lit = self._GetLightAt(evt.pos)

                    if evt.button == 3:
                        self._Evt_ShowContextMenu(evt.pos)

                    elif evt.button == 1:
                        self._Evt_AddObject(evt.pos)

                    elif evt.button == 2 and self.BaseEntity and self.BaseEntity.on:
                        self.BaseEntity.on = False
                        self.BaseEntity.Move(evt.pos)
                        self.BaseEntity.end = evt.pos

                elif evt.type == pygame.KEYDOWN:
                    if evt.key == pygame.K_f: self.PyGame.focus()

            if pygame.mouse.get_focused():
                self._Evt_HandlePanning(pygame.mouse.get_pos())

            self.screen.fill((0, 0, 0))
            fn = self.EntityList.get()

            for e in self.entities: e.Update(self.screen)
            for l in self.lights:   l.Update(self.screen)
            for x in xrange(1, len(self.verts)):
                pygame.draw.line(self.screen, (255, 255, 255),
                                 self.verts[x], self.verts[x-1])
            for p in self.polys:
                pygame.draw.polygon(self.screen, (255, 255, 255), p)

            if self.ObjVar.get() == 1 and fn:
                mp = pygame.mouse.get_pos()
                if not self.BaseEntity or self.BaseEntity.filename != fn:
                    self.BaseEntity = Entity().Load(filename=self.EntityList.get())

                if self.BaseEntity.on: self.BaseEntity.end = mp
                else: self.BaseEntity.Move(mp)
                self.BaseEntity.Update(self.screen)

            elif self.ObjVar.get() == 2:
                if not self.BaseLight: self.BaseLight = Light()
                self.BaseLight.Move(pygame.mouse.get_pos())
                self.BaseLight.Update(self.screen)

            pygame.draw.line(self.screen, (255, 255, 0),
                (self.origin[0] - 24, self.origin[1]),
                (self.origin[0] + 24, self.origin[1]))

            pygame.draw.line(self.screen, (255, 255, 0),
                (self.origin[0], self.origin[1] - 24),
                (self.origin[0], self.origin[1] + 24))

            pygame.display.update()
            self.window.update()

    def Exit(self): self.quit = True

    def _Setup(self):
        self.PyGame     = tk.Frame(width=800, height=600)
        self.SideBar    = tk.Frame(width=200, height=600)
        self.PyGame.grid( row=0, column=0)
        self.SideBar.grid(row=0, column=1, padx='10')

        tk.Label(self.SideBar, text='Editing Mode:').grid(row=0, column=0)
        self.ObjVar     = tk.IntVar()
        self.EntityRad  = ttk.Radiobutton(self.SideBar, text='Entities',
                                          variable=self.ObjVar, value=1)
        self.LightRad   = ttk.Radiobutton(self.SideBar, text='Lights',
                                          variable=self.ObjVar, value=2)
        self.SpawnRad   = ttk.Radiobutton(self.SideBar, text='Spawn Points',
                                          variable=self.ObjVar, value=3)
        self.VertRad    = ttk.Radiobutton(self.SideBar, text='Vertices',
                                          variable=self.ObjVar, value=4)
        self.EntityList = ttk.Combobox(self.SideBar, state='readonly')

        self.EntityRad.grid(column=0, row=1, sticky='ew')
        self.LightRad.grid(column=0, row=2, sticky='ew')
        self.SpawnRad.grid(column=0, row=3, sticky='ew')
        self.VertRad.grid(column=0, row=4, sticky='ew')
        self.EntityList.grid(column=0, row=5, pady=10)

        self.EntityMenu = ContextMenu(self.PyGame, tearoff=0)
        self.EntityMenu.add_command(label='Delete Object',
                                    command=lambda: self._DelObj(
            self.entities, self._GetEntityAt(pygame.mouse.get_pos())))
        self.EntityMenu.add_command(label='Properties',
                                    command=self._Evt_ShowEntProp)

        self.LightMenu = ContextMenu(self.PyGame, tearoff=0)
        self.LightMenu.add_command(label='Delete Light',
                                    command=lambda: self._DelObj(
            self.lights, self._GetLightAt(pygame.mouse.get_pos())))
        self.LightMenu.add_command(label='Properties',
                                    command=self._Evt_ShowLightProp)

    def _Export(self):
        filename = tkfile.asksaveasfilename(
            defaultextension='.zlv',
            filetypes=[('Zenderer Level files', '*.zlv'),
                       ('All Files', '.*')],
            initialfile='level.zlv',
            title='Export Level As...', parent=self.window)

        with open(filename, 'w') as f:
            for e in self.entities:
                e.Move((e.start[0] - self.origin[0], e.start[1] - self.origin[1]))
                Exporter.ExportEntity(f, e)
                e.Move((e.start[0] + self.origin[0], e.start[1] + self.origin[1]))

            for l in self.lights:
                l.Move((l.start[0] - self.origin[0], l.start[1] - self.origin[1]))
                Exporter.ExportLight(f, l)
                l.Move((l.start[0] + self.origin[0], l.start[1] + self.origin[1]))

            for i in xrange(len(self.polys)):
                self.polys[i] = [(
                    v[0] - self.origin[0],
                    v[1] - self.origin[1]) for v in self.polys[i]
                ]

                Exporter.ExportPolygon(f, self.polys[i], self.indices[i])

                self.polys[i] = [(
                    v[0] + self.origin[0],
                    v[1] + self.origin[1]) for v in self.polys[i]
                ]

    def _Evt_AddObject(self, pos):
        if self.ObjVar.get() == 1 and self.EntityList.get():
            if not self.BaseEntity.on:
                self.BaseEntity.Start(pos)
                return

            self.BaseEntity.Stop(pos)
            self.entities.append(Entity().Load(filename=self.EntityList.get()))
            self.entities[-1].pos = pos
            self.entities[-1].start = self.BaseEntity.start
            self.entities[-1].end = self.BaseEntity.end
            self.BaseEntity.start = self.BaseEntity.end = pos
            print self.entities[-1].start, self.entities[-1].end

        elif self.ObjVar.get() == 2:
            self.lights.append(Light())
            self.lights[-1].Move(pos)

        elif self.ObjVar.get() == 4:
            self.verts.append(pos)

    def _Evt_LoadEntity(self):
        self.entities.append(Entity().Load(filename=self.EntityList.get()))

    def _Evt_ShowContextMenu(self, pos):
        wx = self.PyGame.winfo_rootx()
        wy = self.PyGame.winfo_rooty()

        ent = self._GetEntityAt(pos)
        lit = self._GetLightAt(pos)

        if self.ObjVar.get() == 4 and len(self.verts) > 2:
            tmpverts = []
            tmpinds  = []

            if geometry.is_concave(self.verts):
                self.verts = geometry.triangulate(self.verts)
                print self.verts
                for v in self.verts:
                    if v not in tmpverts:
                        tmpverts.append(v)
                        tmpinds.append(tmpverts.index(v))

            else: tmpverts = self.verts
            self.indices.append(tmpinds)
            self.polys.append(tmpverts)
            self.verts = []

        elif self.ObjVar.get() == 2 and lit:
            self.LightMenu.post(pos[0] + wx, pos[1] + wy)
            self.LightMenu.registered = lit

        elif ent:
            self.EntityMenu.post(pos[0] + wx, pos[1] + wy)
            self.EntityMenu.registered = ent

    def _Evt_ShowEntProp(self):
        EntityPropertyWindow(self.EntityMenu.registered, self.window)

    def _Evt_ShowLightProp(self):
        LightPropertyWindow(self.LightMenu.registered, self.window)

    def _GetEntityAt(self, pos):
        self.entities.reverse()
        for e in self.entities:
            if e.Collides(pygame.Rect(pos[0], pos[1], 1, 1)):
                break
        else:
            e = None
        self.entities.reverse()
        return e

    def _GetLightAt(self, pos):
        self.lights.reverse()
        for l in self.lights:
            if l.Collides(pygame.Rect(pos[0], pos[1], 1, 1)):
                break
        else:
            l = None
        self.lights.reverse()
        return l

    def _DelObj(self, objs, ref): objs.remove(ref)

    def _Evt_HandlePanning(self, pos):
        dx = 0
        dy = 0

        if pos[0] < 20:     dx =  2
        elif pos[0] > 780:  dx = -2

        if pos[1] < 20:     dy =  2
        elif pos[1] > 580:  dy = -2

        all_objects = self.entities + self.lights
        for o in all_objects: o.Move((o.start[0] + dx, o.start[1] + dy))

        self.verts  = [(v[0] + dx, v[1] + dy) for v in self.verts]
        self.origin = (self.origin[0] + dx, self.origin[1] + dy)

        for p in self.polys:
            self.polys[self.polys.index(p)] = [(v[0] + dx, v[1] + dy) for v in p]

if __name__ == '__main__':
    Editor = Main()
    Editor.Run()
