from zLight     import *

class Exporter:
    @staticmethod
    def ExportEntity(f, e):
        final  = '<entity>\n'
        final += '    depth=%d\n' % int(e.details['depth'])
        final += '    position=%d,%d\n' % (int(e.start[0]), int(e.start[1]))

        if e.end[0] != e.start[0] or e.end[1] != e.start[1]:
            final += '    vertex=0,0\n'
            final += '    vertex=%d,0\n'    % (e.end[0] - e.start[0])
            final += '    vertex=%d,%d\n'   % (e.end[0] - e.start[0],
                                               e.end[1] - e.start[1])
            final += '    vertex=0,%d\n\n'  % (e.end[1] - e.start[1])

        final += '    stretch=%s\n' % e.details['stretch']
        final += '    texture=%s\n' % e.details['texture']
        final += '    attributes=%s\n' % e.details['attributes']
        final += '</entity>\n\n'
        f.write(final)
        f.flush()

    @staticmethod
    def ExportLight(f, l):
        final  = '<light type="%s">\n' % l.details['type'].upper()
        final += '    color=%s\n' % (make2f(l.details['color']))
        final += '    position=%d,%d\n' % (int(l.start[0]), int(l.start[1]))
        final += '    attenuation=%s\n' % (make2f(l.details['attenuation']))
        final += '    brightness=%0.2f\n' % (float(l.details['brightness']))
        
        if l.details['type'].upper() == 'SPOT':
            final += '    maxangle=%0.2f\n' % float(l.details['maxangle'])
            final += '    minangle=%0.2f\n' % float(l.details['minangle'])
            
        final += '</light>\n\n'
        f.write(final)
        f.flush()

    @staticmethod
    def ExportPolygon(f, p, i=None):
        final  = '<entity>\n'
        final += '    vertex='
        final += '\n    vertex='.join('%d,%d' % (int(v[0]), int(v[1])) for v in p)
        if i: final += '\n    indices=%s\n' % (','.join([str(x) for x in i]))
        final += '\n</entity>\n\n'
        f.write(final)
        f.flush()

    @staticmethod
    def ExportSpawn(f, s):
        final  = '<spawn type="%s">\n' % s.details['type'].upper()
        final += '    position=%d,%d\n' % (s.start[0], s.start[1])
        if s.details['whitelist']:
            final += '    whitelist=%s\n' % s.details['whitelist']
        if s.details['blacklist']:
            final += '    blacklist=%s\n' % s.details['blacklist']
        final += '</spawn>\n\n'
        f.write(final)
        f.flush()

import re
from zEntity    import *
from zSpawn     import *

class Importer:
    def __init__(self, ents, lights, spawns, verts):
        self.ents   = ents
        self.lights = lights
        self.spawns = spawns
        self.verts  = verts
    
    def LoadFromFile(self, filename):
        f = open(filename, 'r')
        while True:
            line = f.readline()
            if not line: break
            line = line.strip()
            
            if line.find('<entity') == 0:
                verts= []
                ent = Entity()

                # We are beginning a block either of an entity or
                # of an arbitrary vertex polygon. We differentiate by
                # whether it has a texture or not, and how many
                # vertices it has.
                while True:
                    line = f.readline()
                    if not line: break
                    line = line.strip()
                    
                    if line.strip() == '</entity>':
                        if len(verts) > 4 and not ent.filename:
                            self.verts.append(verts)
                            break

                        else:
                            if verts:
                                ent.end = (verts[2][0] + ent.start[0],
                                           verts[2][1] + ent.start[1])

                            self.ents.append(ent)
                            break

                    if not line or line[0] == '/': continue

                    elif line.find('depth=') == 0:
                        ent.details['depth'] = line.split('=')[1]

                    elif line.find('position=') == 0:
                        parts = line.split('=')[1].split(',')
                        ent.Move([int(i) for i in parts])

                    elif line.find('vertex=') == 0:
                        parts = line.split('=')[1].split(',')
                        verts.append((int(parts[0]), int(parts[1])))

                    elif line.find('stretch=') == 0:
                        ent.details['stretch'] = line.split('=')[1]

                    elif line.find('texture=') == 0:
                        ent.Load(filename=line.split('=')[1])

            elif line.find('<light') == 0:
                lit = Light()
                lit.details['type'] = re.match('<light type="([A-Z]+)">',
                                               line, re.IGNORECASE).groups()[0]

                # We are beginning a light.
                while True:
                    line = f.readline()
                    if not line: break
                    line = line.strip()
                    
                    if line.strip() == '</light>':
                        self.lights.append(lit)
                        break

                    if not line or line[0] == '/': continue

                    elif line.find('color=') == 0:
                        lit.details['color'] = line.split('=')[1]
                        lit.SetColor(stoc(lit.details['color']))

                    elif line.find('attenuation=') == 0:
                        lit.details['attenuation'] = line.split('=')[1]

                    elif line.find('brightness=') == 0:
                        lit.details['brightness'] = line.split('=')[1]

                    elif line.find('maxangle=') == 0:
                        lit.details['maxangle'] = line.split('=')[1]

                    elif line.find('minangle=') == 0:
                        lit.details['minangle'] = line.split('=')[1]

                    elif line.find('position=') == 0:
                        parts = line.split('=')[1].split(',')
                        lit.Move([int(i) for i in parts])

            elif line.find('<spawn') == 0:
                spn = Spawn()
                spn.details['type'] = re.match('<spawn type="([A-Z]+)">',
                                               line, re.IGNORECASE).groups()[0]

                # We are beginning a spawn point block.
                while True:
                    line = f.readline()
                    if not line: break
                    line = line.strip()

                    if line.strip() == '</spawn>':
                        self.spawns.append(spn)
                        break

                    if not line or line[0] == '/': continue

                    elif line.find('whitelist=') == 0:
                        spn.details['whitelist'] = line.split('=')[1]

                    elif line.find('blacklist=') == 0:
                        spn.details['blacklist'] = line.split('=')[1]

                    elif line.find('position=') == 0:
                        parts = line.split('=')[1].split(',')
                        spn.Move([int(i) for i in parts])
