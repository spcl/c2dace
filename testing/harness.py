from unittest import defaultTestLoader, TextTestRunner
import subprocess
import argparse


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


def execute_command(command):
    process = subprocess.run(command,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
    return process.returncode, process.stdout + " " + process.stderr


class CTDTests:
    @staticmethod
    def full_ctd_test(test_case, dir, file_core, file_extension):
        source_file_name = dir + file_core + file_extension
        compiled_file_name = "tmp/" + file_core + ".out"
        transformed_file_name = dir + file_core + "-dace.cpp"
        compiled_transformed_file_name = "tmp/" + file_core + "-dace.out"

        #print("compiling " + source_file_name + " ...")
        code, output = execute_command(
            ["clang++", source_file_name, "-o", compiled_file_name])
        test_case.assertEqual(
            code, 0,
            Formatting.format_string(
                "compilation of " + source_file_name + " failed",
                Formatting.RED) + "\n" + output)

        #print("running CTD for " + source_file_name + " ...")
        code, output = execute_command(
            ["python3", "0.8/CTD_0_8.py", "-f", source_file_name])
        test_case.assertEqual(
            code, 0,
            Formatting.format_string("CTD for " + source_file_name + " failed",
                                     Formatting.RED) + "\n" + output)

        #print("compiling output file of CTD...")
        code, output = execute_command([
            "clang++", "0.8/tests/csrc/dace-base.c", transformed_file_name,
            "-o", compiled_transformed_file_name, "-I",
            dace_directory + "/runtime/include"
        ])
        test_case.assertEqual(
            code, 0,
            Formatting.format_string(
                "compilation of " + transformed_file_name + " failed",
                Formatting.RED) + "\n" + output)

        #print("running " + compiled_file_name + " ...")
        code, output = execute_command(["./" + compiled_file_name])
        test_case.assertEqual(
            code, 0,
            Formatting.format_string(
                "running " + compiled_file_name + " failed", Formatting.RED) +
            "\n" + output)

        #print("running " + compiled_transformed_file_name + " ...")
        code, output = execute_command(["./" + compiled_transformed_file_name])
        test_case.assertEqual(
            code, 0,
            Formatting.format_string(
                "running " + compiled_transformed_file_name + " failed",
                Formatting.RED) + "\n" + output)


def get_cli_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--test", help="only run one test")

    return parser.parse_args()


def main():
    args = get_cli_arguments()
    if args.test is not None:
        defaultTestLoader.testNamePatterns = ["*" + args.test]

    test_suite = defaultTestLoader.discover("simple/", pattern="*.py")
    runner = TextTestRunner(verbosity=2)
    runner.run(test_suite)


if __name__ == "__main__":
    main()
