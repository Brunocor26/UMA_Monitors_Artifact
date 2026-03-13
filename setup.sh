#!/bin/bash
set -e

git submodule update --init --recursive
git apply --directory=rtmlib rtmlib.patch

echo "Done."
