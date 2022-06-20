import re
import argparse
import os

import dace
from dace.sdfg import *
from dace.data import Scalar
from dace.properties import CodeBlock
from regex import W

from c2d_ast2sdfg import *
from c2d_c_ast_transforms import *
from c2d_c_ast_validators import *
from c2d_ast_output import *


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


def c2d_workflow(_dir,
                 filecore,
                 fileextension,
                 autoopt=True,
                 verbose=False,
                 print_ast=False):
    dace.Config.set("compiler", "default_data_types", value="C")

    filename = _dir + "/" + filecore + fileextension

    print("transforming file: " + filename)

    index = clang.cindex.Index.create()
    print("parsing...")
    tu = index.parse(filename)
    if len(tu.diagnostics) > 0:
        print("encountered " + str(len(tu.diagnostics)) +
              Formatting.format_string(" diagnostics", Formatting.YELLOW) +
              " during parsing...")
        if verbose:
            for d in tu.diagnostics:
                print(" " + str(d))
        else:
            print("run the program in verbose mode to print diagnostics.")

    print("copying ast...")
    from os import listdir
    from os.path import isfile, join
    filelist = [f for f in listdir(_dir) if isfile(join(_dir, f))]
    dirfilelist = []
    print("FILELIST:", dirfilelist)
    files = [filename] + dirfilelist

    own_ast = create_own_ast(tu.cursor, files)
    changed_ast = own_ast

    attr_validator = ValidateAllAttributesPresent()
    changed_ast = attr_validator.visit(changed_ast)
    print(attr_validator.report())

    if print_ast:
        print("saving ast after copying to tmp/before.txt")
        with open("tmp/before.txt", "w") as f:
            f.write(dump(changed_ast, include_attributes=True))

        with open("tmp/before.pseudo.cpp", "w") as f:
            try:
                f.write(get_pseudocode(changed_ast))
            except Exception as e:
                print("printing pseudocode failed!")
                #raise e
                print(e)

    transformations = [
        PowerOptimization,
        Calloc2Malloc,
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
        CompoundArgumentsExtractor,
        #ArrayPointerExtractor,
        #ArrayPointerReset,
        #UnaryReferenceAndPointerRemover,
        IndicesExtractor,
        InitExtractor,
        ForDeclarer,
        ParenExprRemover,
        MallocForceInitializer,
    ]

    debug = True
    global_array_map = dict()

    transformation_args = {
        ArrayPointerExtractor: [global_array_map],
        ArrayPointerReset: [global_array_map],
    }

    for transformation in transformations:
        if debug:
            print("="*10)
            print(transformation)
            if transformation == Calloc2Malloc:
                with open("tmp/middle.pseudo.cpp", "w") as f:
                    f.write(get_pseudocode(changed_ast))
            #PrinterVisitor().visit(changed_ast) 
        args = transformation_args.get(transformation, [])
        changed_ast = transformation(*args).visit(changed_ast)

    type_validator = ValidateNodeTypes()
    changed_ast = type_validator.visit(changed_ast)
    print(type_validator.report())

    if print_ast:
        print("saving ast after transformation to tmp/after.txt")
        with open("tmp/after.txt", "w") as f:
            f.write(dump(changed_ast, include_attributes=True))
        with open("tmp/after.pseudo.cpp", "w") as f:
            try:
                f.write(get_pseudocode(changed_ast))
            except Exception as e:
                print("printing pseudocode failed!")
                print(e)

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

    globalsdfg.add_array(name_mapping[globalsdfg]["argv_loc"], ['_argcount'],
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
    translator = AST2SDFG(last_call_expression, globalsdfg, "main",
                          name_mapping)

    translator.translate(changed_ast, globalsdfg)

    print("SDFG creation done")

    from dace import propagate_memlets_sdfg
    from dace.transformation.interstate import StateFusion, StateAssignElimination, InlineSDFG, LoopToMap, InlineTransients, HoistState, RefineNestedAccess
    from dace.transformation.dataflow import MergeSourceSinkArrays, PruneConnectors, AugAssignToWCR, MapCollapse, TrivialMapElimination, GPUTransformMap, GPUTransformLocalStorage
    from dace.sdfg.utils import fuse_states
    from dace.transformation import helpers as xfh
    from dace.sdfg.analysis import scalar_to_symbol as scal2sym
    import time

    for node, parent in globalsdfg.all_nodes_recursive():
        if isinstance(node, dace.nodes.NestedSDFG):
            if 'kernel_' in node.sdfg.name:
                print(f'Hinting that {node.sdfg.name} should not be inlined')
                #node.no_inline = True
                #node.instrument = dace.InstrumentationType.Timer
    globalsdfg.save("tmp/" + filecore + "-untransformed.sdfg")
    globalsdfg.validate()

    for sd in globalsdfg.all_sdfgs_recursive():
        promoted = scal2sym.promote_scalars_to_symbols(sd)
    globalsdfg.save("tmp/" + filecore + "-promoted-notfused.sdfg")
    for codeobj in globalsdfg.generate_code():
        if codeobj.title == 'Frame':
            with open("tmp/middle_code.cc", 'w') as fp:
                fp.write(codeobj.clean_code)

    globalsdfg.compile()
    return

    globalsdfg.simplify()
    globalsdfg.save("tmp/" + filecore + "-simplified.sdfg")
    globalsdfg.apply_transformations_repeated(PruneConnectors)
    xfh.split_interstate_edges(globalsdfg)
    propagate_memlets_sdfg(globalsdfg)

    for sd in globalsdfg.all_sdfgs_recursive():
        promoted = scal2sym.promote_scalars_to_symbols(sd, ignore=set(['c2d_retval']))
        #promoted = scal2sym.promote_scalars_to_symbols(sd)
        print(sd.label, 'promoting', promoted)
    globalsdfg.save("tmp/" + filecore + "-nomap.sdfg")
    xform_types = [
        TrivialMapElimination,
        HoistState,
        #InlineTransients,
        AugAssignToWCR
    ]

    for i in range(4):
        propagate_memlets_sdfg(globalsdfg)
        globalsdfg.simplify()
        xforms = globalsdfg.apply_transformations_repeated(xform_types,
                                                           validate_all=True)

        # Strict transformations and loop parallelization
        transformed = True
        while transformed:
            globalsdfg.apply_transformations_repeated(xform_types)
            for sd in globalsdfg.all_sdfgs_recursive():
                xfh.split_interstate_edges(sd)
            num = globalsdfg.apply_transformations_repeated(RefineNestedAccess)
            print("Refine nested acesses:", num)
            globalsdfg.save("tmp/pre.sdfg")
            l2ms = globalsdfg.apply_transformations_repeated(LoopToMap, validate=False)
            transformed = l2ms > 0

        globalsdfg.apply_transformations_repeated(LoopToMap, validate=False)

        if xforms == 0:
            break
    for sd in globalsdfg.all_sdfgs_recursive():
        sd.apply_transformations_repeated(StateAssignElimination,
                                          validate=False)

    globalsdfg.save("tmp/" + filecore + "-perf.sdfg")
    from dace.transformation.auto import auto_optimize as aopt
    aopt.move_small_arrays_to_stack(globalsdfg)
    aopt.make_transients_persistent(globalsdfg, dace.DeviceType.CPU)
    for sdfg in globalsdfg.all_sdfgs_recursive():
        sdfg.openmp_sections = False
    globalsdfg.save("tmp/" + filecore + "-opt.sdfg")
    for codeobj in globalsdfg.generate_code():
        if codeobj.title == 'Frame':
            with open("tmp/" + filecore + '-dace.cc', 'w') as fp:
                fp.write(codeobj.clean_code)

    globalsdfg.compile()

def shortcut(filecore):
    from dace import propagate_memlets_sdfg
    from dace.transformation.interstate import StateFusion, StateAssignElimination, InlineSDFG, LoopToMap, InlineTransients, HoistState, RefineNestedAccess
    from dace.transformation.dataflow import MergeSourceSinkArrays, PruneConnectors, AugAssignToWCR, MapCollapse, TrivialMapElimination, GPUTransformMap, GPUTransformLocalStorage
    from dace.sdfg.utils import fuse_states
    from dace.transformation import helpers as xfh
    from dace.sdfg.analysis import scalar_to_symbol as scal2sym
    import time

    globalsdfg = SDFG("_" + filecore).from_file("tmp/" + filecore + "-promoted-notfused.sdfg")
    globalsdfg.simplify()
    globalsdfg.apply_transformations_repeated(PruneConnectors)
    xfh.split_interstate_edges(globalsdfg)
    propagate_memlets_sdfg(globalsdfg)

    for sd in globalsdfg.all_sdfgs_recursive():
        promoted = scal2sym.promote_scalars_to_symbols(sd)
        print(sd.label, 'promoting', promoted)
    globalsdfg.save("tmp/" + filecore + "-nomap.sdfg")
    xform_types = [
        TrivialMapElimination, HoistState, InlineTransients, AugAssignToWCR
    ]
    for i in range(4):
        propagate_memlets_sdfg(globalsdfg)
        globalsdfg.simplify()
        xforms = globalsdfg.apply_transformations_repeated(xform_types,
                                                           validate_all=True)

        # Strict transformations and loop parallelization
        transformed = True
        while transformed:
            globalsdfg.apply_transformations_repeated(xform_types)
            for sd in globalsdfg.all_sdfgs_recursive():
                xfh.split_interstate_edges(sd)
            num = globalsdfg.apply_transformations_repeated(RefineNestedAccess)
            print("Refine nested acesses:", num)
            l2ms = globalsdfg.apply_transformations_repeated(LoopToMap,
                                                             validate=False)
            transformed = l2ms > 0

        globalsdfg.apply_transformations_repeated(LoopToMap, validate=False)

        if xforms == 0:
            break
    for sd in globalsdfg.all_sdfgs_recursive():
        sd.apply_transformations_repeated(StateAssignElimination,
                                          validate=False)

    globalsdfg.save("tmp/" + filecore + "-perf.sdfg")
    from dace.transformation.auto import auto_optimize as aopt
    aopt.move_small_arrays_to_stack(globalsdfg)
    aopt.make_transients_persistent(globalsdfg, dace.DeviceType.CPU)
    for sdfg in globalsdfg.all_sdfgs_recursive():
        sdfg.openmp_sections = False
    globalsdfg.save("tmp/" + filecore + "-opt.sdfg")
    for codeobj in globalsdfg.generate_code():
        if codeobj.title == 'Frame':
            with open("tmp/" + filecore + '-dace.cc', 'w') as fp:
                fp.write(codeobj.clean_code)

    globalsdfg.compile()