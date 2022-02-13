import re
import argparse
import os
import subprocess

import dace
from dace.sdfg import *
from dace.data import Scalar
from dace.properties import CodeBlock

from c2d_ast2sdfg import *
from c2d_c_ast_transforms import *
from c2d_c_ast_validators import *
from c2d_ast_output import *
from c2d_workflow import *


def get_cli_arguments():
    parser = argparse.ArgumentParser(
        description='CTD - Transform C programs to SDFG graphs.')
    parser.add_argument("-f", help="input file; C or C++")
    parser.add_argument(
        "-d",
        help="input directory; transform all C or C++ files in directory")
    parser.add_argument(
        "-p",
        "--print-ast",
        help="print the entire AST to \'tmp/before.txt\' and \'tmp/after.txt\'",
        action="store_true")
    parser.add_argument("-v",
                        "--verbose",
                        help="print more output",
                        action="store_true")

    arguments = parser.parse_args()

    if arguments.f is None and arguments.d is None:
        print(
            "please provide either an input file (-f) or an input directory (-d)"
        )
        exit()

    return arguments


def get_files(cli_arguments):
    files = []

    if cli_arguments.f is not None:
        path, filename = os.path.split(cli_arguments.f)
        filecore, extension = os.path.splitext(filename)
        files.append({
            "path": path,
            "filecore": filecore,
            "extension": extension
        })

    if cli_arguments.d is not None:
        with os.scandir(cli_arguments.d) as scan:
            for entry in scan:
                path, filename = os.path.split(entry.path)
                filecore, extension = os.path.splitext(filename)

                if extension in [".cpp", ".c", ".cxx", ".cc"]:
                    files.append({
                        "path": path,
                        "filecore": filecore,
                        "extension": extension
                    })

    return files


def execute_command(command):
    process = subprocess.run(command,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
    return process.returncode, process.stdout + " " + process.stderr


if __name__ == "__main__":

    args = get_cli_arguments()
    filecores = [
        #"spmv",
        #"2mm",
        #"sparselu",
        #"2mm",
        "3mm",
        #"adi",
        #"atax",
        #"bicg",
        #"cholesky",  #issue out degree
        #"correlation",
        #"covariance",
        #"deriche",
        #"doitgen",
        #"durbin",  #issue n not defined
        #"fdtd2d",
        #"floyd_warshall",  #issue, infer expr type
        #"gemm",
        #"gemver",
        #"gesummv",
        #"gramschmidt",
        #"heat",
        #"lu",  #issue out degree
        #"jacobi_1d",  #issue out degree
        #"jacobi_2d",
        #"gemm",
        #"ludcmp",  #issue out degree
        #"mvt",
        #"nussinov",  #broken
        #"seidel",
        #"symm",  #issue out degree
        #"syr2k",
        # "syrk",
        # "trisolv",  #        -topological
        # "trmm",  #issue  out degree
    ]
    #    filecores2 = [
    #"2mm",
    #"3mm",
    #"adi",
    #"atax",
    #"bicg",
    #"cholesky",  #issue out degree
    #"correlation",
    #"covariance",
    ##"deriche",
    #"doitgen",
    #"durbin",  #issue n not defined
    #"fdtd2d",
    #"floyd_warshall",  #issue, infer expr type
    #"gemm",
    #"gemver",
    #"gesummv",
    #"gramschmidt",
    #"heat",
    #"lu",  #issue out degree
    #"jacobi_1d",  #issue out degree
    #"jacobi_2d",
    #"gemm",
    #"ludcmp",  #issue out degree
    #"mvt",
    #"nussinov",  #issue infer expr type
    #"seidel",
    #"symm",  #issue out degree
    #"syr2k",
    #"syrk",
    #"trisolv",  #        -topological
    #"trmm",  #issue  out degree
    #]
    autoopt = False

    filecore = os.path.splitext(os.path.basename(args.f))[0]
    fileextension = os.path.splitext(os.path.basename(args.f))[1]

    fileextension = ".c"

    _dir = os.path.dirname(args.f)

    for filecore in filecores:
        filename = _dir + "/" + filecore + fileextension

        c2d_workflow(_dir,
                     filecore,
                     fileextension,
                     autoopt=False,
                     verbose=False,
                     print_ast=False)
