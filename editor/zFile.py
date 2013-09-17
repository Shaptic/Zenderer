from zLight import make2f

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
        final += '</light>\n\n'
        f.write(final)
        f.flush()

    @staticmethod
    def ExportPolygon(f, p):
        final  = '<entity>\n'
        final += '    vertex='
        final += '\n    vertex='.join('%d,%d' % (int(v[0]), int(v[1])) for v in p)
        final += '\n</entity>\n\n'
        f.write(final)
        f.flush()
