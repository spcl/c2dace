from operator import ne
from typing import List, Tuple, Set
import clang.cindex
from clang.cindex import Cursor
import copy
import dace
from dace.sdfg.nodes import Tasklet
from dace.data import Scalar
from dace.properties import CodeBlock
from dace.sdfg import *
#import config as cfg

from c2d_c_ast_transforms import *

#clang.cindex.Config.set_library_file(cfg.clang_library_file)


class NameMap(dict):
    def __getitem__(self, k):
        assert isinstance(k, SDFG)
        if k not in self:
            self[k] = {}

        return super().__getitem__(k)

    def get(self, k):
        return self[k]

    def __setitem__(self, k, v) -> None:
        assert isinstance(k, SDFG)
        return super().__setitem__(k, v)


class Operand:
    def __init__(self, name, sdfg: SDFG, dictionary):
        self.name = name
        self.indices = []
        self.within_index = False
        self.inout = "unknown"
        self.disambiguated_name = name
        self.temp_name = name
        self.sdfg_name = sdfg._find_new_name(name)
        self.context_change_name = name
        self.disambiguated = False


class Components:
    outputs: List[Operand]
    inputs: List[Operand]
    code: List[str]

    def __init__(self):
        self.outputs = []
        self.inputs = []
        self.code = []


def raise_exception(error_message):
    print(error_message)
    raise UserWarning(error_message)


def get_var_name(node):
    if isinstance(node, ArraySubscriptExpr):
        return get_var_name(node.unprocessed_name)

    if isinstance(node, ParenExpr):
        return get_var_name(node.expr)

    if isinstance(node, DeclRefExpr):
        return node.name
    else:
        print("WARNING cannot find name of ", node)


def make_nested_sdfg_with_no_context_change(top_sdfg: Cursor, new_sdfg: Cursor,
                                            name: str, used_variables, node,
                                            state):
    local_variables_new_sdfg_names = []
    local_variables_names = []
    substate = top_sdfg.add_state("state" + name)
    used_vars = [
        node for node in walk(node.body) if isinstance(node, DeclRefExpr)
    ]
    binop_nodes = [node for node in walk(node.body) if isinstance(node, BinOp)]
    write_nodes = [node for node in binop_nodes if node.op == "="]
    call_nodes = [node.args for node in walk(node.body) if isinstance(node, CallExpr)]
    call_nodes = sum(call_nodes, [])
    call_nodes = filter(lambda x: hasattr(x, "type"), call_nodes)
    call_nodes = filter(lambda x: isinstance(x.type, Pointer), call_nodes)

    write_vars = [node.lvalue for node in write_nodes] + list(copy.deepcopy(call_nodes))
    read_vars = copy.deepcopy(used_vars) + list(call_nodes)
    for i in write_vars:
        if i in read_vars:
            read_vars.remove(i)
        #print(write_vars)
        #print(read_vars)
    write_vars = remove_duplicates(write_vars)
    read_vars = remove_duplicates(read_vars)
    used_vars = remove_duplicates(used_vars)
    write_names = []
    read_names = []

    for i in write_vars:
        if state.name_mapping.get(top_sdfg).get(get_var_name(i)) in top_sdfg.arrays:
            write_names.append(get_var_name(i))
    for i in read_vars:
        if state.name_mapping.get(top_sdfg).get(get_var_name(i)) in top_sdfg.arrays:
            read_names.append(get_var_name(i))

    for i in top_sdfg.arrays:
        found = False
        for j in used_variables:
            # print("USED:",j.name)
            if state.name_mapping.get(top_sdfg).get(j.name) == i:
                # print(i,j.name)
                found = True
                global_value = False

                break
        if not found:
            for j in state.globalsdfg.arrays:
                if state.name_mapping.get(top_sdfg).get(j) == i:
                    found = True
                    global_value = True
                    break
        if not found:
            # print("Skipping:", i, j.name, )
            continue
        else:
            # print("NOT Skipping:", i, j.name, )
            if global_value:
                j = DeclRefExpr(name=j)
            state.name_mapping[new_sdfg][j.name] = find_new_array_name(
                state.all_array_names, j.name)
            state.all_array_names.append(state.name_mapping[new_sdfg][j.name])
            m = top_sdfg.arrays.get(i)
            if m.total_size == 1:
                new_sdfg.add_scalar(state.name_mapping[new_sdfg][j.name],
                                    m.dtype, m.storage, False)
            else:
                new_sdfg.add_array(state.name_mapping[new_sdfg][j.name],
                                   shape=m.shape,
                                   dtype=m.dtype,
                                   transient=False,
                                   strides=m.strides,
                                   offset=m.offset)
            local_variables_new_sdfg_names.append(
                state.name_mapping[new_sdfg][j.name])
            local_variables_names.append(j.name)
            if j.name in state.libstates:
                if j.name not in write_names:
                    write_names.append(j.name)
                if j.name not in read_names:
                    read_names.append(j.name)

    sym_dict = {}
    for i in top_sdfg.symbols:
        sym_dict[i] = i

    # for i in top_sdfg.arrays:
    # m = top_sdfg.arrays[i]
    # print("ARS no context change:", i, m.shape, m.total_size)
    # print(sym_dict)
    new_in_names = []
    new_out_names = []
    for i in write_names:
        new_out_names.append(state.name_mapping[new_sdfg][i])
    for i in read_names:
        new_in_names.append(state.name_mapping[new_sdfg][i])
    internal_sdfg = substate.add_nested_sdfg(new_sdfg,
                                             top_sdfg,
                                             new_in_names,
                                             new_out_names,
                                             symbol_mapping=sym_dict)
    for i in local_variables_names:
        shape = top_sdfg.arrays[state.name_mapping[top_sdfg][i]].shape
        memlet = "0"
        done = False
        first = True
        if len(shape) == 1:
            if shape[0] == 1:
                done = True
        if isinstance(top_sdfg.arrays[state.name_mapping[top_sdfg][i]],
                      Scalar):
            memlet = ""
            #print("MEMLET fixes:",i)
            done = True

        start_name = state.arr_start_name_mapping.get(i)
        if not start_name:
            start_name = "0"

        if not done:
            for k in shape:
                if first:
                    memlet = start_name + ":" + str(k)
                    first = False
                else:
                    memlet = memlet + ",0:" + str(k)
        if i in read_names:
            add_memlet_read(substate, state.name_mapping[top_sdfg][i],
                            internal_sdfg, state.name_mapping[new_sdfg][i],
                            memlet)
        if i in write_names:
            add_memlet_write(substate, state.name_mapping[top_sdfg][i],
                             internal_sdfg, state.name_mapping[new_sdfg][i],
                             memlet)

    return substate


def add_tasklet(substate: SDFGState, name: str, vars_in: Set[str],
                vars_out: Set[str], code: str):
    tasklet = substate.add_tasklet(name="T" + name,
                                   inputs=vars_in,
                                   outputs=vars_out,
                                   code=code,
                                   language=dace.Language.CPP)
    return tasklet


def add_memlet_read(substate: SDFGState, var_name: str, tasklet: Tasklet,
                    dest_conn: str, memlet_range: str):
    src = substate.add_access(var_name)
    if memlet_range != "":
        substate.add_memlet_path(src,
                                 tasklet,
                                 dst_conn=dest_conn,
                                 memlet=dace.Memlet(expr=var_name,
                                                    subset=memlet_range))
    else:
        substate.add_memlet_path(src,
                                 tasklet,
                                 dst_conn=dest_conn,
                                 memlet=dace.Memlet(expr=var_name))


def add_memlet_write(substate: SDFGState, var_name: str, tasklet: Tasklet,
                     source_conn: str, memlet_range: str):
    dst = substate.add_write(var_name)
    if memlet_range != "":
        substate.add_memlet_path(tasklet,
                                 dst,
                                 src_conn=source_conn,
                                 memlet=dace.Memlet(expr=var_name,
                                                    subset=memlet_range))
    else:
        substate.add_memlet_path(tasklet,
                                 dst,
                                 src_conn=source_conn,
                                 memlet=dace.Memlet(expr=var_name))


def add_simple_state_to_sdfg(state: SDFGState, top_sdfg: SDFG,
                             state_name: str):
    if state.last_sdfg_states.get(top_sdfg) is not None:
        substate = top_sdfg.add_state(state_name)
    else:
        substate = top_sdfg.add_state(state_name, is_start_state=True)
    finish_add_state_to_sdfg(state, top_sdfg, substate)
    return substate


def finish_add_state_to_sdfg(state: SDFGState, top_sdfg: SDFG,
                             substate: SDFGState):
    if state.last_sdfg_states.get(top_sdfg) is not None:
        top_sdfg.add_edge(state.last_sdfg_states[top_sdfg], substate,
                          dace.InterstateEdge())
    state.last_sdfg_states[top_sdfg] = substate


def remove_duplicates(vars: List[DeclRefExpr]):
    i = 0
    while i < len(vars):
        if isinstance(vars[i], ParenExpr):
            vars[i] = vars[i].expr
        elif isinstance(vars[i], DeclRefExpr):
            i += 1
        elif isinstance(vars[i], ArraySubscriptExpr):
            i += 1
        else:
            print("WARNING (remove_duplicates) - UNKNOWN EXPRESSION TYPE:", vars[i])
            i += 1

    new_set = []
    for i in vars:
        not_found = True
        for j in new_set:
            if get_var_name(i) == get_var_name(j):
                not_found = False
                break

        if not_found:
            new_set.append(i)
    return new_set


def match_names(sdfgname: str, cname: str):
    regex_to_match = "_*dace_" + cname
    # print(sdfgname,cname)
    if (re.fullmatch(regex_to_match, sdfgname)) is not None:
        # print("match")
        return True
    else:
        # print("NOT match")
        return False


class TaskletWriter:
    def __init__(self, outputs: List[str], outputs_changes: List[str], name_mapping=None):
        self.outputs = outputs
        self.outputs_changes = outputs_changes
        self.name_mapping = name_mapping
        self.ast2ctypes = {
            Double: "double",
            Float: "float",
            Int: "int",
            Char: "char",
            Long: "long",
            LongLong: "long long"
        }
        self.ast_elements = {
            BinOp: self.binop2string,
            DeclRefExpr: self.declref2string,
            ArraySubscriptExpr: self.arraysub2string,
            IntLiteral: self.intlit2string,
            CharLiteral: self.charlit2string,
            FloatLiteral: self.floatlit2string,
            StringLiteral: self.stringlit2string,
            BoolLiteral: self.boollit2string,
            UnOp: self.unop2string,
            CxxCastExpr: self.cxxcast2string,
            TypeRef: self.typeref2string,
            ParenExpr: self.parenexpr2string,
            CCastExpr: self.ccast2string,
            CallExpr: self.call2string,
            InitListExpr: self.initlistexpr2string,
        }

    def intlit2string(self, node: IntLiteral):

        return "".join(map(str, node.value))

    def boollit2string(self, node: BoolLiteral):

        return "".join(map(str, node.value))

    def charlit2string(self, node: CharLiteral):

        return "".join(map(str, node.value))

    def stringlit2string(self, node: StringLiteral):

        return "".join(map(str, node.value))

    def floatlit2string(self, node: FloatLiteral):

        return "".join(map(str, node.value))

    def initlistexpr2string(self, node: InitListExpr):
        return "{%s}" % ", ".join(
            [self.write_tasklet_code(e) for e in node.body])

    def binop2string(self, node: BinOp):
        #print("BL: ",self.write_tasklet_code(node.lvalue))
        #print("RL: ",self.write_tasklet_code(node.rvalue))
        # print(node.op)
        op = node.op
        if op == "&&":
            op=" and "

        if op == "||":
            op=" or "
        # if self.write_tasklet_code(node.lvalue) is None:
        #    a=1
        # if self.write_tasklet_code(node.rvalue) is None:
        #    a=1
        return self.write_tasklet_code(
            node.lvalue) + op + self.write_tasklet_code(node.rvalue)

    def declref2string(self, node: DeclRefExpr):
        return_value = node.name

        if self.name_mapping is not None:
            if self.name_mapping.get(node.name) is not None:
                return_value = self.name_mapping[node.name]

        if len(self.outputs) > 0:
            #print("TASK WRITER:",node.name,self.outputs[0],self.outputs_changes[0])
            if node.name == self.outputs[0]:
                if self.outputs[0] != self.outputs_changes[0]:
                    return_value = self.outputs_changes[0]
                self.outputs.pop(0)
                self.outputs_changes.pop(0)
            #print("RETURN VALUE:",return_value)
        return return_value

    def parenexpr2string(self, node: ParenExpr):
        return "(" + self.write_tasklet_code(node.expr) + ")"

    def call2string(self, node: CallExpr):
        # print("NAME:",node.name)
        #print("NAME:", self.write_tasklet_code(node.name) )

        if isinstance(node.name, Node):
            retstring = self.write_tasklet_code(node.name) + "("
        else:
            retstring = node.name + "("

        first = True
        for i in node.args:
            if first:
                retstring += self.write_tasklet_code(i)
                first = False
            else:
                retstring += "," + self.write_tasklet_code(i)
        retstring += ")"
        return retstring

    def arraysub2string(self, node: ArraySubscriptExpr):
        return self.write_tasklet_code(
            node.unprocessed_name) + "[" + self.write_tasklet_code(
                node.index) + "]"

    def unop2string(self, node: UnOp):
        return node.op + self.write_tasklet_code(node.lvalue)

    def cxxcast2string(self, node: CxxCastExpr):
        return "(" + self.ast2ctypes[
            node.type.__class__] + ")(" + self.write_tasklet_code(
                node.expr) + ")"

    def ccast2string(self, node: CCastExpr):
        # print("CCAST: ","("+self.write_tasklet_code(node.expr)+")")
        if isinstance(node.type, Pointer):
            (depth, var_type) = node.type.get_pointer_chain_end()
            pointer_chain = ""
            for _ in range(0, depth):
                pointer_chain = pointer_chain + "*"
                if isinstance(var_type, ConstantArray):
                    size = var_type.size
                    var_type = var_type.element_type
                    return "(" + self.ast2ctypes[
                        var_type.__class__] + "(" + pointer_chain + ")[" + str(
                            size) + "])(" + self.write_tasklet_code(
                                node.expr) + ")"
                return "(" + self.ast2ctypes[
                    var_type.
                    __class__] + "(" + pointer_chain + "))(" + self.write_tasklet_code(
                        node.expr) + ")"
        return "(" + self.ast2ctypes[
            node.type.__class__] + ")(" + self.write_tasklet_code(
                node.expr) + ")"

    def typeref2string(self, node: TypeRef):
        return node.name

    def write_tasklet_code(self, node: Node):
        if node.__class__ in self.ast_elements:
            text = self.ast_elements[node.__class__](node)
            #print("RET TW:",text)
            return text
        else:

            print("ERROR:", node.__class__.__name__)


class ConditionWriter(TaskletWriter):
    def __init__(self, sdfg: SDFG, mapping):
        self.sdfg = sdfg
        self.mapping = mapping
        self.ast2ctypes = {
            Double: "double",
            Float: "float",
            Int: "int",
            Char: "char",
        }
        self.ast_elements = {
            BinOp: self.binop2string,
            DeclRefExpr: self.declref2string,
            ArraySubscriptExpr: self.arraysub2string,
            IntLiteral: self.intlit2string,
            CharLiteral: self.charlit2string,
            FloatLiteral: self.floatlit2string,
            StringLiteral: self.stringlit2string,
            BoolLiteral: self.boollit2string,
            UnOp: self.unop2string,
            CxxCastExpr: self.cxxcast2string,
            TypeRef: self.typeref2string,
            ParenExpr: self.parenexpr2string,
            CCastExpr: self.ccast2string,
            CallExpr: self.call2string,
            InitListExpr: self.initlistexpr2string,
        }

    def declref2string(self, node: DeclRefExpr):
        name = node.name
        # print("CW:",name)
        for i in self.sdfg.arrays:
            sdfg_name = self.mapping.get(self.sdfg).get(name)
            if sdfg_name == i:
                name = i
                break
        return name


def find_new_array_name(arrays, name: str):
    """ Tries to find a new name by adding an underscore and a number. """
    index = 0
    while "dace_" + name + ('_%d' % index) in arrays:
        index += 1

    return "dace_" + name + ('_%d' % index)


def generate_memlet(op, top_sdfg, state, offset='0'):
    if state.name_mapping.get(top_sdfg).get(op.name) is not None:
        shape = top_sdfg.arrays[state.name_mapping[top_sdfg][op.name]].shape
    elif state.name_mapping.get(state.globalsdfg).get(op.name) is not None:
        shape = state.globalsdfg.arrays[state.name_mapping[state.globalsdfg][
            op.name]].shape
    else:
        raise NameError("Variable name not found: ", op.name)
    #print("SHAPE: ", shape)
    tmp_node = op
    indices = []
    while isinstance(tmp_node, ArraySubscriptExpr):
        if isinstance(tmp_node.index, DeclRefExpr):
            indices.append(state.name_mapping[top_sdfg][tmp_node.index.name])
        elif isinstance(tmp_node.index, IntLiteral):
            indices.append("".join(map(str, tmp_node.index.value)))
        tmp_node = tmp_node.unprocessed_name
    # for i in indices:
    # print("INDICES:",i)

    memlet = '0'
    count = 0
    first = True
    if len(shape) == 1:
        if shape[0] == 1:
            return memlet
    for i in shape:
        if len(indices) > count:
            # print(indices[count])
            if first:
                memlet = indices[count]
                first = False
            else:
                memlet = memlet + ',' + indices[count]
            count = count + 1
        else:
            if first:
                if i != 1:
                    memlet = offset + ':' + str(i)
                else:
                    memlet = '0'
                first = False
            else:
                if i != 1:
                    memlet = memlet + ',0:' + str(i)
                else:
                    memlet = memlet + ',0'

    return memlet


class AST2SDFG:
    def __init__(
            self,
            last_call_expression: List[Operand],
            #sdfg: SDFG,
            globalsdfg: SDFG,
            start_function="main",
            name_mapping=None):
        self.start_function = start_function
        self.last_sdfg_states = {}
        self.loop_depth = -1
        self.ast = Node()
        self.globalsdfg = globalsdfg
        self.function_depth = -1
        self.function_names = {}
        self.typedefs = {}
        self.libraries = {}
        self.libraries["printf"] = "print"
        self.libraries["fprintf"] = "print"
        self.libstates = ["print"]
        self.incomplete_arrays = {}
        self.name_mapping = name_mapping or NameMap()
        self.arr_start_name_mapping = {}
        self.last_loop_continue = {}
        self.last_loop_break = {}
        self.last_loop_continue_state = {}
        self.last_loop_break_state = {}
        self.last_return_state = {}
        self.last_function_state = {}
        self.last_call_expression = {globalsdfg: last_call_expression}
        self.tasklet_count = 0
        self.all_array_names = []
        self.globalsdfg.add_scalar("print", dace.int32, transient=True)
        substate = add_simple_state_to_sdfg(self, self.globalsdfg, "AllState")
        inouts_in_new_sdfg = []
        for i in self.globalsdfg.arrays:
            if i in self.libstates:
                self.name_mapping[globalsdfg][i] = i

        for i in globalsdfg.arrays:
            self.all_array_names.append(i)
        self.ast_elements = {
            WhileStmt: self.while2sdfg,
            DoStmt: self.do2sdfg,
            RetStmt: self.ret2sdfg,
            IfStmt: self.ifstmt2sdfg,
            ForStmt: self.forstmt2sdfg,
            BasicBlock: self.basicblock2sdfg,
            FuncDecl: self.funcdecl2sdfg,
            BinOp: self.binop2sdfg,
            DeclStmt: self.declstmt2sdfg,
            VarDecl: self.vardecl2sdfg,
            ParmDecl: self.parmdecl2sdfg,
            TypeDecl: self.typedecl2sdfg,
            CallExpr: self.call2sdfg,
            BreakStmt: self.break2sdfg,
            ContinueStmt: self.continue2sdfg,
            AST: self.tu2sdfg
        }
        self.ctypes2dacetypes = {
            Double: dace.float64,
            Float: dace.float32,
            Int: dace.int32,
            Char: dace.int8,
            Long: dace.int32,
            LongLong: dace.int64,
            ULongLong: dace.uint64,
            Bool: dace.int8,
        }

    def translate(self, node: Node, sdfg: SDFG):
        if node.__class__ in self.ast_elements:
            #print('-'*10)
            #print(self.ast_elements[node.__class__])
            self.ast_elements[node.__class__](node, sdfg)
        else:
            print("WARNING:", node.__class__.__name__)
            # raise TypeError("Unsupported ast node type - will be implemented")

    def tu2sdfg(self, node: AST, sdfg: SDFG):
        for i in node.typedefs:
            self.typedecl2sdfg(i)

        self.ast = node
        for i in node.vardefs:
            self.vardecl2sdfg(i, sdfg)
        for i in node.decldefs:
            self.declstmt2sdfg(i, sdfg)
        for i in node.funcdefs:
            if i.name == self.start_function:
                self.funcdecl2sdfg(i, sdfg)

    def get_dace_type(self, type: Type):
        assert isinstance(type, Type)
        if isinstance(type, Array):
            return self.get_dace_type(type.element_type)

        if isinstance(type, Pointer):
            return self.get_dace_type(type.pointee_type)

        return self.ctypes2dacetypes[type.__class__]

    def typedecl2sdfg(self, node: TypeDecl):
        if hasattr(node, "type"):
            if isinstance(node.type, str) and node.type[0] != "struct":
                # print("TDPRE:",node.type)
                #print("TD: ",node.name," ",self.get_dace_type(node.type))
                self.typedefs[node.name] = self.get_dace_type(node.type)
            elif hasattr(node, "typeref"):
                self.typedefs[node.name] = self.typedefs[node.typeref[0].name]
            else:
                self.typedefs[node.name] = self.get_dace_type(node.type)
                # print("TD: ", node.name, " ", self.typedefs[node.typeref[0].name])

    def strip(self, node: Node):
        if isinstance(node, UnOp):
            if node.op == "&":
                return self.strip(node.lvalue)
            if node.op == "*":
                return self.strip(node.lvalue)
            if node.op == "-":
                return self.strip(node.lvalue)
            else:
                raise TypeError("Should be &")
        elif isinstance(node, ParenExpr):
            return self.strip(node.expr)
        else:
            return node

    def ret2sdfg(self, node: RetStmt, sdfg: SDFG):
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1
        return_state = add_simple_state_to_sdfg(self, sdfg,
                                                "ReturnState" + str(line))
        self.last_return_state[sdfg] = return_state
        sdfg.add_edge(return_state, self.last_function_state[sdfg],
                      dace.InterstateEdge())

    def break2sdfg(self, node: BreakStmt, sdfg: SDFG):
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1
        break_state = add_simple_state_to_sdfg(self, sdfg,
                                               "BreakState" + str(line))
        self.last_loop_break_state[sdfg] = break_state
        sdfg.add_edge(break_state, self.last_loop_break[sdfg],
                      dace.InterstateEdge())

    def continue2sdfg(self, node: ContinueStmt, sdfg: SDFG):
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1
        continue_state = add_simple_state_to_sdfg(self, sdfg,
                                                  "ContinueState" + str(line))
        self.last_loop_continue_state[sdfg] = continue_state
        sdfg.add_edge(continue_state, self.last_loop_continue[sdfg],
                      dace.InterstateEdge())

    def funcdecl2sdfg(self, node: FuncDecl, sdfg: SDFG):
        print("FUNC: ", node.name)

        if node.body is None:
            print("Empty function")
            return

        used_vars = [
            node for node in walk(node.body) if isinstance(node, DeclRefExpr)
        ]
        binop_nodes = [
            node for node in walk(node.body) if isinstance(node, BinOp)
        ]
        call_nodes = [node.args for node in walk(node.body) if isinstance(node, CallExpr)]
        call_nodes = sum(call_nodes, [])
        call_nodes = filter(lambda x: hasattr(x, "type"), call_nodes)
        call_nodes = filter(lambda x: isinstance(x.type, Pointer), call_nodes)

        write_nodes = [node for node in binop_nodes if node.op == "="]
        write_vars = [] + list(copy.deepcopy(call_nodes))
        for n in write_nodes:
            tmp = n.lvalue
            while isinstance(tmp, ParenExpr):
                tmp = tmp.expr

            write_vars.append(tmp)

        read_vars = copy.deepcopy(used_vars) + list(call_nodes)
        for i in write_vars:
            if i in read_vars:
                read_vars.remove(i)
        write_vars = remove_duplicates(write_vars)
        read_vars = remove_duplicates(read_vars)
        used_vars = remove_duplicates(used_vars)
        write_names = []
        read_names = []

        for i in write_vars:
            write_names.append(get_var_name(i))
        for i in read_vars:
            read_names.append(get_var_name(i))

        parameters = node.args.copy()
        new_sdfg = dace.SDFG(node.name)
        substate = add_simple_state_to_sdfg(self, sdfg, "state" + node.name)
        variables_in_call = []
        if self.last_call_expression.get(sdfg) is not None:
            variables_in_call = self.last_call_expression[sdfg]

        inouts_in_new_sdfg = []
        ins_in_new_sdfg = []
        outs_in_new_sdfg = []
        var2 = []
        literals = []
        literal_values = []
        par2 = []
        symbol_arguments = []

        for arg_i, variable in enumerate(variables_in_call):
            variable = self.strip(variable)

            if isinstance(variable, Literal) or variable.name == "LITERAL":
                literals.append(parameters[arg_i])
                literal_values.append(variable)
                continue
            elif variable.name in sdfg.symbols:
                symbol_arguments.append((parameters[arg_i], variable))
                continue

            par2.append(parameters[arg_i])
            var2.append(variable)

        variables_in_call = var2
        parameters = par2
        assigns = []
        for lit, litval in zip(literals, literal_values):
            local_name = lit
            self.translate(local_name, new_sdfg)
            assigns.append(
                BinOp(lvalue=DeclRefExpr(name=local_name.name),
                      rvalue=litval,
                      op="="))

        for parameter, symbol in symbol_arguments:
            self.translate(parameter, new_sdfg)
            assigns.append(
                BinOp(lvalue=DeclRefExpr(name=parameter.name),
                      rvalue=DeclRefExpr(name=symbol.name),
                      op="="))

        for i, variable_in_call in enumerate(variables_in_call):
            all_arrays = self.get_arrays_in_context(sdfg)

            sdfg_name = self.name_mapping.get(sdfg).get(variable_in_call.name)
            globalsdfg_name = self.name_mapping.get(self.globalsdfg).get(
                variable_in_call.name)
            matched = False
            for array_name, array in all_arrays.items():
                if array_name in [sdfg_name]:
                    matched = True
                    local_name = parameters[i]
                    self.name_mapping[new_sdfg][
                        local_name.name] = find_new_array_name(
                            self.all_array_names, local_name.name)
                    self.all_array_names.append(
                        self.name_mapping[new_sdfg][local_name.name])

                    if local_name.name in read_names:
                        ins_in_new_sdfg.append(
                            self.name_mapping[new_sdfg][local_name.name])
                    if local_name.name in write_names:
                        outs_in_new_sdfg.append(
                            self.name_mapping[new_sdfg][local_name.name])

                    inouts_in_new_sdfg.append(
                        self.name_mapping[new_sdfg][local_name.name])

                    indices = 0
                    tmp_node = variable_in_call
                    while isinstance(tmp_node, ArraySubscriptExpr):
                        indices += 1
                        tmp_node = tmp_node.unprocessed_name

                    shape = array.shape[indices:]

                    if local_name.name in read_names or local_name.name in write_names:
                        if shape == () or shape == (1, ):
                            new_sdfg.add_scalar(
                                self.name_mapping[new_sdfg][local_name.name],
                                array.dtype, array.storage, False)
                        else:
                            new_sdfg.add_array(
                                self.name_mapping[new_sdfg][local_name.name],
                                shape=shape,
                                dtype=array.dtype,
                                transient=False)
            if not matched:
                for array_name, array in all_arrays.items():
                    if array_name in [globalsdfg_name]:
                        local_name = parameters[i]
                        self.name_mapping[new_sdfg][
                            local_name.name] = find_new_array_name(
                                self.all_array_names, local_name.name)
                        self.all_array_names.append(
                            self.name_mapping[new_sdfg][local_name.name])

                        inouts_in_new_sdfg.append(
                            self.name_mapping[new_sdfg][local_name.name])

                        if local_name.name in read_names:
                            ins_in_new_sdfg.append(
                                self.name_mapping[new_sdfg][local_name.name])
                        if local_name.name in write_names:
                            outs_in_new_sdfg.append(
                                self.name_mapping[new_sdfg][local_name.name])

                        indices = 0
                        tmp_node = variable_in_call
                        while isinstance(tmp_node, ArraySubscriptExpr):
                            indices += 1
                            tmp_node = tmp_node.unprocessed_name

                        shape = array.shape[indices:]
                        if local_name.name in read_names or local_name.name in write_names:
                            if shape == () or shape == (1, ):
                                new_sdfg.add_scalar(
                                    self.name_mapping[new_sdfg][
                                        local_name.name], array.dtype,
                                    array.storage, False)
                            else:
                                new_sdfg.add_array(
                                    self.name_mapping[new_sdfg][
                                        local_name.name], shape, array.dtype,
                                    array.storage, False)
        sym_dict = {}
        for i in sdfg.symbols:
            sym_dict[i] = i

        for i in self.libstates:
            self.name_mapping[new_sdfg][i] = find_new_array_name(
                self.all_array_names, i)
            self.all_array_names.append(self.name_mapping[new_sdfg][i])
            inouts_in_new_sdfg.append(self.name_mapping[new_sdfg][i])
            ins_in_new_sdfg.append(self.name_mapping[new_sdfg][i])
            outs_in_new_sdfg.append(self.name_mapping[new_sdfg][i])
            new_sdfg.add_scalar(self.name_mapping[new_sdfg][i],
                                dace.int32,
                                transient=False)

        internal_sdfg = substate.add_nested_sdfg(new_sdfg,
                                                 sdfg,
                                                 ins_in_new_sdfg,
                                                 outs_in_new_sdfg,
                                                 symbol_mapping=sym_dict)

        for i in self.libstates:
            memlet = "0"
            add_memlet_write(substate, self.name_mapping[sdfg][i],
                             internal_sdfg, self.name_mapping[new_sdfg][i],
                             memlet)
            add_memlet_read(substate, self.name_mapping[sdfg][i],
                            internal_sdfg, self.name_mapping[new_sdfg][i],
                            memlet)

        for c, i in enumerate(variables_in_call):

            local_name = parameters[c]
            if self.name_mapping.get(sdfg).get(i.name) is not None:
                var = sdfg.arrays.get(self.name_mapping[sdfg][i.name])
                mapped_name = self.name_mapping[sdfg][i.name]
            elif i.name in sdfg.symbols:
                var = i.name
                mapped_name = i.name
            elif self.name_mapping.get(self.globalsdfg).get(
                    i.name) is not None:
                var = self.globalsdfg.arrays.get(
                    self.name_mapping[self.globalsdfg][i.name])
                mapped_name = self.name_mapping[self.globalsdfg][i.name]
            else:
                print(i)
                raise NameError("Variable name not found: " + i.name)

            if not hasattr(var, "shape") or len(var.shape) == 0:
                memlet = ""
            elif (len(var.shape) == 1 and var.shape[0] == 1):
                memlet = "0"
            else:
                memlet = generate_memlet(i, sdfg, self)
            if local_name.name in write_names:
                add_memlet_write(substate, mapped_name, internal_sdfg,
                                 self.name_mapping[new_sdfg][local_name.name],
                                 memlet)
            if local_name.name in read_names:
                add_memlet_read(substate, mapped_name, internal_sdfg,
                                self.name_mapping[new_sdfg][local_name.name],
                                memlet)

        start_state = new_sdfg.add_state("Start_State_Function" + node.name,
                                         is_start_state=True)
        self.last_sdfg_states[new_sdfg] = start_state

        for i in assigns:
            self.translate(i, new_sdfg)

        final_state = new_sdfg.add_state("Final_State_Function" + node.name)
        self.last_function_state[new_sdfg] = final_state

        if node.body is not None:
            self.translate(node.body, new_sdfg)
            if (self.last_sdfg_states[new_sdfg],
                    final_state) not in new_sdfg._edges:
                new_sdfg.add_edge(self.last_sdfg_states[new_sdfg], final_state,
                                  dace.InterstateEdge())

        print("ENDF: ", node.name)

    def basicblock2sdfg(self, node: BasicBlock, sdfg: SDFG):
        #print("BASIC BLOCK")
        call_expressions = [n for n in walk(node) if isinstance(n, CallExpr)]
        #for i in call_expressions:
        #    print(i.name)
        for i in node.body:
            self.translate(i, sdfg)

    def ifstmt2sdfg(self, node: IfStmt, sdfg: SDFG):
        # print("LEN:",len(node.body_if))
        used_vars = [
            node for node in walk(node.body_if[0])
            if isinstance(node, DeclRefExpr)
        ]
        # for j in used_vars:
        #    print("USEDDUPLIC:",j.name)
        used_vars = remove_duplicates(used_vars)
        # for j in used_vars:
        #    print("USED:",j.name)
        # for i in node.__dict__:
        #    print(i)
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1

        name = "If" + str(line)
        new_sdfg = dace.SDFG(name)
        begin_state = add_simple_state_to_sdfg(self, sdfg,
                                               "BeginState" + str(line))
        guard_substate = sdfg.add_state("GuardState" + str(line))
        sdfg.add_edge(begin_state, guard_substate, dace.InterstateEdge())

        condition = ConditionWriter(
            sdfg, self.name_mapping).write_tasklet_code(node.cond[0])
        body_ifstart_state = sdfg.add_state("BodyIfStartState" + str(line))
        self.last_sdfg_states[sdfg] = body_ifstart_state
        self.translate(node.body_if[0], sdfg)

        #if_state = make_nested_sdfg_with_no_context_change(
        #    sdfg, new_sdfg, name, used_vars, self)
        #self.translate(node.body_if[0], new_sdfg)
        final_substate = sdfg.add_state("MergeState" + str(line))

        sdfg.add_edge(guard_substate, body_ifstart_state,
                      dace.InterstateEdge(condition))
        self.last_loop_continue_state.get(sdfg)
        if self.last_sdfg_states[sdfg] not in [
                self.last_loop_break_state.get(sdfg),
                self.last_loop_continue_state.get(sdfg),
                self.last_return_state.get(sdfg)
        ]:
            body_ifend_state = add_simple_state_to_sdfg(
                self, sdfg, "BodyIfEndState" + str(line))
            sdfg.add_edge(body_ifend_state, final_substate,
                          dace.InterstateEdge())

        if hasattr(node, "body_else"):
            used_vars_else = [
                node for node in walk(node.body_else[0])
                if isinstance(node, DeclRefExpr)
            ]
            used_vars_else = remove_duplicates(used_vars_else)
            name_else = "Else" + str(line)
            new_sdfg_else = dace.SDFG(name_else)
            body_elsestart_state = sdfg.add_state("BodyElseStartState" +
                                                  str(line))
            self.last_sdfg_states[sdfg] = body_elsestart_state
            self.translate(node.body_else[0], sdfg)
            body_elseend_state = add_simple_state_to_sdfg(
                self, sdfg, "BodyElseEndState" + str(line))

            #else_state = make_nested_sdfg_with_no_context_change(
            #    sdfg, new_sdfg_else, name_else, used_vars_else, self)
            #self.translate(node.body_else[0], new_sdfg_else)
            sdfg.add_edge(guard_substate, body_elsestart_state,
                          dace.InterstateEdge("not " + condition))
            sdfg.add_edge(body_elseend_state, final_substate,
                          dace.InterstateEdge())
        else:
            sdfg.add_edge(guard_substate, final_substate,
                          dace.InterstateEdge("not " + condition))
        self.last_sdfg_states[sdfg] = final_substate

    def while2sdfg(self, node: WhileStmt, sdfg: SDFG):
        # print("LEN:",len(node.body_if))
        used_vars = [
            node for node in walk(node.body[0])
            if isinstance(node, DeclRefExpr)
        ]
        # for j in used_vars:
        #    print("USEDDUPLIC:",j.name)
        used_vars = remove_duplicates(used_vars)
        # for j in used_vars:
        #    print("USED:",j.name)
        # for i in node.__dict__:
        #    print(i)
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1

        name = "While" + str(line)
        new_sdfg = dace.SDFG(name)
        begin_state = add_simple_state_to_sdfg(self, sdfg,
                                               "BeginState" + str(line))
        guard_substate = sdfg.add_state("GuardState" + str(line))
        sdfg.add_edge(begin_state, guard_substate, dace.InterstateEdge())

        condition = ConditionWriter(
            sdfg, self.name_mapping).write_tasklet_code(node.cond[0])

        body_start_state = sdfg.add_state("BodyWhileStartState" + str(line))
        self.last_sdfg_states[sdfg] = body_start_state
        final_substate = sdfg.add_state("MergeState" + str(line))
        self.last_loop_break[sdfg] = final_substate
        self.last_loop_continue[sdfg] = guard_substate
        self.translate(node.body[0], sdfg)
        body_end_state = add_simple_state_to_sdfg(
            self, sdfg, "BodyWhileEndState" + str(line))

        #body_state = make_nested_sdfg_with_no_context_change(
        #    sdfg, new_sdfg, name, used_vars, self)

        self.translate(node.body[0], new_sdfg)

        sdfg.add_edge(guard_substate, body_start_state,
                      dace.InterstateEdge(condition))
        sdfg.add_edge(body_end_state, guard_substate, dace.InterstateEdge())
        sdfg.add_edge(guard_substate, final_substate,
                      dace.InterstateEdge("not " + condition))
        self.last_sdfg_states[sdfg] = final_substate

    def do2sdfg(self, node: DoStmt, sdfg: SDFG):
        # print("LEN:",len(node.body_if))
        used_vars = [
            node for node in walk(node.body[0])
            if isinstance(node, DeclRefExpr)
        ]
        # for j in used_vars:
        #    print("USEDDUPLIC:",j.name)
        used_vars = remove_duplicates(used_vars)
        # for j in used_vars:
        #    print("USED:",j.name)
        # for i in node.__dict__:
        #    print(i)
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1

        name = "Do" + str(line)
        new_sdfg = dace.SDFG(name)
        begin_state = add_simple_state_to_sdfg(self, sdfg,
                                               "BeginState" + str(line))

        condition = ConditionWriter(
            sdfg, self.name_mapping).write_tasklet_code(node.cond[0])

        #body_state = make_nested_sdfg_with_no_context_change(
        #    sdfg, new_sdfg, name, used_vars, self)
        body_start_state = sdfg.add_state("BodyDoStartState" + str(line))
        self.last_sdfg_states[sdfg] = body_start_state
        self.translate(node.body[0], sdfg)
        guard_substate = sdfg.add_state("GuardState" + str(line))
        final_substate = sdfg.add_state("MergeState" + str(line))

        self.last_loop_break[sdfg] = final_substate
        self.last_loop_continue[sdfg] = guard_substate
        body_end_state = add_simple_state_to_sdfg(self, sdfg,
                                                  "BodyDoEndState" + str(line))

        sdfg.add_edge(begin_state, body_start_state, dace.InterstateEdge())
        sdfg.add_edge(guard_substate, body_start_state,
                      dace.InterstateEdge(condition))
        sdfg.add_edge(body_end_state, guard_substate, dace.InterstateEdge())
        sdfg.add_edge(guard_substate, final_substate,
                      dace.InterstateEdge("not " + condition))
        self.last_sdfg_states[sdfg] = final_substate

    def forstmt2sdfg(self, node: ForStmt, sdfg: SDFG):
        if isinstance(node.body, list):
            node.body = node.body[0]
        used_vars = [
            node for node in walk(node.body) if isinstance(node, DeclRefExpr)
        ]
        # for j in used_vars:
        #    print("USEDDUPLIC:",j.name)
        used_vars = remove_duplicates(used_vars)
        # for j in used_vars:
        #    print("USED:",j.name)
        # for i in node.__dict__:
        #    print(i)
        if hasattr(node, "lineno"):
            line = node.lineno
        else:
            line = self.tasklet_count
            self.tasklet_count += 1
        declloop = False
        name = "FOR" + str(line)
        new_sdfg = dace.SDFG(name)
        begin_state = add_simple_state_to_sdfg(self, sdfg,
                                               "BeginState" + str(line))
        guard_substate = sdfg.add_state("GuardState" + str(line))
        final_substate = sdfg.add_state("MergeState" + str(line))
        self.last_loop_break[sdfg] = final_substate

        decl_node = node.init[0]
        entry = {}
        #print("FORSTMT:", decl_node.__class__)
        if isinstance(decl_node, DeclStmt):
            declloop = True
            # print("DECL in FOR")
            if len(decl_node.vardecl) != 1:
                raise ValueError("Only one var decl in for decl supported")
            if hasattr(decl_node.vardecl[0], "typeref"):
                # print("HERE:",decl_node.vardecl[0].typeref[0])
                datatype = self.typedefs[decl_node.vardecl[0].typeref[0].name]
            elif hasattr(decl_node.vardecl[0], "type"):
                # print(decl_node.vardecl[0].type)
                datatype = self.get_dace_type(decl_node.vardecl[0].type)
            if datatype == dace.int32:
                iter_name = decl_node.vardecl[0].name
                if iter_name not in sdfg.symbols:
                    sdfg.add_symbol(iter_name, dace.int32)
                    # self.name_mapping[(sdfg,iter_name)]=iter_name

                if hasattr(decl_node.vardecl[0], "init"):
                    init = decl_node.vardecl[0].init
                    #print("FOR INIT:", init)
                    if isinstance(init, IntLiteral):
                        entry[iter_name] = init.value[0]
                    elif isinstance(init, DeclRefExpr):
                        if init.name in sdfg.symbols:
                            entry[iter_name] = init.name
                        else:
                            entry[iter_name] = self.name_mapping[sdfg][
                                init.name]
                    elif isinstance(init, BinOp):
                        entry[iter_name] = ConditionWriter(
                            sdfg, self.name_mapping).write_tasklet_code(init)
                    elif isinstance(init, UnOp):
                        entry[iter_name] = ConditionWriter(
                            sdfg, self.name_mapping).write_tasklet_code(init)
                    else:
                        raise TypeError("Unexpected for declaration type")
                #sdfg.add_edge(begin_state, guard_substate, dace.InterstateEdge(assignments=entry))
        elif isinstance(decl_node, BinOp):
            assert decl_node.op == "=", "expecting init BinOp of for-stmt to be ="
            assert isinstance(
                decl_node.lvalue,
                DeclRefExpr), "expecting lvalue of binop to be a declRefExpr"
            iter_name = self.name_mapping[sdfg][decl_node.lvalue.name]
            datatype = self.get_dace_type(decl_node.lvalue.type)
            if isinstance(decl_node.rvalue, IntLiteral):
                entry[iter_name] = decl_node.rvalue.value[0]
            elif isinstance(decl_node.rvalue[0], DeclRefExpr):
                if decl_node.rvalue[0].name in sdfg.symbols:
                    entry[iter_name] = decl_node.rvalue[0].name
                else:
                    entry[iter_name] = self.name_mapping[sdfg][
                        decl_node.rvalue[0].name]
            else:
                raise_exception(
                    TypeError("Unsupported node type" +
                              decl_node.rvalue.__class__))
        #print("FORSTMT: ", entry)
        sdfg.add_edge(begin_state, guard_substate,
                      dace.InterstateEdge(assignments=entry))

        condition = ConditionWriter(
            sdfg, self.name_mapping).write_tasklet_code(node.cond[0])

        increment = "i+0+1"
        if isinstance(node.iter[0], BinOp):
            op = node.iter[0].op
            if op == "=" and isinstance(node.iter[0].lvalue, DeclRefExpr):
                if declloop and node.iter[0].lvalue.name == iter_name:
                    increment = TaskletWriter([], []).write_tasklet_code(
                        node.iter[0].rvalue)
                elif not declloop and self.name_mapping[sdfg][
                        node.iter[0].lvalue.name] == iter_name:
                    increment = ConditionWriter(
                        sdfg, self.name_mapping).write_tasklet_code(
                            node.iter[0].rvalue)
        entry = {iter_name: increment}
        #begin_loop_state = sdfg.add_state("BeginLoopState" + str(line))
        #end_loop_state = sdfg.add_state("EndLoopState" + str(line))
        #self.last_sdfg_states[sdfg] = begin_loop_state
        #self.last_loop_continue[sdfg] = end_loop_state
        #self.translate(node.body, sdfg)

        #sdfg.add_edge(self.last_sdfg_states[sdfg], end_loop_state,
        #             dace.InterstateEdge())

        loop_state = make_nested_sdfg_with_no_context_change(
            sdfg, new_sdfg, name, used_vars, node, self)
        self.translate(node.body, new_sdfg)

        self.last_sdfg_states[sdfg] = final_substate
        #sdfg.add_edge(guard_substate, begin_loop_state,
        #              dace.InterstateEdge(condition))
        #sdfg.add_edge(end_loop_state, guard_substate,
        #              dace.InterstateEdge(assignments=entry))
        #sdfg.add_edge(guard_substate, final_substate,
        #              dace.InterstateEdge("not " + condition))

        sdfg.add_edge(guard_substate, loop_state,
                      dace.InterstateEdge(condition))
        sdfg.add_edge(loop_state, guard_substate,
                      dace.InterstateEdge(assignments=entry))
        sdfg.add_edge(guard_substate, final_substate,
                      dace.InterstateEdge("not " + condition))

    def call2sdfg(self, node: CallExpr, sdfg: SDFG):
        #print("CALL EXPR")
        #print(node.name.name)
        self.last_call_expression[sdfg] = node.args
        match_found = False
        rettype = node.type
        hasret = False
        #print("CALLTYPE:", rettype)
        for i in self.ast.funcdefs:
            if i.name == node.name.name:
                match_found = True
                rettype = i.result_type

                #print("aa: ", rettype)
                self.funcdecl2sdfg(i, sdfg)
        if not match_found:
            libstate = self.libraries.get(node.name.name)
            if not isinstance(rettype, Void) and hasattr(node, "hasret"):
                if node.hasret:
                    hasret = True
                    retval = node.args.pop(len(node.args) - 1)
            if node.name.name == "free":
                return
            input_names_tasklet = {}
            output_names_tasklet = []
            input_names = []
            output_names = []
            special_list_in = {}
            special_list_out = []
            if libstate is not None:
                #print("LIBSTATE:", libstate)
                special_list_in[self.name_mapping[sdfg][libstate] +
                                "_task"] = dace.pointer(
                                    sdfg.arrays.get(self.name_mapping[sdfg]
                                                    [libstate]).dtype)
                special_list_out.append(self.name_mapping[sdfg][libstate] +
                                        "_task_out")
            used_vars = [
                node for node in walk(node) if isinstance(node, DeclRefExpr)
            ]

            for i in used_vars:
                for j in sdfg.arrays:
                    if self.name_mapping.get(sdfg).get(
                            i.name) == j and j not in input_names:
                        elem = sdfg.arrays.get(j)
                        scalar = False
                        if len(elem.shape) == 0:
                            scalar = True
                        elif (len(elem.shape) == 1 and elem.shape[0] == 1):
                            scalar = True
                        if not scalar and not node.name.name in [
                                "fprintf", "printf"
                        ]:
                            #    print("ADDING!",
                            #          not node.name.name in ["fprintf", "printf"],
                            #          not scalar)
                            output_names.append(j)
                            output_names_tasklet.append(i.name)
                        #print("HERE: ", elem.__class__, j, scalar,
                        #      node.name.name)

                        input_names_tasklet[i.name] = dace.pointer(elem.dtype)
                        input_names.append(j)

            output_names_changed = []
            for o, o_t in zip(output_names, output_names_tasklet):
                # changes=False
                # for i,i_t in zip(input_names,input_names_tasklet):
                #    if o_t==i_t:
                #        var=sdfg.arrays.get(i)
                #        if len(var.shape) == 0 or (len(var.shape) == 1 and var.shape[0] is 1):
                output_names_changed.append(o_t + "_out")

            node.location_line = self.tasklet_count
            tw = TaskletWriter(output_names_tasklet.copy(),
                               output_names_changed.copy())
            if not isinstance(rettype, Void) and hasret:
                special_list_in[retval.name] = dace.pointer(
                    self.get_dace_type(rettype))
                # special_list_in.append(retval.name)
                special_list_out.append(retval.name + "_out")
                text = tw.write_tasklet_code(
                    BinOp(lvalue=retval, op="=", rvalue=node)) + ";"

            else:
                text = tw.write_tasklet_code(node) + ";"
            substate = add_simple_state_to_sdfg(
                self, sdfg, "_state" + str(node.location_line) + "_" +
                str(self.tasklet_count))
            self.tasklet_count = self.tasklet_count + 1

            tasklet = add_tasklet(substate, str(node.location_line), {
                **input_names_tasklet,
                **special_list_in
            }, output_names_changed + special_list_out, "text")
            if libstate is not None:
                add_memlet_read(substate, self.name_mapping[sdfg][libstate],
                                tasklet,
                                self.name_mapping[sdfg][libstate] + "_task",
                                "0")

                add_memlet_write(
                    substate, self.name_mapping[sdfg][libstate], tasklet,
                    self.name_mapping[sdfg][libstate] + "_task_out", "0")
            if not isinstance(rettype, Void) and hasret:
                add_memlet_read(substate, self.name_mapping[sdfg][retval.name],
                                tasklet, retval.name, "0")

                add_memlet_write(substate,
                                 self.name_mapping[sdfg][retval.name], tasklet,
                                 retval.name + "_out", "0")

            for i, j in zip(input_names, input_names_tasklet):
                memlet_range = self.get_memlet_range(sdfg, used_vars, i, j)
                add_memlet_read(substate, i, tasklet, j, memlet_range)

            for i, j, k in zip(output_names, output_names_tasklet,
                               output_names_changed):

                memlet_range = self.get_memlet_range(sdfg, used_vars, i, j)
                add_memlet_write(substate, i, tasklet, k, memlet_range)

            setattr(tasklet, "code", CodeBlock(text, dace.Language.CPP))

    def malloc2sdfg(self, node: BinOp, sdfg: SDFG):
        varname = get_var_name(node.lvalue)

        #print("VARNAME:", varname)

        # sanity check
        if not isinstance(node.rvalue, CallExpr):
            print("WARNING: malloc2sdfg: rvalue is not a call")

        rvalue = node.rvalue.args[0]

        if self.incomplete_arrays.get((sdfg, varname)) is not None:
            oldnode = self.incomplete_arrays.get((sdfg, varname))
            del self.incomplete_arrays[(sdfg, varname)]
            oldsizes = []
            totalsize = 1
            etype = oldnode.type.pointee_type
            while isinstance(etype, ConstantArray):
                oldsizes.append(etype.size)
                totalsize = totalsize * etype.size
                etype = etype.element_type

            # if is a single value
            if len(oldsizes) == 0:
                oldsizes.append(1)

            datatype = self.get_dace_type(oldnode.type)
            sizes = []
            if isinstance(rvalue, IntLiteral):
                sizes.insert(0, rvalue.value[0])
            elif isinstance(rvalue, BinOp):
                tw = TaskletWriter([], [], self.name_mapping[sdfg])
                text = tw.write_tasklet_code(rvalue)
                sizes.insert(0, text)
            else:
                raise TypeError("malloc value cannot be parsed")
            if totalsize == dace.symbolic.pystr_to_symbolic(sizes[0]):
                #print("MAtch for sizes:", totalsize)
                sizes = oldsizes
            else:
                if (totalsize != 1):
                    print("Not Match for sizes:", totalsize, sizes[0])
                    raise ValueError("pointer sizes mismatch")
            
            # check if we have a double pointer
            if "STRUCT" in varname:
                if isinstance(oldnode.type.pointee_type, Pointer) and isinstance(oldnode.type.pointee_type.pointee_type, Pointer):
                    sizes.append(0)
            else:
                if isinstance(oldnode.type.pointee_type, Pointer):
                    sizes.append(0)
            #print(datatype.__class__.__name__)
            #print(oldnode.name, sizes, datatype)
            print("mallocing ", oldnode.name, " of size ", sizes, " and type ", datatype)
            mapped_name = find_new_array_name(self.all_array_names, oldnode.name)
            self.name_mapping[sdfg][oldnode.name] = mapped_name
            sdfg.add_array(mapped_name,
                           shape=sizes,
                           dtype=datatype,
                           transient=True)
            self.all_array_names.append(mapped_name)

        else:
            mapped_name = self.name_mapping[sdfg][varname]
            arr = sdfg._arrays.get(mapped_name)
            if arr is None:
                print("WARNING: array to overwrite not found in sdfg")
                return

            datatype = arr.dtype

            shape = list(arr.shape)

            # check if we are assigning to the elements of an array
            if isinstance(node.lvalue, ArraySubscriptExpr):
                index = 1
                if shape[1] != 0:
                    print("WARNING: skipping allocation of ", mapped_name, " because already allocated")
                    return
            else:
                index = 0
                if shape[0] != 1:
                    print("WARNING: skipping allocation of ", mapped_name, " because already allocated")
                    return

            if isinstance(rvalue, IntLiteral):
                shape[0] = rvalue.value[0]
            elif isinstance(rvalue, BinOp):
                tw = TaskletWriter([], [], self.name_mapping[sdfg])
                text = tw.write_tasklet_code(rvalue)
                shape[index] = text
            else:
                raise TypeError("malloc value cannot be parsed")

            print("Modified array:", mapped_name, " to have shape:", shape)
            self.name_mapping[sdfg][varname] = find_new_array_name(self.all_array_names, varname)
            sdfg.add_array(self.name_mapping[sdfg][varname],
                           shape=shape,
                           dtype=datatype,
                           transient=True)
            self.all_array_names.append(self.name_mapping[sdfg][varname])

    def arrayassign2sdfg(self, node, sdfg, output_names, output_names_tasklet, output_vars, input_vars, array_subsets, array_subsets_vars, reverse_mapping):
        output_names_changed = [o_t + "_out" for o_t in output_names_tasklet]

        substate = add_simple_state_to_sdfg(
            self, sdfg,
            "_state" + str(node.location_line) + "_" + str(self.tasklet_count))
        self.tasklet_count = self.tasklet_count + 1

        for arr, arr_ptr in array_subsets.items():
            view_name = find_new_array_name(self.all_array_names, arr + "_view")
            self.all_array_names.append(view_name)

            src = substate.add_read(arr)
            view = substate.add_access(view_name)
            arr_obj = self.get_arrays_in_context(sdfg).get(arr)

            if arr_obj is None:
                print("Arr is not defined, arrassign")

            memlet_subset = self.get_memlet_range(sdfg, input_vars, arr, reverse_mapping[arr], offset=arr_ptr)
            view_memlet = dace.Memlet(data=arr, subset=memlet_subset)
            padded_subset = "0:" + str(view_memlet.volume)
            view_memlet.other_subset = padded_subset

            sdfg.add_view(view_name, [view_memlet.volume], arr_obj.dtype)
            substate.add_edge(src, None, view, 'views', view_memlet)
            for i, j, k in zip(output_names, output_names_tasklet,
                            output_names_changed):

                memlet_range = self.get_memlet_range(sdfg, output_vars, i, j)

                memlet_split = memlet_range.split(",")
                if isinstance(node.lvalue, ArraySubscriptExpr):
                    index = node.lvalue.index.name
                    memlet_split[0] = self.get_name_mapping_in_context(sdfg)[index]
                    memlet_split[1] = padded_subset
                else:
                    memlet_split[0] = padded_subset

                memlet_range = ",".join(memlet_split)

                dst = substate.add_write(i)
                memlet = dace.Memlet(data=i, subset=memlet_range, other_subset=padded_subset)
                substate.add_nedge(view, dst, memlet)


    def binop2sdfg(self, node: BinOp, sdfg: SDFG):
        node.location_line = self.tasklet_count
        call_expressions = [n for n in walk(node) if isinstance(n, CallExpr)]
        #print("Num calls:", len(call_expressions))
        # assert(len(call_expressions)<=1)
        if len(call_expressions) == 1:
            augmented_call = call_expressions[0]
            #print("CALL NAME:", augmented_call.name.name)
            if augmented_call.name.name == "malloc":
                #print("FOUND MALLOC")
                self.malloc2sdfg(node, sdfg)
                return
            if augmented_call.name.name not in ["sqrt", "expf", "powf"]:
                augmented_call.args.append(node.lvalue)
                augmented_call.hasret = True
                self.call2sdfg(augmented_call, sdfg)
                return
        if node.op == "**":
            print("ERROR HERE")
        self.tasklet_count += 1

        inputnodefinder = FindInputNodesVisitor()
        inputnodefinder.visit(node)
        input_vars = inputnodefinder.nodes
        input_names = []
        input_names_tasklet = []

        previous_array = None
        array_subsets = dict()
        array_subsets_vars = []
        reverse_mapping = dict()

        for i in input_vars:
            mapped_name = self.get_name_mapping_in_context(sdfg).get(i.name)
            arrays = self.get_arrays_in_context(sdfg)

            if mapped_name not in arrays:
                continue

            if previous_array is not None and i.name.startswith("tmp_array_ptr"):
                array_subsets[previous_array] = mapped_name
                array_subsets_vars.append(i.name)
                previous_array = None

            arr = arrays.get(mapped_name)
            if len(arr.shape) > 1 or arr.shape[0] != 1:
                previous_array = mapped_name            
                reverse_mapping[mapped_name] = i.name

            # do not add duplicates
            if mapped_name in input_names:
                continue

            input_names.append(mapped_name)
            input_names_tasklet.append(i.name)


        outputnodefinder = FindOutputNodesVisitor()
        outputnodefinder.visit(node)
        output_vars = outputnodefinder.nodes
        output_names = []
        output_names_tasklet = []

        for i in output_vars:
            arrays = self.get_arrays_in_context(sdfg)
            mapped_name = self.get_name_mapping_in_context(sdfg).get(i.name)

            if mapped_name in arrays and mapped_name not in output_names:
                output_names.append(mapped_name)
                output_names_tasklet.append(i.name)

        substate = add_simple_state_to_sdfg(
            self, sdfg,
            "_state" + str(node.location_line) + "_" + str(self.tasklet_count))
        self.tasklet_count = self.tasklet_count + 1

        output_names_changed = [o_t + "_out" for o_t in output_names_tasklet]

        #output_names_dict = {on: dace.pointer(dace.int32) for on in output_names_changed}

        tasklet = add_tasklet(substate, str(node.location_line),
                              input_names_tasklet, output_names_changed,
                              "text")

        for i, j in zip(input_names, input_names_tasklet):

            memlet_range = self.get_memlet_range(sdfg, input_vars, i, j)
            add_memlet_read(substate, i, tasklet, j, memlet_range)

        for i, j, k in zip(output_names, output_names_tasklet,
                           output_names_changed):

            memlet_range = self.get_memlet_range(sdfg, output_vars, i, j)
            add_memlet_write(substate, i, tasklet, k, memlet_range)

        tw = TaskletWriter(output_names_tasklet, output_names_changed)
        # print("BINOP:",output_names,output_names_tasklet,output_names_changed)
        text = tw.write_tasklet_code(node) + ";"
        
        # remove offset that was already applied
        oldtext = text
        for i in array_subsets_vars:
            text = text.replace("+"+i, "")

        if len(array_subsets_vars) > 0:
            print("applied offset transformation ", oldtext, " => ", text)

        # print("BINOPTASKLET:",text)
        tasklet.code = CodeBlock(text, dace.Language.CPP)

    def declstmt2sdfg(self, node: DeclStmt, sdfg: SDFG):
        for i in node.vardecl:
            self.translate(i, sdfg)

    def vardecl2sdfg(self, node: VarDecl, sdfg: SDFG):
        # for i in node.__dict__:
        #    print(i)
        #    print(node.__getattribute__(i))
        if hasattr(node, "typeref") and len(node.typeref) > 0:
            #print("vardecl typeref",node.typeref[0].name)
            if node.typeref[0].name in self.typedefs:
                datatype = self.typedefs[node.typeref[0].name]
            else:
                datatype = self.get_dace_type(node.type)
        elif node.type is not None:
            # print("vardecl type",node.type)
            if isinstance(node.type, Pointer):
                self.incomplete_arrays[(sdfg, node.name)] = node
                return

            datatype = self.get_dace_type(node.type)
        if hasattr(node, "sizes"):
            sizes = []
            for i in node.sizes:
                if isinstance(i, IntLiteral):
                    sizes.insert(0, i.value[0])
                elif isinstance(i, BinOp):
                    tw = TaskletWriter([], [])
                    text = tw.write_tasklet_code(i)
                    sizes.insert(0, text)
                else:
                    print("Size:", i.__class__.__name__)
        else:
            sizes = [1]
        self.name_mapping[sdfg][node.name] = find_new_array_name(
            self.all_array_names, node.name)
        if len(sizes) == 0 or (len(sizes) == 1 and sizes[0] == 1):
            sdfg.add_scalar(self.name_mapping[sdfg][node.name],
                            dtype=datatype,
                            transient=True)
        else:
            sdfg.add_array(self.name_mapping[sdfg][node.name],
                           shape=sizes,
                           dtype=datatype,
                           transient=True)

        # print(datatype.__class__.__name__)
        # print(node.name,sizes,datatype)

        #sdfg.add_array(self.name_mapping[(sdfg, node.name)], shape=sizes, dtype=datatype,transient=True)
        self.all_array_names.append(self.name_mapping[sdfg][node.name])

    def parmdecl2sdfg(self, node: ParmDecl, sdfg: SDFG):
        # for i in node.__dict__:
        #    print("PARMDECL: ",i)
        #    print(node.__getattribute__(i))
        if hasattr(node, "typeref") and len(node.typeref) > 0:
            #    print("parmdecl typeref",node.typeref[0].name)
            datatype = self.typedefs[node.typeref[0].name]
        elif node.type is not None:
            # print("vardecl type",node.type)

            datatype = self.get_dace_type(node.type)
        if hasattr(node, "sizes"):
            if len(node.sizes) == 0:
                scalar = True
            else:
                sizes = []
                scalar = False
                for i in node.sizes:
                    if isinstance(i, IntLiteral):
                        sizes.insert(0, i.value[0])
                    elif isinstance(i, BinOp):
                        tw = TaskletWriter([], [])
                        text = tw.write_tasklet_code(i)
                        sizes.insert(0, text)
                    else:
                        print("Size:", i.__class__.__name__)
        else:
            scalar = True
        # print(datatype.__class__.__name__)
        # print(node.name,sizes,datatype)
        self.name_mapping[sdfg][node.name] = find_new_array_name(
            self.all_array_names, node.name)
        if scalar:
            #print("SCALAR ADD:",node.name)
            sdfg.add_scalar(self.name_mapping[sdfg][node.name],
                            dtype=datatype,
                            transient=True)
        else:
            sdfg.add_array(self.name_mapping[sdfg][node.name],
                           shape=sizes,
                           dtype=datatype,
                           transient=True)
        self.all_array_names.append(self.name_mapping[sdfg][node.name])

    def get_memlet_range(self, sdfg: SDFG, variables: List[Node],
                         var_name: str, var_name_tasklet: str, offset='0') -> str:
        var = self.get_arrays_in_context(sdfg).get(var_name)

        if len(var.shape) == 0:
            return ""

        if (len(var.shape) == 1 and var.shape[0] == 1):
            return "0"

        for o_v in variables:
            if o_v.name == var_name_tasklet:
                return generate_memlet(o_v, sdfg, self, offset)

    def get_arrays_in_context(self, sdfg: SDFG):
        a = self.globalsdfg.arrays.copy()
        if sdfg is not self.globalsdfg:
            a.update(sdfg.arrays)
        return a

    def get_name_mapping_in_context(self, sdfg: SDFG):
        a = self.name_mapping[self.globalsdfg].copy()
        if sdfg is not self.globalsdfg:
            a.update(self.name_mapping[sdfg])
        return a
