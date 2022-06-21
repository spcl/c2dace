from curses.ascii import isdigit
import unittest, os, sys, subprocess
from parameterized import parameterized

FOLDER = './simple_tests/'
DACE_INCLUDE = "~/.local/lib/python3.10/site-packages/dace/runtime/include"

def execute_command(command, shell=False):
    process = subprocess.run(command,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True, shell=shell)
    return process.returncode, process.stdout + " " + process.stderr

class Formatting:
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

    @staticmethod
    def format_string(s, format):
        return format + s + Formatting.END

    @staticmethod
    def format_bool(b):
        if b:
            return Formatting.format_string(str(b), Formatting.GREEN)
        return Formatting.format_string(str(b), Formatting.RED)

files = [f for f in os.listdir(FOLDER) if f.endswith('.c')]
tests = []
for f in files:

    code, output = execute_command("gcc " + FOLDER + f + " -lm", shell=True)
    if code != 0:
        print(Formatting.format_string("compilation of " + FOLDER + f + " failed", Formatting.RED) + "\n" + output)
        sys.exit(1)

    code, output = execute_command(["./a.out"])

    tests.append([f[:-2], output])

print("Loaded tests: ", tests)

class Testing(unittest.TestCase):
    @parameterized.expand(tests)
    def test_file(self, file, expected):
        execute_command(["rm -Rf tmp/* .dacecache _dacegraphs a.out orig"], shell=True)

        code, output = execute_command(
            ["python3", "c2dace/c2d.py", "-f", FOLDER + file + ".c"])
        self.assertEqual(
            code, 0,
            Formatting.format_string(
                "transformation of " + file + " failed",
                Formatting.RED) + "\n" + output)

        gcc_cmd = "gcc .dacecache/_" + file + "/sample/_" + file + "_main.cpp .dacecache/_" + file + "/src/cpu/_" + file + ".cpp -I " + DACE_INCLUDE + " -lstdc++ -lm"
        code, output = execute_command(gcc_cmd, shell=True)
        self.assertEqual(
            code, 0,
            Formatting.format_string(
                "compilation of " + file + " failed",
                Formatting.RED) + "\n" + output)

        code, output = execute_command(["./a.out"])
        self.assertEqual(
            output, expected,
            Formatting.format_string(
                "expected different return code of " + file + " failed",
                Formatting.RED) + "\n" + output)

if __name__ == '__main__':
    unittest.main()