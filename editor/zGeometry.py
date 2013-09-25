import math

def triangle_area(p):
    assert len(p) == 3, 'must be a triangle'
    
    return (p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) - \
           (p[2][0] - p[0][0]) * (p[1][1] - p[0][1]) > 0;
 
def triangle_contains(v, tri):
    # Barycentric coordinates.
    denom = ((tri[1][1] - tri[2][1]) * (tri[0][0] - tri[2][0]) + \
             (tri[2][0] - tri[1][0]) * (tri[0][1] - tri[2][1]))
    if denom == 0.0: return False
    denom = 1.0 / denom
 
    # alpha = ((p2.y - p3.y)*(p.x - p3.x)  + (p3.x - p2.x)*(p.y - p3.y)) /
    #         ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y))
    alpha = denom * ((tri[1][1] - tri[2][1]) * (v[0] - tri[2][0]) \
                  +  (tri[2][0] - tri[1][0]) * (v[1] - tri[2][1]))
    if alpha < 0: return False
 
    # beta = ((p3.y - p1.y)*(p.x - p3.x) + (p1.x - p3.x)*(p.y - p3.y)) /
    #        ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));
    beta = denom  * ((tri[2][1] - tri[0][1]) * (v[0] - tri[2][0]) \
                  +  (tri[0][0] - tri[2][0]) * (v[1] - tri[2][1]))
    if beta < 0: return False
            
    return (1.0 - alpha - beta) >= 0
 
def clip(shape, ear):
    l = len(shape)
    return (shape[(ear-1) % l], shape[ear], shape[(ear+1) % l])

def get_orientation(vertices):
    # Use orientation of the top-left-most vertex.
    i = 0; left = vertices[i]
    
    for x in xrange(len(vertices)):
        v = vertices[x]
        if v[0] < left[0] or (v[0] == left[0] and v[1] < left[1]):
            left = v
            i = x

    return triangle_area(clip(vertices, i))

def is_concave(vertices):
    if len(vertices) < 4: return False

    for i in xrange(len(vertices)):
        curr = vertices[i]
        next = vertices[(i + 1) % len(vertices)]
        prev = vertices[(i - 1) % len(vertices)]
        
        one = (prev[0] - curr[0], prev[1] - curr[1])
        two = (next[0] - curr[0], next[1] - curr[1])
        nml = (one[1], -one[0])
        
        mag = math.sqrt(nml[0]*nml[0] + nml[1]*nml[1]) * \
              math.sqrt(two[0]*two[0] + two[1]*two[1])
          
        ang = math.acos((nml[0] * two[0] + nml[1] * two[1]) / mag);
        print (ang * 180 / math.pi)
        if (ang * 180.0 / math.pi) > 90.0: return True
    
    return True

def triangulate(vertices):
    if len(vertices) <= 3: return vertices
    
    triangulated = []
    
    # Use orientation of the top-left-most vertex.
    o = get_orientation(vertices)

    while len(vertices) >= 3:
        reflex = []
        eartip = None
        tringl = []
        
        # For each vertex in the shape
        for i in xrange(len(vertices)):
            if eartip: break

            # Create a triangle from vertex to adjacent vertices.
            tri = clip(vertices, i)

            prev= tri[0]
            curr= tri[1]
            next= tri[2]
            
            # If polygon's orientation doesn't match that of the triangle, 
            # it's definitely a reflex and not an ear.
            if get_orientation(tri) != o:
                reflex.append(i)
                continue
            
            # Test reflex vertices first.
            for x in reflex:
                # If we are testing ourselves, skip.
                if vertices[x] in tri: continue
                
                # If any v in triangle, not ear.
                elif triangle_contains(vertices[x], tri): break
            
            else:
                # No reflexes, so we test all past current vertex.
                for x in xrange(i+2, len(vertices)):
                    if vertices[x] in tri: continue
                    elif triangle_contains(vertices[x], tri): break
                
                # No vertices in the triangle, we are an ear.
                else: eartip = i
        
        if not eartip: break
        tringl = clip(vertices, eartip)
        del vertices[eartip]
        triangulated += list(tringl)

    return triangulated