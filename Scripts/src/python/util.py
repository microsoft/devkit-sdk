import string
import random
import sys
from os import environ
from os.path import (
    join,
    isfile,
    isdir,
)
import subprocess


def unique_id_generator(size=6, chars=string.ascii_lowercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))


def get_pip_path():
    python_path = sys.executable[:sys.executable.rindex('\\python.exe')]
    return join(python_path, 'Scripts', 'pip.exe')


def add_env_path(path, key):
    if isfile(path) or isdir(path):
        environ[key] = path
    else:
        print '{} is not a valid dir or file path'.format(path)

if __name__ == '__main__':
    script_path = get_pip_path()
    add_env_path(script_path, 'pip')
    subprocess.call('{} install azure-cli'.format(environ['pip']))
