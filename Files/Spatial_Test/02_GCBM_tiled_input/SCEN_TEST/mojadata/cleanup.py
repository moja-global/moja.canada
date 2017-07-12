import os
import shutil
from contextlib import contextmanager

_temp_files = []
_temp_dirs = []

@contextmanager
def cleanup():
    try:
        yield None
    finally:
        for file in _temp_files:
            if os.path.exists(file):
                os.unlink(file)

            file_dir = os.path.abspath(os.path.dirname(file))
            if os.path.isdir(file_dir) and not os.listdir(file_dir):
                os.rmdir(file_dir)

        for dir in _temp_dirs:
            shutil.rmtree(dir, ignore_errors=True)

def register_temp_file(path):
    _temp_files.append(path)

def register_temp_dir(path):
    _temp_dirs.append(path)
