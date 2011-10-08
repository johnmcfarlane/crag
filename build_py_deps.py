#  build_py_deps.py
#  crag
#
#  Created by John McFarlane on 10/2/11.
#  Copyright 2011 John McFarlane. All rights reserved.


from modulefinder import ModuleFinder
import os
import string
import shutil
import sys


def process_file(filename, find_dir, replace_dir):
    if not filename:
        return True

    left = filename.find(find_dir)
    if left < 0:
        return False
        
    right = left + len(find_dir)
    
    dest_filename = filename[:left] + replace_dir + filename[right:]

    try:
        dest_dir = os.path.dirname(dest_filename)
        os.makedirs(dest_dir)
    except OSError:
        pass

    destination = dest_dir + filename[right :]

    try:
        shutil.copyfile(filename, dest_filename)
    except IOError:
        return False

    return True


# project_directory means the whole project - not just ./xcode/crag
def process_files(script, source_dir, target_dir):
    print('scanning',script)
    
    finder = ModuleFinder()
    finder.run_script(script)
    #finder.report()

    # Print modules found
    keys = finder.modules.keys()
    for key in keys:
        m = finder.modules[key]
        process_file(m.__file__, source_dir, target_dir) 

	# Get any old filename
    m = finder.modules['encodings.aliases']
    aliases_filename = m.__file__
    encodings_dir = os.path.dirname(aliases_filename)

    # This is the list of necessary modules which ModduleFinder fails to find.
    process_file(encodings_dir + '/utf_8.py', source_dir, target_dir)
    process_file(encodings_dir + '/latin_1.py', source_dir, target_dir)
    process_file(encodings_dir + '/ascii.py', source_dir, target_dir)
    process_file(encodings_dir + '/utf_32_be.py', source_dir, target_dir)


process_files(os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2]), os.path.abspath(sys.argv[3]))


#finder.report()

#print('Loaded modules:')
#for name, mod in finder.modules.items():
#    print(name, ': ', mod.__path__, end='')
#    print(','.join(list(mod.globalnames.keys())[:1]))

#print('-'*50)
#print('Modules not imported:')
#print('\n'.join(finder.badmodules.keys()))
