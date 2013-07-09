import os

SHADER_FILE = 'include/Zenderer/CoreGraphics/ShaderFiles.hpp'
SHADER_PATH = 'data/shaders/'

def shader(path):
    data = open(path, 'r').readlines()
    return data, len(data)

def main():
    header = open(SHADER_FILE, 'r')
    data = header.readlines()
    header.close()
    index = 0
    
    for i in xrange(len(data)):
        if data[i].find('// Begin shader data.') != -1:
            index = i
            break

    data = data[:index+1] + ['\n']

    for i in os.listdir(SHADER_PATH):
        fn = i.split('.')
        const = '_'.join([x.upper() for x in fn])
        
        print "Creating shader constant for %s ... " % i,
        
        data.append('%s/// Created from %s.\n' % ((' ' * 4), SHADER_PATH + i))
        data.append('%sstatic const string_t %s = string_t(\n' % (' ' * 4, const))
        
        lines, count = shader(SHADER_PATH + i)
        for x in xrange(len(lines)):
            line = lines[x]
            string = ''
            
            if line.strip() != '':
                string = '%s"%s"' % ((' ' * 8), line[:-1])
                
            data.append(string + '\n')
            
        data.append('%s);\n\n' % (' ' * 4))
        print "Done."
        
    print "Writing %d bytes to %s ... " % (sum([len(x) for x in data]), SHADER_FILE),
    
    header = open(SHADER_FILE, 'w')
    header.write(''.join(data))
    
    # Close the namespaces / preprocessor stuff
    header.write('''}
}

#endif // ZENDERER__CORE_GRAPHICS__SHADER_FILES_HPP

/** @} **/
    ''')
    header.close()
        
    print "Done."

if __name__ == "__main__": main()
 