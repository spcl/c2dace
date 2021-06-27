import re
import argparse
import os

import dace
from dace.sdfg import *
from dace.data import Scalar
from dace.properties import CodeBlock

import sys

from c2d_util import *
from c2d_c_ast_transforms import *
from c2d_c_ast_validators import *
from c2d_ast_output import *


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


def dump(node: Node, annotate_fields=True, include_attributes=False):
    """
    Return a formatted dump of the tree in node.  This is mainly useful for
    debugging purposes.  If annotate_fields is true (by default),
    the returned string will show the names and the values for fields.
    If annotate_fields is false, the result string will be more compact by
    omitting unambiguous field names.  Attributes such as line
    numbers and column offsets are not dumped by default.  If this is wanted,
    include_attributes can be set to true.
    """
    def _format(node, depth=0):
        prefix = "\n" + depth * "  "
        if isinstance(node, Node):
            args = []
            keywords = annotate_fields
            for field in node._fields:
                try:
                    value = getattr(node, field)
                    # print(value)
                except AttributeError:
                    keywords = True
                else:
                    if keywords:
                        args.append('%s=%s' %
                                    (field, _format(value, depth=depth + 1)))
                    else:
                        args.append(_format(value, depth=depth + 1))
            if include_attributes and node._attributes:
                for a in node._attributes:
                    try:
                        args.append(
                            '%s=%s' %
                            (a, _format(getattr(node, a), depth=depth + 1)))
                    except AttributeError:
                        pass
            if len(node.integrity_exceptions) > 0:
                args.append('%s=%s' % (Formatting.format_string(
                    "integrity_exceptions",
                    Formatting.RED), str(node.integrity_exceptions)))
            return prefix + \
                Formatting.format_string(node.__class__.__name__, [Formatting.BOLD, Formatting.BLUE]) + \
                '(%s)' % ((prefix + " ") + (prefix + " ").join(args))
        elif isinstance(node, list):
            return '[%s]' % "".join(_format(x, depth=depth + 1) for x in node)
        return repr(node)

    if not isinstance(node, Node):
        raise TypeError('expected AST, got %r' % node.__class__.__name__)
    return _format(node)


if __name__ == "__main__":

    args = get_cli_arguments()
    dace.Config.set("compiler", "default_data_types", value="C")
    #dace.Config.set("optimizer", "detect_control_flow", value="false")
    filecores = [
        "2mm",
        #"3mm",
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
        #"nussinov",  #issue infer expr type
        #"seidel",
        #"symm",  #issue out degree
        #"syr2k",
        #"syrk",
        #"trisolv",  #        -topological
        #"trmm",  #issue  out degree
    ]
    autoopt = False

    filecore = os.path.splitext(os.path.basename(args.f))[0]
    fileextension = os.path.splitext(os.path.basename(args.f))[1]

    #filecores = ["add"]
    fileextension = ".c"
    #fileextension = "_short.cc"

    _dir = os.path.dirname(args.f)
    #_dir = os.path.dirname("C://")
    #clang.cindex.Config.set_library_file(cfg.clang_library_file)

    for filecore in filecores:
        filename = _dir + "/" + filecore + fileextension

        print("transforming file: " + filename)

        index = clang.cindex.Index.create()
        print("parsing...")
        #tu = index.parse("bla.txt")
        tu = index.parse(filename)
        if len(tu.diagnostics) > 0:
            print("encountered " + str(len(tu.diagnostics)) +
                  Formatting.format_string(" diagnostics", Formatting.YELLOW) +
                  " during parsing...")
            if args.verbose:
                for d in tu.diagnostics:
                    print(" " + str(d))
            else:
                print("run the program in verbose mode to print diagnostics.")

        print("copying ast...")
        from os import listdir
        from os.path import isfile, join
        filelist = [f for f in listdir(_dir) if isfile(join(_dir, f))]
        dirfilelist = []
        #for i in filelist:
        #    dirfilelist.append(_dir + "/" + i)
        print("FILELIST:", dirfilelist)
        files = [filename] + dirfilelist

        own_ast = create_own_ast(tu.cursor, files)
        changed_ast = own_ast

        attr_validator = ValidateAllAttributesPresent()
        changed_ast = attr_validator.visit(changed_ast)
        print(attr_validator.report())

        if args.print_ast:
            print("saving ast after copying to tmp/before.txt")
            with open("tmp/before.txt", "w") as f:
                f.write(dump(changed_ast, include_attributes=True))

            with open("tmp/before.pseudo.cpp", "w") as f:
                try:
                    f.write(c2d_ast_output.get_pseudocode(changed_ast))
                except Exception as e:
                    print("printing pseudocode failed!")
                    #raise e
                    print(e)

        transformations = [
            InsertMissingBasicBlocks,
            CXXClassToStruct,
            FlattenStructs,
            ReplaceStructDeclStatements,
            UnaryReferenceAndPointerRemover,
            CondExtractor,
            UnaryExtractor,
            UnaryToBinary,
            CallExtractor,
            MoveReturnValueToArguments,
            CompoundToBinary,
            IndicesExtractor,
            InitExtractor,
            ForDeclarer,
        ]

        for transformation in transformations:
            changed_ast = transformation().visit(changed_ast)

        type_validator = ValidateNodeTypes()
        changed_ast = type_validator.visit(changed_ast)
        print(type_validator.report())

        if args.print_ast:
            print("saving ast after transformation to tmp/after.txt")
            with open("tmp/after.txt", "w") as f:
                f.write(dump(changed_ast, include_attributes=True))
            with open("tmp/after.pseudo.cpp", "w") as f:
                try:
                    f.write(c2d_ast_output.get_pseudocode(changed_ast))
                except Exception as e:
                    print("printing pseudocode failed!")
                    print(e)

        print("saving ast after transformation to tmp/after.txt")
        with open("after.txt", "w") as f:
            f.write(dump(changed_ast, include_attributes=True))
        # for node in tu.cursor.get_children():
        # if node.spelling == "InitStressTermsForElems":
        # create_ast_copy(new_AST, node, filename)

        print("Own AST creation done")

        #sdfg = SDFG("_" + filecore + "_inner")
        globalsdfg = SDFG("_" + filecore)
        globalsdfg.add_symbol("_argcount", dace.int32)
        name_mapping = NameMap()
        name_mapping[globalsdfg]["argv_loc"] = "argv_loc"
        name_mapping[globalsdfg]["argc_loc"] = "argc_loc"
        name_mapping[globalsdfg]["c2d_retval"] = "c2d_retval"

        globalsdfg.add_array(name_mapping[globalsdfg]["argv_loc"],
                             ['_argcount'],
                             dace.int8,
                             transient=False)
        globalsdfg.add_scalar(name_mapping[globalsdfg]["argc_loc"],
                              dace.int32,
                              transient=False)
        globalsdfg.add_scalar(name_mapping[globalsdfg]["c2d_retval"],
                              dace.int32,
                              transient=True)
        last_call_expression = [
            DeclRefExpr(name="argc_loc"),
            DeclRefExpr(name="argv_loc"),
            DeclRefExpr(name="c2d_retval")
        ]
        #translator = AST2SDFG(last_call_expression, globalsdfg, "main",
        #                      name_mapping)
        translator = AST2SDFG(last_call_expression, globalsdfg, "main",
                              name_mapping)

        translator.translate(changed_ast, globalsdfg)
        print("done")
        globalsdfg.save("tmp/" + filecore + "-unvalidated.sdfg")
        globalsdfg.validate()
        globalsdfg.save("tmp/" + filecore + ".sdfg")
        globalsdfg = SDFG.from_file("tmp/" + filecore + ".sdfg")

        print("SDFG creation done")

        #code = globalsdfg.compile("tmp/" + filecore + '-raw.cc')
        # list_all_new(new_AST)

        from dace import propagate_memlets_sdfg
        from dace.transformation.interstate import StateFusion, StateAssignElimination, InlineSDFG, LoopToMap, InlineTransients, HoistState, RefineNestedAccess
        from dace.transformation.dataflow import MergeSourceSinkArrays, PruneConnectors, AugAssignToWCR, MapCollapse
        from dace.transformation.transformation import strict_transformations
        from dace.sdfg.utils import fuse_states
        from dace.sdfg.analysis import scalar_to_symbol as scal2sym
        import time

        for node, parent in globalsdfg.all_nodes_recursive():
            if isinstance(node, dace.nodes.NestedSDFG):
                if 'kernel_' in node.sdfg.name:
                    print(
                        f'Hinting that {node.sdfg.name} should not be inlined')
                    node.no_inline = True
                    #node.instrument = dace.InstrumentationType.Timer

        for sd in globalsdfg.all_sdfgs_recursive():
            promoted = scal2sym.promote_scalars_to_symbols(sd)
            print(sd.label, 'promoting', promoted)
        for sd in globalsdfg.all_sdfgs_recursive():
            print(sd.name, sd.symbols)
        print(time.time(), 'sfusion')
        globalsdfg.save("tmp/" + filecore + "-promoted-notfused.sdfg")
        #globalsdfg = SDFG.from_file("tmp/" + filecore +
        #                            "-promoted-notfused.sdfg")
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')

        fuse_states(globalsdfg)

        globalsdfg.save("tmp/" + filecore + "-before_prune.sdfg")
        globalsdfg = SDFG.from_file("tmp/" + filecore + "-before_prune.sdfg")
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')
        print(time.time(), 'sprop')
        globalsdfg.apply_transformations_repeated(PruneConnectors, strict=True)
        #globalsdfg.save("tmp/" + filecore + "-promoted-fused.sdfg")
        #globalsdfg = SDFG.from_file("tmp/" + filecore + "-promoted-fused.sdfg")
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')
        propagate_memlets_sdfg(globalsdfg)

        #globalsdfg.save("tmp/" + filecore + "-promoted-fused.sdfg")
        #globalsdfg = SDFG.from_file("tmp/" + filecore + "-promoted-fused.sdfg")
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')

        #
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-early.cc')
        print(time.time(), 'stricting')
        strict_reduced = [
            xf for xf in strict_transformations()
            if 'Redundant' not in xf.__name__ and 'OutMerge' not in xf.__name__
        ]
        #globalsdfg.apply_transformations_repeated(strict_reduced, strict=True)
        #globalsdfg.save("tmp/" + filecore + "-stricted.sdfg")
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')

        print(time.time(), 'promoting v2')
        for sd in globalsdfg.all_sdfgs_recursive():
            promoted = scal2sym.promote_scalars_to_symbols(sd)
            print(sd.label, 'promoting', promoted)

        #globalsdfg.save("tmp/" + filecore + '-repromoted.sdfg')
        #globalsdfg = dace.SDFG.from_file("tmp/" + filecore +
        #                                 '-repromoted.sdfg')
        #code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')
        for i in range(4):
            print(time.time(), 'sprop')
            propagate_memlets_sdfg(globalsdfg)
            #globalsdfg.save("tmp/" + filecore + '-tmp.sdfg')
            #globalsdfg = dace.SDFG.from_file("tmp/" + filecore + '-tmp.sdfg')
            print(time.time(), 'stricting v2')
            globalsdfg.apply_strict_transformations()
            # xforms = sdfg.apply_transformations_repeated(
            #    [LoopToMap],
            #    strict=True)
            #globalsdfg.save("tmp/" + filecore + '-tmp.sdfg')
            #globalsdfg = dace.SDFG.from_file("tmp/" + filecore + '-tmp.sdfg')
            xforms = globalsdfg.apply_transformations_repeated(
                [HoistState, InlineTransients, AugAssignToWCR] +
                strict_reduced,
                strict=True,
                validate_all=True)
            #globalsdfg.save("tmp/" + filecore + '-tmp.sdfg')
            #globalsdfg = dace.SDFG.from_file("tmp/" + filecore + '-tmp.sdfg')
            #globalsdfg.apply_transformations_repeated(RefineNestedAccess)
            globalsdfg.apply_transformations_repeated(LoopToMap)
            globalsdfg.save("tmp/" + filecore + '-tmp.sdfg')
            globalsdfg = dace.SDFG.from_file("tmp/" + filecore + '-tmp.sdfg')
            if xforms == 0:
                break
        globalsdfg.save("tmp/" + filecore + "-perf.sdfg")
        # code=sdfg.compile()
        #code = globalsdfg.compile("tmp/" + filecore +
        #                          '-dace-auto-new-unopt.cc')
        #print("AUTOOPT")
        #from dace.transformation import auto_optimize as aopt
        #if autoopt:
        #aopt.auto_optimize(globalsdfg, dace.DeviceType.CPU)
        globalsdfg.save("tmp/" + filecore + "-opt.sdfg")
        #globalsdfg = SDFG.from_file("tmp/" + filecore + "-opt.sdfg")
        code = globalsdfg.compile("tmp/" + filecore + '-dace-auto-new.cc')
        #for codeobj in globalsdfg.generate_code():
        #    if codeobj.title == 'Frame':
        #        with open("tmp/" + filecore + '-dace-auto-new.cc', 'w') as fp:
        #            fp.write(codeobj.code)

        import subprocess

        source_file_name = "tmp/" + filecore + fileextension
        compiled_file_name = "tmp/" + filecore + ".out"
        transformed_file_name = "tmp/" + filecore + "-dace.cpp"
        compiled_transformed_file_name = "tmp/" + filecore + "-dace.out"

        #print("compiling " + source_file_name + " ...")
        #code, output = execute_command(
        #    ["g++", source_file_name, "-o", compiled_file_name] +
        #    cfg.clang_cli_arguments)

        #print(output)
