#!/usr/bin/env python3
import os
import re
import sys
import sysconfig
import platform
import subprocess
from pathlib import Path

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
from glob import glob

# Setup following advice from
# https://www.benjack.io/2017/06/12/python-cpp-tests.html
#
# In particular here:
# https://gist.github.com/hovren/5b62175731433c741d07ee6f482e2936

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        build_directory = os.path.abspath(self.build_temp)

        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + build_directory,
            '-DPYTHON_EXECUTABLE=' + sys.executable
        ]

        cfg = 'Debug' if self.debug else 'Release'
        #cfg = 'Debug'
        build_args = ['--config', cfg]

        cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]

        # Assuming Makefiles
        import multiprocessing
        build_args += ['--', '--jobs='+str(multiprocessing.cpu_count())]
        self.build_args = build_args
        
        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''),
            self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # CMakeLists.txt is in the same directory as this setup.py file
        cmake_list_dir = os.path.abspath(os.path.dirname(__file__))
        print('-'*10, 'Running CMake prepare', '-'*40)
        subprocess.check_call(['cmake', cmake_list_dir] + cmake_args,
                              cwd=self.build_temp, env=env)

        print('-'*10, 'Building extensions', '-'*40)
        cmake_cmd = ['cmake', '--build', '.'] + self.build_args
        subprocess.check_call(cmake_cmd,
                              cwd=self.build_temp)

        # Move from build temp to final position
        for ext in self.extensions:
            self.move_output(ext)

    def move_output(self, ext):
        build_temp = Path(self.build_temp).resolve()
        dest_path = Path(self.get_ext_fullpath(ext.name)).resolve()
        source_path = build_temp / self.get_ext_filename(ext.name)
        dest_directory = dest_path.parents[0]
        dest_directory.mkdir(parents=True, exist_ok=True)
        self.copy_file(source_path, dest_path)
        
        
ext_modules = [
  CMakeExtension('stator.core'),
]

from setuptools.command.test import test as TestCommand
class CatchTestCommand(TestCommand):
    """
    A custom test runner to execute both Python unittest tests and C++ tests.
    """
    def distutils_dir_name(self, dname):
        """Returns the name of a distutils build directory"""
        dir_name = "{dirname}.{platform}-{version[0]}.{version[1]}"
        return dir_name.format(dirname=dname,
                               platform=sysconfig.get_platform(),
                               version=sys.version_info)

    def run(self):
        # Run Python tests
        super(CatchTestCommand, self).run()
        print("\nPython tests complete, now running C++ tests...\n")
        # Run catch tests
        subprocess.call(['ctest -j8 --output-on-failure'], cwd=os.path.join('build', self.distutils_dir_name('temp')), shell=True)



setup(
    name="stator",
    version="0.0.1",
    author="Marcus Bannerman",
    author_email="m.bannerman@gmail.com",
    url="https://github.com/toastedcrumpets/stator",
    description="The python stator interface.",
    long_description="",
    package_dir={
        '': 'pysrc',
    },
    packages=find_packages('pysrc'),
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    cmdclass=dict(build_ext=CMakeBuild, test=CatchTestCommand),
    setup_requires=['pytest-runner'],
    tests_require=['pytest'],
    zip_safe=False,
)
