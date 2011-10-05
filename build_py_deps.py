#!./xcode4/bin/python3.1

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


def process_file(filename, src_dir, dst_dir):
    if not filename:
        return True
        
    left = filename.find(src_dir)
    if left < 0:
        return False
    
    right = left + len(src_dir)

    try:
        os.makedirs(dst_dir)
    except OSError:
        pass

    src = filename
    dst = dst_dir + filename[right :]
    #print("...")
    #print(dst_dir)
    print(src, '->', dst)

    try:
        shutil.copyfile(src, dst)
    except IOError:
        return False

    return True


# project_directory means the whole project - not just ./xcode/crag
def process_files(project_directory, target_directory):
    script = project_directory + '/script/main.py'
    print('scanning',script)
    
    finder = ModuleFinder()
    finder.run_script(script)
    #finder.report()

    # Print modules found
    keys = sorted(finder.modules.keys())
    for key in keys:
        m = finder.modules[key]
        process_file(m.__file__, 'lib/python3.1/', target_directory + '/lib/python3.1/') or process_file(m.__file__, 'lib/python3.1/encodings/', target_directory + '/lib/python3.1/encodings/') or process_file(m.__file__, 'lib/python3.1/lib-dynload/', target_directory + '/lib/python3.1/lib-dynload/')
    

    # This is the list of necessary modules which ModduleFinder fails to find.
    shutil.copyfile(project_directory + '/xcode4/lib/python3.1/encodings/utf_8.py', target_directory + '/lib/python3.1/encodings/utf_8.py')
    shutil.copyfile(project_directory + '/xcode4/lib/python3.1/encodings/latin_1.py', target_directory + '/lib/python3.1/encodings/latin_1.py')
    shutil.copyfile(project_directory + '/xcode4/lib/python3.1/encodings/ascii.py', target_directory + '/lib/python3.1/encodings/ascii.py')
    shutil.copyfile(project_directory + '/xcode4/lib/python3.1/encodings/utf_32_be.py', target_directory + '/lib/python3.1/encodings/utf_32_be.py')


process_files(sys.argv[1], sys.argv[2])


#finder.report()

#print('Loaded modules:')
#for name, mod in finder.modules.items():
#    print(name, ': ', mod.__path__, end='')
#    print(','.join(list(mod.globalnames.keys())[:1]))

#print('-'*50)
#print('Modules not imported:')
#print('\n'.join(finder.badmodules.keys()))
