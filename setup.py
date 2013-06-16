from distutils.core import setup
from distutils.extension import Extension

setup(name='giljoy',
      packages=['giljoy'],
      ext_modules=[Extension('giljoy._giljoy',
                             ['giljoy/_giljoy.c'],
                             ),
                   ]
      )
