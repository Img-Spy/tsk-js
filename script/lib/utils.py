import os
import subprocess
import sys
import shutil
import platform

def execute_no_stdout(argv, env, cwd):
    try:
        output = subprocess.check_output(argv, stderr=subprocess.STDOUT, env=env, cwd=cwd)
        return output
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e

def execute_stdout(argv, env, cwd):
    try:
        process = subprocess.Popen(argv, stderr=None, cwd=cwd, env=env)
        process.wait()
        return process
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e

def execute(argv, env=os.environ, cwd=None, print_stdout=False, root=False):
    if not isinstance(argv, list):
        argv = [argv]

    if root and platform.system() == 'Linux':
        argv = ["sudo"] + argv

    if print_stdout:
        return execute_stdout(argv, env, cwd)
    else:
        return execute_no_stdout(argv, env, cwd)
    
def file_exists(path):
    return os.path.exists(path)

def rmdir(path):
    shutil.rmtree(path, ignore_errors=True)
