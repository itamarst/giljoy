from distutils.core import setup
from distutils.extension import Extension

setup(name='giljoy',
      packages=['giljoy'],
      ext_modules=[Extension('giljoy._giljoy',
                             ['giljoy/_giljoy.c'],
                             libraries=['pthread', 'dl'],
                             define_macros=[('_GNU_SOURCE', '1')]),
                   ],
      Extension('giljoy._giljoypreload',
                             ['giljoy/_giljoypreload.c'],
                             libraries=['pthread', 'dl'],
                             define_macros=[('_GNU_SOURCE', '1')]),
                   ]
      )
