#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=['src/'])
sources = Glob('src/*.cpp')

if env['platform'] == "osx":
    env.Append(CPPPATH=["/opt/homebrew/include"])
    env.Append(LIBPATH=["/opt/homebrew/lib"])
    env.Append(LIBS=["libmosquittopp"])
elif env['platform'] in ('x11', 'linux'):
    env.Append(CPPPATH=["/usr/include"])
    env.Append(LIBPATH=["/usr/lib"])
    env.Append(LIBS=["libmosquittopp"])

# Create lib
sources = Glob("src/*.cpp")
library = env.SharedLibrary("demo/addons/mosquitto/{}/libgdmosquitto{}".format(env['platform'], env["SHLIBSUFFIX"]), source=sources)
Default(library)
