#!/bin/bash
set -e -u -x

function repair_wheel {
    wheel="$1"
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel" --plat "$PLAT" -w /io/wheelhouse/
    fi
}


# Install a system package required by our library
yum install -y gtest-devel 

# Compile wheels
ls /opt/python/
mkdir -p /io/wheelhouse
for VER in cp39-cp39; do #cp36-cp36m cp37-cp37m cp38-cp38 
    PYBIN=/opt/python/$VER/bin
    "${PYBIN}/pip" install -r /io/dev-requirements.txt
    "${PYBIN}/pip" wheel /io/ --no-deps -w /io/wheelhouse/
done

# Bundle external shared libraries into the wheels
for whl in /io/wheelhouse/*.whl; do
    #auditwheel repair "$whl"
    repair_wheel "$whl"
done

# Install packages and test
for VER in cp39-cp39; do #cp36-cp36m cp37-cp37m cp38-cp38 
    PYBIN=/opt/python/$VER/bin
    "${PYBIN}/pip" install stator --no-index -f /io/wheelhouse
    #(cd "$HOME"; "${PYBIN}/nosetests" pymanylinuxdemo)
done
