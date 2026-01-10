#!/usr/bin/env python
import os
import sys

# Find godot-cpp
godot_cpp_path = os.environ.get('GODOT_CPP_PATH', '../godot-cpp')
if not os.path.exists(godot_cpp_path):
    print(f"Error: godot-cpp not found at {godot_cpp_path}")
    print("Set GODOT_CPP_PATH environment variable or clone godot-cpp next to this project")
    sys.exit(1)

env = SConscript(godot_cpp_path + '/SConstruct')

# Add source files
env.Append(CPPPATH=['src/'])
sources = Glob('src/*.cpp')

# Link against libutil for forkpty
env.Append(LIBS=['util'])

# Build the shared library
library = env.SharedLibrary(
    target='bin/libgdpty{}{}'.format(env['suffix'], env['SHLIBSUFFIX']),
    source=sources,
)

Default(library)
