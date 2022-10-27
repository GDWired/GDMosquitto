#!/usr/bin/env python

import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=['src/'])
sources = Glob('src/*.cpp')
if env['platform'] == "osx":
    # brew install mosquitto
    env.Append(CPPPATH=["/opt/homebrew/include"])
    env.Append(LIBPATH=["/opt/homebrew/lib"])
    env.Append(LIBS=["libmosquittopp"])
elif env['platform'] in ('x11', 'linux'):
    #env.Append(CPPPATH=["/usr/include"])
    #env.Append(LIBPATH=["/usr/lib"])
    env.Append(LIBS=["libmosquittopp"])
if env['platform'] == "windows":
    # Assume that mosquitto/devel is in the path
    env.Append(CPPPATH=[os.environ['PATH'].split(';')])
    env.Append(LIBPATH=[os.environ['PATH'].split(';')])
    env.Append(LIBS=["mosquittopp"])
if env['platform'] == "android":
    # brew install mosquitto
    env.Append(CPPPATH=["android/include"])
    if env["android_arch"] == "arm64v8":
        env.Append(LIBPATH=["android/lib/arm64-v8a"])
        env['platform'] += "/arm64-v8a"
    else:
        env.Append(LIBPATH=["android/lib/armeabi-v7a"])
        env['platform'] += "/armeabi-v7a"
    env.Append(LIBS=["libmosquittopp"])
    if sys.platform == "darwin":
        env.Append(LINKFLAGS=["-shared"])

# Create lib
sources = Glob("src/*.cpp")
library = env.SharedLibrary("demo/addons/GDMosquitto/{}/libgdmosquitto{}".format(env['platform'], env["SHLIBSUFFIX"]), source=sources)
Default(library)
