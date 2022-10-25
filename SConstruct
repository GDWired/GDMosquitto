#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=['src/'])
sources = Glob('src/*.cpp')

if env['platform'] == "osx":
    env.Append(CPPPATH=["/opt/homebrew/include"])
    env.Append(LIBPATH=["/opt/homebrew/lib"])
    env.Append(LIBS=["libmosquittopp"])
elif env['platform'] in ('x11', 'linux'):
    #env.Append(CPPPATH=["/usr/include"])
    #env.Append(LIBPATH=["/usr/lib"])
    env.Append(LIBS=["libmosquittopp"])
if env['platform'] == "windows":
    env.Append(CPPPATH=["C:\Program Files\mosquitto\devel"])
    env.Append(LIBPATH=["C:\Program Files\mosquitto\devel"])
    env.Append(LIBS=["mosquittopp"])

# Create lib
sources = Glob("src/*.cpp")
library = env.SharedLibrary("demo/addons/GDMosquitto/{}/libgdmosquitto{}".format(env['platform'], env["SHLIBSUFFIX"]), source=sources)
Default(library)
