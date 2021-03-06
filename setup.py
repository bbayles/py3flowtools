# setup.py
# Copyright 2015 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import os
import sys

from setuptools import setup, find_packages

long_description = (
    'Tools to NetFlow data from logs by providing Python wrappers around the '
    'flow-tools and flowd package binaries.'
)

install_dir = os.path.abspath(os.path.dirname(__file__))

# Install requires configparser for Python 2.x
install_requires = ['subprocess32'] if (sys.version_info[0] < 3) else []

setup(
    name='py3flowtools',

    version='2015.01.03',

    description='Library for reading NetFlow data from logs',
    long_description=(
        'Library for reading NetFlow data from logs. Requires flow-tools and '
        'flowd.'
    ),

    url='https://github.com/bbayles/py3flowtools',

    author='Bo Bayles',
    author_email='bbayles@gmail.com',

    license='MIT',

    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Telecommunications Industry',
        'Topic :: Utilities',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
    ],

    keywords='netflow flow-tools flowd',

    packages=find_packages(exclude=['tests']),

    install_requires=install_requires,

    test_suite='tests'
)
