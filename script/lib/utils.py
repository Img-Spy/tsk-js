import os
import subprocess
import sys


def execute(argv, env=os.environ, cwd=None):
    try:
        output = subprocess.check_output(argv, stderr=subprocess.STDOUT, env=env, cwd=cwd)
        return output
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e


def file_exists(path):
    return os.path.exists(path)
