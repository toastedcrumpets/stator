from setuptools import setup, find_packages
import sys,os

# Track down the pybind11 directory, add it to path, import
# requirements, then remove it again.
DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(DIR, "extern", "pybind11"))
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
del sys.path[-1]


__version__ = "0.0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension("stator.core",
        ["pysrc/stator/core.cpp"],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        include_dirs=['.', 'extern/eigen']
        ),
]

setup(
    name="stator",
    version=__version__,
    author="Marcus Bannerman",
    author_email="m.bannerman@gmail.com",
    url="https://github.com/toastedcrumpets/stator",
    description="The python stator interface.",
    long_description="",
    packages=find_packages('pysrc'),
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    setup_requires=['pytest-runner'],
    tests_require=['pytest'],
    zip_safe=False,
)
