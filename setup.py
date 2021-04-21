#!/usr/bin/env python3
import os
import re
import sys
import sysconfig
import platform
import subprocess

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
from glob import glob


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)

        import multiprocessing
        subprocess.check_call(['cmake', '--build', '.'] + build_args + ['--', '--jobs='+str(multiprocessing.cpu_count())], cwd=self.build_temp)
        print()

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
        subprocess.call(['./*_test'],
                        cwd=os.path.join('build',
                                         self.distutils_dir_name('temp')),
                        shell=True)



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
    ext_modules=[
        CMakeExtension('stator.core')
    ],
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass=dict(build_ext=CMakeBuild),
    setup_requires=['pytest-runner'],
    tests_require=['pytest'],
    zip_safe=False,
)
