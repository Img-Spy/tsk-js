import os
import subprocess
import sys


slethkit_path="./vendor/sleuthkit"


def execute(argv, env=os.environ, cwd=None):
    try:
        output = subprocess.check_output(argv, stderr=subprocess.STDOUT, env=env, cwd=cwd)
        return output
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e


def file_exists(path):
    return os.path.exists(path)


def main():
    print("Update submodule")
    execute(["git", "submodule", "update"])

    print("Prepare sleutkit library")
    if not file_exists(slethkit_path + "/configure"):
        print("  - Bootstrap")
        execute("./bootstrap", cwd=slethkit_path)

    if not file_exists(slethkit_path + "/Makefile"):
        print("  - Configure")
        execute("./configure", cwd=slethkit_path)

    print("Build sleutkit library")
    execute("make", cwd=slethkit_path)


if __name__ == '__main__':
    sys.exit(main())
