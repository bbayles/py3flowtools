#! /usr/bin/env python

import sys

from distutils.core import setup, Extension

setup( name="pyflowtools", 
       version="0.3.4.1",
       author="Paul P. Komkoff Jr",
       author_email="i@stingr.net",
       license="GPL",
       url="http://pyflowtools.googlecode.com",
       ext_modules = [ Extension( "flowtools", ["flowtools.c"],
                                  libraries = [ "ft", "z" ],
                                 ) ] )

