import clang.cindex
from clang.cindex import Cursor, CursorKind, TypeKind, TokenKind
from c2d_c_ast import *

from typing import List


# === NODE TRAMSLATIONS ============================================================================
def translation_unit(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    funcdefs = [node for node in nodes if isinstance(node, FuncDecl)]
    typedefs = [node for node in nodes if isinstance(node, TypeDecl)]
    vardefs = [node for node in nodes if isinstance(node, VarDecl)]
    decldefs = [node for node in nodes if isinstance(node, DeclStmt)]
    #print("VD: ",len(vardefs)," DD:",len(decldefs))
    structdefs = [node for node in nodes if isinstance(node, StructDecl)]
    classdefs = [node for node in nodes if isinstance(node, ClassDecl)]
    cxxmethods = [node for node in nodes if isinstance(node, CXXMethod)]

    return AST(typedefs=typedefs,
               funcdefs=funcdefs,
               vardefs=vardefs,
               decldefs=decldefs,
               structdefs=structdefs,
               classdefs=classdefs,
               cxxmethods=cxxmethods,
               lineno=cnode.location.line)


def typedef_decl(cnode, files):
    return Node()
    children = list(cnode.get_children())
    tokens = [tok.spelling for tok in list(cnode.get_tokens())]
    tokens.pop(tokens.index('typedef'))
    tokens.pop(tokens.index(cnode.spelling))
    if len(children) == 0:
        # print("TYPEDEF simple:", tokens)
        return TypeDecl(name=cnode.spelling,
                        type=get_c_type_from_clang_type(cnode.type),
                        lineno=cnode.location.line)
    else:
        nodes = [create_own_ast(i, files) for i in cnode.get_children()]
        # print("TYPEDEF:",nodes,tokens)
        if len(nodes) == 1 and isinstance(nodes[0], TypeRef):
            #list_all(cnode)
            tokens.pop(tokens.index(nodes[0].__getattribute__("name")))
            if len(tokens) == 0:
                return TypeDecl(name=cnode.spelling,
                                typeref=[nodes[0]],
                                lineno=cnode.location.line)
            else:
                return TypeDecl(name=cnode.spelling,
                                type=get_c_type_from_clang_type(cnode.type),
                                typeref=[nodes[0]],
                                lineno=cnode.location.line)
        if len(nodes) == 1 and isinstance(nodes[0], StructDecl):
            return TypeDecl(name=cnode.spelling,
                            typeref=[nodes[0]],
                            lineno=cnode.location.line)
        else:
            #list_all(cnode)
            raise (Exception("Complex typedefs not supported"))


def func_decl(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    args = [node for node in nodes if isinstance(node, ParmDecl)]
    body = [node for node in nodes if isinstance(node, BasicBlock)]
    #print("FUNCDECL:",str(cnode.result_type.kind))
    if len(body) == 1:
        return FuncDecl(name=cnode.spelling,
                        args=args,
                        body=body[0],
                        lineno=cnode.location.line,
                        result_type=get_c_type_from_clang_type(
                            cnode.result_type))
    else:
        return FuncDecl(name=cnode.spelling,
                        args=args,
                        body=None,
                        lineno=cnode.location.line,
                        result_type=get_c_type_from_clang_type(
                            cnode.result_type))


def parm_decl(cnode, files):
    sizes = []
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    typerefs = [n for n in nodes if isinstance(n, TypeRef)]

    if cnode.type.kind is TypeKind.CONSTANTARRAY:
        sizes = [n for n in nodes if not isinstance(n, TypeRef)]
    return ParmDecl(name=cnode.spelling,
                    type=get_c_type_from_clang_type(cnode.type),
                    typeref=typerefs,
                    sizes=sizes,
                    lineno=cnode.location.line)


def remove_common_tokens(parent: List[str], child: List[str]):
    return_string = []
    if parent is None:
        return return_string
    if child is None:
        return parent
    local_copy = parent.copy()
    for i in range(0, len(local_copy) - len(child) + 1):
        full_match = True
        for j in range(0, len(child)):
            if local_copy[i + j] != child[j]:
                full_match = False
        if full_match:
            for j in range(0, len(child)):
                local_copy.pop(i)
            return local_copy
    return local_copy


def var_decl(cnode, files):
    #list_all(cnode)

    children = list(cnode.get_children())
    tokens = [tok.spelling for tok in list(cnode.get_tokens())]
    tokens.pop(tokens.index(cnode.spelling))
    typerefs = None
    init = None
    sizes = None
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    cnodes = list(cnode.get_children())
    for node in nodes:
        if isinstance(node, TypeRef):
            typeref = node
            try:
                tokens.pop(tokens.index(node.name))
            except ValueError:
                pass
            cnodes.pop(nodes.index(typeref))
            nodes.pop(nodes.index(typeref))

            typerefs = [typeref]
    try:
        index = tokens.index("=")
    except ValueError:
        index = -1
    if index != -1:
        tokens = tokens[0:tokens.index("=")]
        init = nodes[len(nodes) - 1]
        cnodes.pop(len(nodes) - 1)
        nodes.pop(len(nodes) - 1)
    #print("TOKENS bef:", tokens)
    type = get_c_type_from_clang_type(cnode.type)
    if cnode.type.kind is TypeKind.CONSTANTARRAY:
        #type=get_c_type_from_clang_type(cnode.type.element_type)
        sizes = nodes
        # print("HERE:",len(cnodes))
        for gcnode in cnodes:
            tokens2 = ['['
                       ] + [tok.spelling
                            for tok in list(gcnode.get_tokens())] + [']']
            #print(tokens2)
            tokens = remove_common_tokens(tokens, tokens2)
            #print(tokens)
    #print("type:", tokens,cnode.spelling)
    #print("typeref:",typerefs)
    #if isinstance(init,list):
    #        print("INIT:",cnode.spelling)
    kwargs = dict(name=cnode.spelling,
                  type=type,
                  typeref=typerefs,
                  init=init,
                  sizes=sizes,
                  lineno=cnode.location.line)

    return VarDecl(**{k: v for k, v in kwargs.items() if v is not None})


def basic_block(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    return BasicBlock(body=[x for x in nodes if x is not None])


def decl_stmt(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    vardecl = [node for node in nodes if isinstance(node, VarDecl)]
    notvardecl = [node for node in nodes if not isinstance(node, VarDecl)]
    # list_all(cnode)
    if len(notvardecl) != 0:
        raise (Exception(
            "Only Variable Declaration allowed in Declaration Statements"))
    return DeclStmt(vardecl=vardecl, lineno=cnode.location.line)


def int_literal(cnode, files):
    return IntLiteral(value=[tok.spelling for tok in list(cnode.get_tokens())],
                      lineno=cnode.location.line,
                      type=get_c_type_from_clang_type(cnode.type))


def cxx_bool_literal(cnode, files):
    return BoolLiteral(
        value=[tok.spelling for tok in list(cnode.get_tokens())],
        lineno=cnode.location.line,
        type=get_c_type_from_clang_type(cnode.type))


def string_literal(cnode, files):
    return StringLiteral(
        value=[tok.spelling for tok in list(cnode.get_tokens())],
        lineno=cnode.location.line,
        type=get_c_type_from_clang_type(cnode.type))


def char_literal(cnode, files):
    return CharLiteral(
        value=[tok.spelling for tok in list(cnode.get_tokens())],
        lineno=cnode.location.line,
        type=get_c_type_from_clang_type(cnode.type))


def float_literal(cnode, files):
    return FloatLiteral(
        value=[tok.spelling for tok in list(cnode.get_tokens())],
        lineno=cnode.location.line,
        type=get_c_type_from_clang_type(cnode.type))


def bin_op(cnode, files):

    children = list(cnode.get_children())
    tokens = [tok.spelling for tok in list(cnode.get_tokens())]
    try:
        operator = tokens[len(list(children[0].get_tokens()))]
    except:
        toks = [tok.spelling for tok in list(children[0].get_tokens())]
        #print(toks)
        raise Exception("EXPLOSION BINOP!")
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    #print("Op: ",operator," cnode loc: ",cnode.location.line)
    return BinOp(op=operator,
                 lvalue=nodes[0],
                 rvalue=nodes[1],
                 lineno=cnode.location.line)


def unary_op(cnode, files):
    children = list(cnode.get_children())
    tokens = [tok.spelling for tok in list(cnode.get_tokens())]
    child_tokens = [tok.spelling for tok in list(children[0].get_tokens())]
    postfix = True
    for i in range(len(child_tokens)):
        # print(tokens[i],child_tokens[i])
        if tokens[i] != child_tokens[i]:
            postfix = False
    if postfix:
        op = tokens[-1]
    else:
        op = tokens[0]
    # print(tokens)
    # print (op)
    node = create_own_ast(children[0], files)
    # print(node.__class__.__name__)
    return UnOp(postfix=postfix,
                lvalue=node,
                op=op,
                lineno=cnode.location.line)


def cxx_unary_expr(cnode, files):
    # TODO: IMPLEMENT
    #print("Unimplemented cxx unary expr")
    #list_all(cnode)
    toks = list(cnode.get_tokens())
    if toks[0].spelling == "sizeof":
        return IntLiteral(value=["1"])
    return CXXUnaryExpr(tokens=toks)


def decl_ref_expr(cnode, files):
    return DeclRefExpr(name=cnode.spelling,
                       type=get_c_type_from_clang_type(cnode.type),
                       lineno=cnode.location.line)


def array_subscript_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    decl_ref_node = nodes[0]
    while isinstance(decl_ref_node, ArraySubscriptExpr):
        decl_ref_node = decl_ref_node.unprocessed_name
    actual_name = ""
    if isinstance(decl_ref_node, DeclRefExpr):
        actual_name = decl_ref_node.name
    return ArraySubscriptExpr(unprocessed_name=nodes[0],
                              index=nodes[1],
                              name=actual_name,
                              indices="UNDEF",
                              lineno=cnode.location.line)


def unexposed_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    if len(nodes) == 0:
        return Node
    if len(nodes) != 1:
        print("Error start:")
        list_all(cnode)
        raise (Exception(
            "Unexposed Expression should be implicit cast and only have one child node"
        ))
    return nodes[0]


def unexposed_decl(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    Warning("Passing unexposed declaration")
    if len(nodes) != 1:
        raise (Exception(
            "Unexposed Declaration should be an external function and only have one child node"
        ))
    return nodes[0]


def for_stmt(cnode, files):
    for_list = list(cnode.get_children())
    #print(cnode.location.line)
    return ForStmt(
        init=[create_own_ast(for_list[0], files)],
        cond=[create_own_ast(for_list[1], files)],
        iter=[create_own_ast(for_list[2], files)],
        body=[create_own_ast(for_list[3], files)],
        lineno=cnode.location.line,
    )


def init_list_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    return InitListExpr(body=nodes)


def compound_assign_op(cnode, files):
    children = list(cnode.get_children())
    tokens = [tok.spelling for tok in list(cnode.get_tokens())]
    try:
        operator = tokens[len(list(children[0].get_tokens()))]
    except:
        toks = [tok.spelling for tok in list(children[0].get_tokens())]
        print(toks)
        raise Exception("EXPLOSION COMPOP!")
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    # print("Op: ",operator)
    return CompoundAssignOp(op=operator,
                            lvalue=nodes[0],
                            rvalue=nodes[1],
                            lineno=cnode.location.line)


def while_stmt(cnode, files):
    while_list = list(cnode.get_children())
    #print(cnode.location.line)
    return WhileStmt(
        cond=[create_own_ast(while_list[0], files)],
        body=[create_own_ast(while_list[1], files)],
        lineno=cnode.location.line,
    )


def do_stmt(cnode, files):
    do_list = list(cnode.get_children())
    print("DO STMT:", cnode.location.line)
    return DoStmt(
        cond=[create_own_ast(do_list[1], files)],
        body=[create_own_ast(do_list[0], files)],
        lineno=cnode.location.line,
    )


def if_stmt(cnode, files):
    if_list = list(cnode.get_children())

    if len(if_list) == 3:
        else_stmt = [create_own_ast(if_list[2], files)]
        return IfStmt(cond=[create_own_ast(if_list[0], files)],
                      body_if=[create_own_ast(if_list[1], files)],
                      body_else=else_stmt,
                      lineno=cnode.location.line)
    else:
        return IfStmt(cond=[create_own_ast(if_list[0], files)],
                      body_if=[create_own_ast(if_list[1], files)],
                      lineno=cnode.location.line)


def ret_stmt(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    if len(nodes) != 0:
        ret_expr = nodes[0]
        return RetStmt(ret_expr=ret_expr, lineno=cnode.location.line)
    else:
        return RetStmt(lineno=cnode.location.line)


def break_stmt(cnode, files):
    return BreakStmt(lineno=cnode.location.line)


def continue_stmt(cnode, files):
    return ContinueStmt(lineno=cnode.location.line)


def cxx_cast_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    return CxxCastExpr(type=nodes[0],
                       expr=nodes[1],
                       lineno=cnode.location.line)


def c_cast_expr(cnode, files):
    #list_all_d(cnode,0)

    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    #print("cast len:",len(nodes))
    #for i in nodes:
    #    print(i.__class__)

    return CCastExpr(expr=nodes[len(nodes) - 1], lineno=cnode.location.line)


def ternary_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    return TernaryExpr(cond=nodes[0],
                       left=nodes[1],
                       right=nodes[2],
                       lineno=cnode.location.line)


def paren_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    return ParenExpr(expr=nodes[0],
                     lineno=cnode.location.line,
                     type=get_c_type_from_clang_type(cnode.type))


def struct_decl(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    fields = [node for node in nodes if isinstance(node, FieldDecl)]

    return StructDecl(name=cnode.spelling,
                      fields=fields,
                      lineno=cnode.location.line)


def class_decl(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    fields = [node for node in nodes if isinstance(node, FieldDecl)]
    methods = [node for node in nodes if isinstance(node, CXXMethod)]

    return ClassDecl(name=cnode.spelling, fields=fields, methods=methods)


def cxx_method(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]
    args = [node for node in nodes if isinstance(node, ParmDecl)]
    body = next((node for node in nodes if isinstance(node, BasicBlock)), None)

    return CXXMethod(name=cnode.spelling,
                     args=args,
                     body=body,
                     parent_class_type=get_c_type_from_clang_type(
                         cnode.semantic_parent.type),
                     lineno=cnode.location.line)


def cxx_access_spec_decl(cnode, files):
    return CXXAccessSpecDecl()


def null_stmt(cnode, files):
    return Node()


def field_decl(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    return FieldDecl(name=cnode.spelling,
                     type=get_c_type_from_clang_type(cnode.type),
                     lineno=cnode.location.line)


def member_ref_expr(cnode, files):
    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    if len(nodes) == 0:
        # this is an implied member ref (example: "this->field" but omitting "this->".in class methods)
        referenced_class_decl = cnode.referenced.semantic_parent
        container_expr = CXXThisExpr(type=Pointer(
            pointee_type=get_c_type_from_clang_type(
                referenced_class_decl.type)))
    else:
        container_expr = nodes[0]

    return MemberRefExpr(name=cnode.spelling,
                         containerexpr=container_expr,
                         type=get_c_type_from_clang_type(cnode.type),
                         lineno=cnode.location.line)


def call_expr(cnode, files):
    #print(cnode.spelling)
    if cnode.spelling in ["malloc"]:
        print("Dound malloc")
    #    return DeclRefExpr(name=cnode.spelling)
    if cnode.spelling in ["__acrt_iob_func"]:
        return DeclRefExpr(name="stderr", type=Special())

    nodes = [create_own_ast(i, files) for i in cnode.get_children()]

    if cnode.spelling[:8] == "operator":
        # this is a operation of struct variables
        operator = cnode.spelling[8]
        if operator == "=":
            return BinOp(lvalue=nodes[0],
                         rvalue=nodes[2],
                         op=operator,
                         type=get_c_type_from_clang_type(cnode.type))
        raise Exception("unknown operator between struct variables: " +
                        operator)

    if cnode.type.kind in [TypeKind.ELABORATED, TypeKind.RECORD]:
        name = cnode.spelling
    elif len(nodes) == 0:
        name = cnode.spelling
    else:
        name = nodes.pop(0)
    #print("creating:",name.name,nodes)
    #list_all_d(cnode,0)
    return CallExpr(name=name, args=nodes, lineno=cnode.location.line)


def cxx_this_expr(cnode, files):
    return CXXThisExpr(type=get_c_type_from_clang_type(cnode.type))


# === TYPE TRAMSLATIONS ============================================================================
def type_unknown(clang_type: clang.cindex.Type):
    return Unknown()


def type_typedef(clang_type: clang.cindex.Type):
    return get_c_type_from_clang_type(clang_type.get_canonical())


def type_ref(cnode, files):

    #return get_c_type_from_clang_type(cnode.type.get_canonical())
    c_type = get_c_type_from_clang_type(cnode.type)
    #print("NODE Spelling", cnode.spelling, c_type)
    #list_all(cnode)
    name = cnode.spelling
    if isinstance(c_type, Struct):
        name = name.strip("struct ")
    return TypeRef(name=name, type=c_type, lineno=cnode.location.line)


def type_int(clang_type: clang.cindex.Type):
    return Int()


def type_longlong(clang_type: clang.cindex.Type):
    return LongLong()


def type_ulonglong(clang_type: clang.cindex.Type):
    return ULongLong()


def type_long(clang_type: clang.cindex.Type):
    return Long()


def type_float(clang_type: clang.cindex.Type):
    return Float()


def type_longdouble(clang_type: clang.cindex.Type):
    return Double()


def type_double(clang_type: clang.cindex.Type):
    return Double()


def type_char(clang_type: clang.cindex.Type):
    return Char()


def type_bool(clang_type: clang.cindex.Type):
    return Bool()


def type_functionproto(lang_type: clang.cindex.Type):
    return FunctionProto()


def type_void(lang_type: clang.cindex.Type):
    return Void()


def type_struct(clang_type: clang.cindex.Type):
    type_name = clang_type.spelling
    if type_name[:len("struct ")] == "struct ":
        type_name = type_name[len("struct "):]

    if clang_type.get_declaration().kind == CursorKind.CLASS_DECL:
        return Class(name=type_name)

    return Struct(name=type_name)


def type_pointer(clang_type: clang.cindex.Type):
    return Pointer(
        pointee_type=get_c_type_from_clang_type(clang_type.get_pointee()))


def type_constantarray(clang_type: clang.cindex.Type):
    return ConstantArray(element_type=get_c_type_from_clang_type(
        clang_type.get_array_element_type()),
                         size=clang_type.element_count)


def type_variablearray(clang_type: clang.cindex.Type):
    return VariableArray(element_type=get_c_type_from_clang_type(
        clang_type.get_array_element_type()),
                         size=-1)


def type_incompletearray(clang_type: clang.cindex.Type):
    return IncompleteArray(element_type=get_c_type_from_clang_type(
        clang_type.get_array_element_type()),
                           size=-1)


def get_c_type_from_clang_type(clang_type: clang.cindex.Type):
    if clang_type.kind in unsupported_type_kinds:
        print("Warning: Type Kind Unsupported: " + str(clang_type.kind))
        return type_unknown(clang_type)

    elif clang_type.kind in supported_type_kinds:
        c_type = supported_type_kinds[clang_type.kind](clang_type)
        return c_type

    raise Exception("How did you even get here?")


def create_own_ast(cnode, files):
    current_file = cnode.location.file

    if current_file is not None and str(current_file) not in files:
        #print("POSITON RETURN:", cnode.location.file, cnode.location.line)
        return
    #
    #print("POSITON PARSE:", cnode.location.file, cnode.location.line)
    if cnode.kind in supported_cursor_kinds:
        #try:
        #print(cnode.kind)
        node = supported_cursor_kinds[cnode.kind](cnode, files)
        #for i in node.__dict__:
        #    print(i)
        if "type" in node._attributes and not hasattr(node, "type"):
            node.type = get_c_type_from_clang_type(cnode.type)

        if hasattr(node, "type"):
            if isinstance(node, TypeRef):
                return node.type
            if not isinstance(node.type, Type):
                a = 1
            assert isinstance(node.type, Type)

        return node
    #except IndexError:
    #    list_all(cnode)
    #    print("EXPLOSION!")
    #    raise Exception("ARGH!")
    elif cnode.kind in ignored_cursor_kinds:
        print(UserWarning("Ignored cursor kind", cnode.kind))
        return
    elif cnode.kind in unsupported_cursor_kinds:
        #list_all(cnode)
        print(Warning(f"Unsupported {cnode.kind} at {cnode.location.line}"))
        return
        #raise ValueError("Cursor Kind Unsupported: " + str(cnode.kind) + " " + str(cnode.location.line))
    raise Exception("How did you even get here?")


ignored_cursor_kinds = {
    clang.cindex.CursorKind.USING_DECLARATION,
    clang.cindex.CursorKind.NULL_STMT,
}

supported_cursor_kinds = {
    clang.cindex.CursorKind.TRANSLATION_UNIT: translation_unit,
    clang.cindex.CursorKind.TYPEDEF_DECL: typedef_decl,
    clang.cindex.CursorKind.TYPE_REF: type_ref,
    clang.cindex.CursorKind.FUNCTION_DECL: func_decl,
    clang.cindex.CursorKind.PARM_DECL: parm_decl,
    clang.cindex.CursorKind.COMPOUND_STMT: basic_block,
    clang.cindex.CursorKind.DECL_STMT: decl_stmt,
    clang.cindex.CursorKind.VAR_DECL: var_decl,
    clang.cindex.CursorKind.CXX_BOOL_LITERAL_EXPR: cxx_bool_literal,
    clang.cindex.CursorKind.INTEGER_LITERAL: int_literal,
    clang.cindex.CursorKind.FLOATING_LITERAL: float_literal,
    clang.cindex.CursorKind.STRING_LITERAL: string_literal,
    clang.cindex.CursorKind.CHARACTER_LITERAL: char_literal,
    clang.cindex.CursorKind.BINARY_OPERATOR: bin_op,
    clang.cindex.CursorKind.DECL_REF_EXPR: decl_ref_expr,
    clang.cindex.CursorKind.ARRAY_SUBSCRIPT_EXPR: array_subscript_expr,
    clang.cindex.CursorKind.UNEXPOSED_EXPR: unexposed_expr,
    clang.cindex.CursorKind.UNEXPOSED_DECL: unexposed_decl,
    clang.cindex.CursorKind.INIT_LIST_EXPR: init_list_expr,
    clang.cindex.CursorKind.UNARY_OPERATOR: unary_op,
    clang.cindex.CursorKind.FOR_STMT: for_stmt,
    clang.cindex.CursorKind.IF_STMT: if_stmt,
    clang.cindex.CursorKind.DO_STMT: do_stmt,
    clang.cindex.CursorKind.WHILE_STMT: while_stmt,
    clang.cindex.CursorKind.RETURN_STMT: ret_stmt,
    clang.cindex.CursorKind.CXX_FUNCTIONAL_CAST_EXPR: cxx_cast_expr,
    clang.cindex.CursorKind.CSTYLE_CAST_EXPR: c_cast_expr,
    clang.cindex.CursorKind.PAREN_EXPR: paren_expr,
    clang.cindex.CursorKind.CALL_EXPR: call_expr,
    clang.cindex.CursorKind.CXX_UNARY_EXPR: cxx_unary_expr,
    clang.cindex.CursorKind.CONDITIONAL_OPERATOR: ternary_expr,
    clang.cindex.CursorKind.STRUCT_DECL: struct_decl,
    clang.cindex.CursorKind.FIELD_DECL: field_decl,
    clang.cindex.CursorKind.MEMBER_REF_EXPR: member_ref_expr,
    clang.cindex.CursorKind.CXX_METHOD: cxx_method,
    clang.cindex.CursorKind.CLASS_DECL: class_decl,
    clang.cindex.CursorKind.CXX_ACCESS_SPEC_DECL: cxx_access_spec_decl,
    clang.cindex.CursorKind.CXX_THIS_EXPR: cxx_this_expr,
    clang.cindex.CursorKind.COMPOUND_ASSIGNMENT_OPERATOR: compound_assign_op,
    clang.cindex.CursorKind.BREAK_STMT: break_stmt,
    clang.cindex.CursorKind.CONTINUE_STMT: continue_stmt,
}

unsupported_cursor_kinds = [
    CursorKind.UNION_DECL,
    CursorKind.ENUM_DECL,
    CursorKind.FIELD_DECL,
    CursorKind.ENUM_CONSTANT_DECL,
    CursorKind.OBJC_INTERFACE_DECL,
    CursorKind.OBJC_CATEGORY_DECL,
    CursorKind.OBJC_PROTOCOL_DECL,
    CursorKind.OBJC_PROPERTY_DECL,
    CursorKind.OBJC_IVAR_DECL,
    CursorKind.OBJC_INSTANCE_METHOD_DECL,
    CursorKind.OBJC_CLASS_METHOD_DECL,
    CursorKind.OBJC_IMPLEMENTATION_DECL,
    CursorKind.OBJC_CATEGORY_IMPL_DECL,
    CursorKind.NAMESPACE,
    CursorKind.LINKAGE_SPEC,
    CursorKind.CONSTRUCTOR,
    CursorKind.DESTRUCTOR,
    CursorKind.CONVERSION_FUNCTION,
    CursorKind.TEMPLATE_TYPE_PARAMETER,
    CursorKind.TEMPLATE_NON_TYPE_PARAMETER,
    CursorKind.TEMPLATE_TEMPLATE_PARAMETER,
    CursorKind.FUNCTION_TEMPLATE,
    CursorKind.CLASS_TEMPLATE,
    CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION,
    CursorKind.NAMESPACE_ALIAS,
    CursorKind.USING_DIRECTIVE,
    CursorKind.USING_DECLARATION,
    CursorKind.TYPE_ALIAS_DECL,
    CursorKind.OBJC_SYNTHESIZE_DECL,
    CursorKind.OBJC_DYNAMIC_DECL,
    CursorKind.OBJC_SUPER_CLASS_REF,
    CursorKind.OBJC_PROTOCOL_REF,
    CursorKind.OBJC_CLASS_REF,
    CursorKind.CXX_BASE_SPECIFIER,
    CursorKind.TEMPLATE_REF,
    CursorKind.NAMESPACE_REF,
    CursorKind.MEMBER_REF,
    CursorKind.LABEL_REF,
    CursorKind.OVERLOADED_DECL_REF,
    CursorKind.VARIABLE_REF,
    CursorKind.INVALID_FILE,
    CursorKind.NO_DECL_FOUND,
    CursorKind.NOT_IMPLEMENTED,
    CursorKind.INVALID_CODE,
    CursorKind.OBJC_MESSAGE_EXPR,
    CursorKind.BLOCK_EXPR,
    CursorKind.IMAGINARY_LITERAL,
    CursorKind.CONDITIONAL_OPERATOR,
    CursorKind.COMPOUND_LITERAL_EXPR,
    CursorKind.ADDR_LABEL_EXPR,
    CursorKind.StmtExpr,
    CursorKind.GENERIC_SELECTION_EXPR,
    CursorKind.GNU_NULL_EXPR,
    CursorKind.CXX_STATIC_CAST_EXPR,
    CursorKind.CXX_DYNAMIC_CAST_EXPR,
    CursorKind.CXX_REINTERPRET_CAST_EXPR,
    CursorKind.CXX_CONST_CAST_EXPR,
    CursorKind.CXX_FUNCTIONAL_CAST_EXPR,
    CursorKind.CXX_TYPEID_EXPR,
    CursorKind.CXX_BOOL_LITERAL_EXPR,
    CursorKind.CXX_NULL_PTR_LITERAL_EXPR,
    CursorKind.CXX_THROW_EXPR,
    CursorKind.CXX_NEW_EXPR,
    CursorKind.CXX_DELETE_EXPR,
    CursorKind.CXX_UNARY_EXPR,
    CursorKind.OBJC_STRING_LITERAL,
    CursorKind.OBJC_ENCODE_EXPR,
    CursorKind.OBJC_SELECTOR_EXPR,
    CursorKind.OBJC_PROTOCOL_EXPR,
    CursorKind.OBJC_BRIDGE_CAST_EXPR,
    CursorKind.PACK_EXPANSION_EXPR,
    CursorKind.SIZE_OF_PACK_EXPR,
    CursorKind.LAMBDA_EXPR,
    CursorKind.OBJ_BOOL_LITERAL_EXPR,
    CursorKind.OBJ_SELF_EXPR,
    CursorKind.OMP_ARRAY_SECTION_EXPR,
    CursorKind.OBJC_AVAILABILITY_CHECK_EXPR,
    CursorKind.UNEXPOSED_STMT,
    CursorKind.LABEL_STMT,
    CursorKind.CASE_STMT,
    CursorKind.DEFAULT_STMT,
    CursorKind.SWITCH_STMT,
    CursorKind.GOTO_STMT,
    CursorKind.INDIRECT_GOTO_STMT,
    CursorKind.ASM_STMT,
    CursorKind.OBJC_AT_TRY_STMT,
    CursorKind.OBJC_AT_CATCH_STMT,
    CursorKind.OBJC_AT_FINALLY_STMT,
    CursorKind.OBJC_AT_THROW_STMT,
    CursorKind.OBJC_AT_SYNCHRONIZED_STMT,
    CursorKind.OBJC_AUTORELEASE_POOL_STMT,
    CursorKind.OBJC_FOR_COLLECTION_STMT,
    CursorKind.CXX_CATCH_STMT,
    CursorKind.CXX_TRY_STMT,
    CursorKind.CXX_FOR_RANGE_STMT,
    CursorKind.SEH_TRY_STMT,
    CursorKind.SEH_EXCEPT_STMT,
    CursorKind.SEH_FINALLY_STMT,
    CursorKind.MS_ASM_STMT,
    CursorKind.OMP_PARALLEL_DIRECTIVE,
    CursorKind.OMP_SIMD_DIRECTIVE,
    CursorKind.OMP_FOR_DIRECTIVE,
    CursorKind.OMP_SECTIONS_DIRECTIVE,
    CursorKind.OMP_SECTION_DIRECTIVE,
    CursorKind.OMP_SINGLE_DIRECTIVE,
    CursorKind.OMP_PARALLEL_FOR_DIRECTIVE,
    CursorKind.OMP_PARALLEL_SECTIONS_DIRECTIVE,
    CursorKind.OMP_TASK_DIRECTIVE,
    CursorKind.OMP_MASTER_DIRECTIVE,
    CursorKind.OMP_CRITICAL_DIRECTIVE,
    CursorKind.OMP_TASKYIELD_DIRECTIVE,
    CursorKind.OMP_BARRIER_DIRECTIVE,
    CursorKind.OMP_TASKWAIT_DIRECTIVE,
    CursorKind.OMP_FLUSH_DIRECTIVE,
    CursorKind.SEH_LEAVE_STMT,
    CursorKind.OMP_ORDERED_DIRECTIVE,
    CursorKind.OMP_ATOMIC_DIRECTIVE,
    CursorKind.OMP_FOR_SIMD_DIRECTIVE,
    CursorKind.OMP_PARALLELFORSIMD_DIRECTIVE,
    CursorKind.OMP_TARGET_DIRECTIVE,
    CursorKind.OMP_TEAMS_DIRECTIVE,
    CursorKind.OMP_TASKGROUP_DIRECTIVE,
    CursorKind.OMP_CANCELLATION_POINT_DIRECTIVE,
    CursorKind.OMP_CANCEL_DIRECTIVE,
    CursorKind.OMP_TARGET_DATA_DIRECTIVE,
    CursorKind.OMP_TASK_LOOP_DIRECTIVE,
    CursorKind.OMP_TASK_LOOP_SIMD_DIRECTIVE,
    CursorKind.OMP_DISTRIBUTE_DIRECTIVE,
    CursorKind.OMP_TARGET_ENTER_DATA_DIRECTIVE,
    CursorKind.OMP_TARGET_EXIT_DATA_DIRECTIVE,
    CursorKind.OMP_TARGET_PARALLEL_DIRECTIVE,
    CursorKind.OMP_TARGET_PARALLELFOR_DIRECTIVE,
    CursorKind.OMP_TARGET_UPDATE_DIRECTIVE,
    CursorKind.OMP_DISTRIBUTE_PARALLELFOR_DIRECTIVE,
    CursorKind.OMP_DISTRIBUTE_PARALLEL_FOR_SIMD_DIRECTIVE,
    CursorKind.OMP_DISTRIBUTE_SIMD_DIRECTIVE,
    CursorKind.OMP_TARGET_PARALLEL_FOR_SIMD_DIRECTIVE,
    CursorKind.OMP_TARGET_SIMD_DIRECTIVE,
    CursorKind.OMP_TEAMS_DISTRIBUTE_DIRECTIVE,
    CursorKind.UNEXPOSED_ATTR,
    CursorKind.IB_ACTION_ATTR,
    CursorKind.IB_OUTLET_ATTR,
    CursorKind.IB_OUTLET_COLLECTION_ATTR,
    CursorKind.CXX_FINAL_ATTR,
    CursorKind.CXX_OVERRIDE_ATTR,
    CursorKind.ANNOTATE_ATTR,
    CursorKind.ASM_LABEL_ATTR,
    CursorKind.PACKED_ATTR,
    CursorKind.PURE_ATTR,
    CursorKind.CONST_ATTR,
    CursorKind.NODUPLICATE_ATTR,
    CursorKind.CUDACONSTANT_ATTR,
    CursorKind.CUDADEVICE_ATTR,
    CursorKind.CUDAGLOBAL_ATTR,
    CursorKind.CUDAHOST_ATTR,
    CursorKind.CUDASHARED_ATTR,
    CursorKind.VISIBILITY_ATTR,
    CursorKind.DLLEXPORT_ATTR,
    CursorKind.DLLIMPORT_ATTR,
    CursorKind.PREPROCESSING_DIRECTIVE,
    CursorKind.MACRO_DEFINITION,
    CursorKind.MACRO_INSTANTIATION,
    CursorKind.INCLUSION_DIRECTIVE,
    CursorKind.MODULE_IMPORT_DECL,
    CursorKind.TYPE_ALIAS_TEMPLATE_DECL,
    CursorKind.STATIC_ASSERT,
    CursorKind.FRIEND_DECL,
    CursorKind.OVERLOAD_CANDIDATE,
]

supported_type_kinds = {
    TypeKind.LONG: type_long,
    TypeKind.LONGLONG: type_longlong,
    TypeKind.ULONGLONG: type_ulonglong,
    TypeKind.FLOAT: type_float,
    TypeKind.LONGDOUBLE: type_longdouble,
    TypeKind.INT: type_int,
    TypeKind.DOUBLE: type_double,
    TypeKind.ELABORATED: type_struct,
    TypeKind.RECORD: type_struct,
    TypeKind.TYPEDEF: type_typedef,
    TypeKind.POINTER: type_pointer,
    TypeKind.CONSTANTARRAY: type_constantarray,
    TypeKind.INCOMPLETEARRAY: type_incompletearray,
    TypeKind.VARIABLEARRAY: type_variablearray,
    TypeKind.CHAR_S: type_char,
    TypeKind.BOOL: type_bool,
    TypeKind.FUNCTIONPROTO: type_functionproto,
    TypeKind.VOID: type_void,
}

unsupported_type_kinds = [
    TypeKind.INVALID,
    TypeKind.UNEXPOSED,
    TypeKind.CHAR_U,
    TypeKind.UCHAR,
    TypeKind.CHAR16,
    TypeKind.CHAR32,
    TypeKind.USHORT,
    TypeKind.UINT,
    TypeKind.ULONG,
    TypeKind.UINT128,
    TypeKind.SCHAR,
    TypeKind.WCHAR,
    TypeKind.SHORT,
    TypeKind.INT128,
    TypeKind.NULLPTR,
    TypeKind.OVERLOAD,
    TypeKind.DEPENDENT,
    TypeKind.OBJCID,
    TypeKind.OBJCCLASS,
    TypeKind.OBJCSEL,
    TypeKind.FLOAT128,
    TypeKind.HALF,
    TypeKind.COMPLEX,
    TypeKind.BLOCKPOINTER,
    TypeKind.LVALUEREFERENCE,
    TypeKind.RVALUEREFERENCE,
    TypeKind.ENUM,
    TypeKind.OBJCINTERFACE,
    TypeKind.OBJCOBJECTPOINTER,
    TypeKind.FUNCTIONNOPROTO,
    TypeKind.VECTOR,
    TypeKind.DEPENDENTSIZEDARRAY,
    TypeKind.MEMBERPOINTER,
    TypeKind.AUTO,
    TypeKind.PIPE,
    TypeKind.OCLIMAGE1DRO,
    TypeKind.OCLIMAGE1DARRAYRO,
    TypeKind.OCLIMAGE1DBUFFERRO,
    TypeKind.OCLIMAGE2DRO,
    TypeKind.OCLIMAGE2DARRAYRO,
    TypeKind.OCLIMAGE2DDEPTHRO,
    TypeKind.OCLIMAGE2DARRAYDEPTHRO,
    TypeKind.OCLIMAGE2DMSAARO,
    TypeKind.OCLIMAGE2DARRAYMSAARO,
    TypeKind.OCLIMAGE2DMSAADEPTHRO,
    TypeKind.OCLIMAGE2DARRAYMSAADEPTHRO,
    TypeKind.OCLIMAGE3DRO,
    TypeKind.OCLIMAGE1DWO,
    TypeKind.OCLIMAGE1DARRAYWO,
    TypeKind.OCLIMAGE1DBUFFERWO,
    TypeKind.OCLIMAGE2DWO,
    TypeKind.OCLIMAGE2DARRAYWO,
    TypeKind.OCLIMAGE2DDEPTHWO,
    TypeKind.OCLIMAGE2DARRAYDEPTHWO,
    TypeKind.OCLIMAGE2DMSAAWO,
    TypeKind.OCLIMAGE2DARRAYMSAAWO,
    TypeKind.OCLIMAGE2DMSAADEPTHWO,
    TypeKind.OCLIMAGE2DARRAYMSAADEPTHWO,
    TypeKind.OCLIMAGE3DWO,
    TypeKind.OCLIMAGE1DRW,
    TypeKind.OCLIMAGE1DARRAYRW,
    TypeKind.OCLIMAGE1DBUFFERRW,
    TypeKind.OCLIMAGE2DRW,
    TypeKind.OCLIMAGE2DARRAYRW,
    TypeKind.OCLIMAGE2DDEPTHRW,
    TypeKind.OCLIMAGE2DARRAYDEPTHRW,
    TypeKind.OCLIMAGE2DMSAARW,
    TypeKind.OCLIMAGE2DARRAYMSAARW,
    TypeKind.OCLIMAGE2DMSAADEPTHRW,
    TypeKind.OCLIMAGE2DARRAYMSAADEPTHRW,
    TypeKind.OCLIMAGE3DRW,
    TypeKind.OCLSAMPLER,
    TypeKind.OCLEVENT,
    TypeKind.OCLQUEUE,
    TypeKind.OCLRESERVEID,
    #TypeKind.EXTVECTOR,
    #TypeKind.ATOMIC
]


def list_all(_node):
    print("NODE2:", _node.kind, _node.spelling)
    print("TOKENS:")
    toks = list(_node.get_tokens())
    tok = []

    for i in toks:
        tok.append(i.spelling)
    print(tok)
    child = 0
    for i in _node.get_children():
        print("CHILD:", child)
        child = child + 1
        list_all(i)


def list_all_d(_node, depth):
    print("NODE2, Depth:", _node.kind, _node.spelling, depth)
    print("TOKENS:")
    toks = list(_node.get_tokens())
    tok = []

    for i in toks:
        tok.append(i.spelling)
    print(tok)
    child = 0
    for i in _node.get_children():
        print("CHILD:", child)
        child = child + 1
        list_all_d(i, depth + 1)
