from c2d_clang2c_ast import *
from typing import List, Dict, Set
import copy


class UnaryExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[UnOp] = []

    def visit_UnOp(self, node: UnOp):
        if node.op in ["++", "--"]:
            self.nodes.append(node)
        return self.generic_visit(node)

    def visit_ForStmt(self, node: ForStmt):
        return self.generic_visit(node.body[0])

    def visit_BasicBlock(self, node: BasicBlock):
        return


class UnaryExtractor(NodeTransformer):
    def __init__(self, count=0):
        self.count = count

    def visit_ForStmt(self, node: ForStmt):
        return ForStmt(init=node.init,
                       body=self.generic_visit(node.body[0]),
                       cond=node.cond,
                       iter=node.iter)

    def visit_UnOp(self, node: UnOp):
        if node.op in ["++", "--"]:
            self.count = self.count + 1
            return DeclRefExpr(name="tmp_unop_result" + str(self.count - 1))
        else:
            return node

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []
        for child in node.body:
            lister = UnaryExtractorNodeLister()
            lister.visit(child)
            res = lister.nodes
            post = []
            tmp_count = self.count
            if res is not None:
                for i in range(0, len(res)):
                    if res[i] in node.body:
                        #print("SKIPPING!")
                        continue
                    tmp_name = "tmp_unop_result" + str(tmp_count)
                    newbody.append(
                        DeclStmt(vardecl=[
                            VarDecl(
                                name=tmp_name, type=Int(), init=res[i].lvalue)
                        ]))
                    if res[i].postfix:
                        post.append(res[i])
                    else:
                        newbody.append(
                            BinOp(op="=",
                                  lvalue=DeclRefExpr(name=tmp_name),
                                  rvalue=res[i]))
                    tmp_count = tmp_count + 1
            if isinstance(child, UnOp):
                newbody.append(
                    UnOp(op=child.op, lvalue=self.visit(child.lvalue)))
            else:
                newbody.append(self.visit(child))
            for i in post:
                newbody.append(i)
        return BasicBlock(body=newbody)


class IndicesExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[ArraySubscriptExpr] = []

    def visit_ArraySubscriptExpr(self, node: ArraySubscriptExpr):
        #if not isinstance(node.index,IntLiteral):
        self.nodes.append(node)
        return self.generic_visit(node)

    def visit_BasicBlock(self, node: BasicBlock):
        return

class ParenExprRemover(NodeTransformer):
    def visit_ArraySubscriptExpr(self, node: ArraySubscriptExpr):
        if isinstance(node.unprocessed_name, ArraySubscriptExpr):
            node.unprocessed_name = self.visit(node.unprocessed_name)
            return node
            
        tmp = node.unprocessed_name
        while isinstance(tmp, ParenExpr):
            tmp = tmp.expr

        node.unprocessed_name = tmp
        return node

class IndicesExtractor(NodeTransformer):
    def __init__(self, count=0):
        self.count = count

    def visit_ArraySubscriptExpr(self, node: ArraySubscriptExpr):

        #if isinstance(node.index,IntLiteral):
        #    return node
        if not hasattr(self, "count"):
            self.count = 0
        else:
            self.count = self.count + 1
        tmp = self.count
        return ArraySubscriptExpr(
            name=node.name,
            indices=node.indices,
            type=node.type,
            unprocessed_name=self.visit(node.unprocessed_name),
            index=DeclRefExpr(name="tmp_index_" + str(tmp - 1), type=Int()))

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            # res = [node for node in Node.walk(child) if isinstance(node, ArraySubscriptExpr)]
            lister = IndicesExtractorNodeLister()
            lister.visit(child)
            res = lister.nodes
            temp = self.count
            if res is not None:
                for i in range(0, len(res)):
                    tmp_name = "tmp_index_" + str(temp)
                    temp = temp + 1
                    newbody.append(
                        DeclStmt(vardecl=[VarDecl(name=tmp_name, type=Int())]))
                    newbody.append(
                        BinOp(op="=",
                              lvalue=DeclRefExpr(name=tmp_name),
                              rvalue=res[i].index))
            newbody.append(self.visit(child))
        return BasicBlock(body=newbody)

class CompoundArgumentsExtractor(NodeTransformer):
    def __init__(self):
        self.count = 0

    def compute_patch(self, node: CallExpr):
        if node.name.name in ["malloc", "expf", "powf", "sqrt", "cbrt", "printf"]:
            return []

        inits = []
        for i in range(len(node.args)):
            if not isinstance(node.args[i], UnOp) and not isinstance(node.args[i], BinOp):
                continue

            var_name = "tmp_arg_" + str(self.count)
            self.count += 1

            var = DeclRefExpr(name=var_name, type=node.args[i].type)
            init_var = DeclStmt(vardecl=[VarDecl(name=var_name, type=node.args[i].type, init=node.args[i])])

            node.args[i] = var
            inits.append(init_var)
        
        return inits

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []
        for child in node.body:
            self.visit(child)
            if isinstance(child, CallExpr):
                newbody += self.compute_patch(child)

            if isinstance(child, BinOp) and isinstance(child.rvalue, CallExpr):
                newbody += self.compute_patch(child.rvalue)

            newbody.append(child)

        return BasicBlock(body=newbody)


class Calloc2Malloc(NodeTransformer):
    def visit_CallExpr(self, node: CallExpr):
        if node.name.name == "calloc":
            node.name = DeclRefExpr(name="malloc")
            node.args[0] = BinOp(op="*", lvalue=copy.deepcopy(node.args[0]), rvalue=copy.deepcopy(node.args[1]))
            del node.args[1]

        return node

class MallocForceInitializer(NodeTransformer):
    def __init__(self):
        self.mallocs = dict()
        self.first_scan = True

    def visit_BasicBlock(self, node: BasicBlock):
        if self.first_scan:
            for child in node.body:
                self.visit(child)
                if isinstance(child, BinOp) and isinstance(child.rvalue, CallExpr) and isinstance(child.rvalue.name, DeclRefExpr) and child.rvalue.name.name == "malloc":
                    tmp = child.lvalue

                    while isinstance(tmp, ParenExpr) or isinstance(tmp, ArraySubscriptExpr):
                        if isinstance(tmp, ParenExpr):
                            tmp = tmp.expr
                        elif isinstance(tmp, ArraySubscriptExpr):
                            tmp = tmp.unprocessed_name

                    if not isinstance(tmp, DeclRefExpr):
                        print("WARNING cannot identify ", tmp)

                    self.mallocs[tmp.name] = child
            
            return node

        else:
            newbody = []
            for child in node.body:
                self.visit(child)
                newbody.append(child)
                if child in self.mallocs.values():
                    newbody.append(
                        BinOp(
                                op="=",
                                lvalue=ArraySubscriptExpr(
                                    name="",
                                    indices="UNDEF",
                                    type=Double(),
                                    unprocessed_name=child.lvalue,
                                    index=IntLiteral(value="0"),
                                ),
                                rvalue=IntLiteral(value="0")
                            )
                    )

            node.body = newbody
            return node
    
    def visit_FuncDecl(self, node: FuncDecl):
        self.mallocs = dict()
        self.first_scan = True

        self.visit(node.body)

        self.first_scan = False
        self.visit(node.body)

        return node

class ArrayPointerExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[DeclRefExpr] = []

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        #if not isinstance(node.index,IntLiteral):
        self.nodes.append(node)
        return self.generic_visit(node)

    def visit_BasicBlock(self, node: BasicBlock):
        return

class ArrayPointerExtractor(NodeTransformer):
    def __init__(self, global_array_map):
        self.array_map = dict()
        self.global_array_map = global_array_map
        self.count = 0

    def pointer_increment(self, node: BinOp):
        name = node.lvalue.name

        if name not in self.array_map:
            print("Warning undeclared array in ArrayPointerExtractor: " + name)
            return self.generic_visit(node)

        if not isinstance(node.rvalue, BinOp):
            return self.generic_visit(node)

        rval = node.rvalue.rvalue
        lval = node.rvalue.lvalue
        op = node.rvalue.op

        if not isinstance(lval, DeclRefExpr):
            return self.generic_visit(node)

        start_ptr_name = self.array_map[name]
        start_ptr = DeclRefExpr(name=start_ptr_name, type=Int())
        binop = BinOp(op=op, lvalue=copy.deepcopy(start_ptr), rvalue=rval)

        return BinOp(op="=", lvalue=start_ptr, rvalue=binop)

    def pointer_assignment(self, node: BinOp):
        if not hasattr(node.lvalue, "type") or not hasattr(node.rvalue, "type"):
            return self.generic_visit(node)

        ltype = node.lvalue.type
        rtype = node.rvalue.type

        if not isinstance(ltype, Pointer) or ltype != rtype:
            return self.generic_visit(node)

        lval = self.visit(node.lvalue)
        rval = self.visit(node.rvalue)

        start_ptr_name = self.array_map.get(rval.name)
        if start_ptr_name is None:
            print("Warning undeclared array in ArrayPointerExtractor: " + rval.name + ", ", rval)
            return self.generic_visit(node)

        start_ptr = DeclRefExpr(name=start_ptr_name, type=Int())
        pointer_binop = BinOp(op="+", lvalue=rval, rvalue=start_ptr)
        binop = BinOp(op="=", lvalue=lval, rvalue=pointer_binop)

        return binop

    def visit_ArraySubscriptExpr(self, node: ArraySubscriptExpr):
        if isinstance(node.unprocessed_name, ParenExpr):
            node.unprocessed_name = node.unprocessed_name.expr
            return self.visit(node)

        if isinstance(node.unprocessed_name, ArraySubscriptExpr):
            return ArraySubscriptExpr(
                name=node.name,
                indices=node.indices,
                type=node.type,
                unprocessed_name=self.visit(node.unprocessed_name),
                index=node.index)

        name = node.unprocessed_name.name

        ptr_name = self.array_map.get(name)
        if ptr_name is None:
            print("Warning undeclared array in ArrayPointerExtractor: " + node.name)
            return node

        ptr_index = DeclRefExpr(name=ptr_name, type=Int())
        binop = BinOp(op="+", lvalue=node.index, rvalue=ptr_index)
        return ArraySubscriptExpr(
            name=node.name,
            indices=node.indices,
            type=node.type,
            unprocessed_name=self.visit(node.unprocessed_name),
            index=binop)

    def visit_BinOp(self, node: BinOp):
        if node.op != "=" :
            return self.generic_visit(node)

        lval = node.lvalue
        while isinstance(lval, ParenExpr):
            lval = lval.expr

        node.lvalue = lval

        # a[i] = something
        if isinstance(lval, ArraySubscriptExpr):
            return self.pointer_assignment(node)

        # unknown lvalue
        if not isinstance(lval, DeclRefExpr):
            return self.generic_visit(node)

        # a = b
        if isinstance(node.rvalue, DeclRefExpr):
            return self.pointer_assignment(node)

        return self.pointer_increment(node)

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            newbody.append(self.visit(child))
            if not isinstance(child, DeclStmt) or len(child.vardecl) != 1:
                continue
                
            # TODO account for multiple declarations
            vardecl = child.vardecl[0]

            if not hasattr(vardecl, "init"):
                continue

            varname = "tmp_array_ptr_" + vardecl.name + "_" + str(self.count)

            if isinstance(vardecl.init, CallExpr) and isinstance(vardecl.init.name, DeclRefExpr) and vardecl.init.name.name == "malloc":
                self.array_map[vardecl.name] = varname
                self.count += 1
                newbody.append(VarDecl(name=self.array_map[vardecl.name], type=Int(), init=IntLiteral(value="0")))
                continue
            
            expr = vardecl.init
            while isinstance(expr, ParenExpr):
                expr = expr.expr

            if isinstance(expr, DeclRefExpr) and expr.name in self.array_map:
                self.array_map[vardecl.name] = varname
                self.count += 1
                newbody.append(VarDecl(name=self.array_map[vardecl.name], type=Int(), init=IntLiteral(value="0")))
                continue

        return BasicBlock(body=newbody)

    def visit_FuncDecl(self, node: FuncDecl):
        if node.name != "main":
            return node

        self.array_map = dict()

        self.generic_visit(node)
        self.global_array_map[node.name] = self.array_map

        return node


class ArrayPointerReset(NodeTransformer):
    def __init__(self, global_array_map):
        self.array_map = dict()
        self.global_array_map = global_array_map

    def reset_pointer(self, node):
        if not isinstance(node, BinOp):
            return []
        
        if not isinstance(node.lvalue, DeclRefExpr):
            return []
        
        ptr_name = self.array_map.get(node.lvalue.name)
        if ptr_name is None:
            return []
        
        return [BinOp(op="=", lvalue=DeclRefExpr(name=ptr_name, type=Int()), rvalue=IntLiteral(value="0"))]

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            newbody.append(child)
            newbody += self.reset_pointer(child)
        
        return BasicBlock(body=newbody)

    def visit_FuncDecl(self, node: FuncDecl):
        self.array_map = self.global_array_map.get(node.name, dict())
        self.generic_visit(node)

        return node

class InitExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[VarDecl] = []

    def visit_ForStmt(self, node: ForStmt):
        return

    def visit_VarDecl(self, node: VarDecl):
        if hasattr(node, "init"):
            self.nodes.append(node)
        return self.generic_visit(node)

    def visit_BasicBlock(self, node: BasicBlock):
        return


class InsertMissingBasicBlocks(NodeTransformer):
    def insert_missing_block(self, body):
        assert isinstance(body, list)
        if isinstance(body[0], BasicBlock):
            return body
        return [BasicBlock(body=body)]

    def visit_ForStmt(self, node: ForStmt):
        node.body = self.insert_missing_block(node.body)
        return self.generic_visit(node)

    def visit_DoStmt(self, node: DoStmt):
        node.body = self.insert_missing_block(node.body)
        return self.generic_visit(node)

    def visit_WhileStmt(self, node: WhileStmt):
        node.body = self.insert_missing_block(node.body)
        return self.generic_visit(node)

    def visit_IfStmt(self, node: IfStmt):
        node.body_if = self.insert_missing_block(node.body_if)
        if hasattr(node, "body_else"):
            node.body_else = self.insert_missing_block(node.body_else)
        return self.generic_visit(node)


class InitExtractor(NodeTransformer):
    def __init__(self, count=0):
        self.count = count

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            # res = [node for node in Node.walk(child) if isinstance(node, ArraySubscriptExpr)]
            lister = InitExtractorNodeLister()
            lister.visit(child)
            res = lister.nodes
            temp = self.count
            newbody.append(self.visit(child))
            if res is None:
                continue

            for i in res:
                '''
                # check if the init is malloc
                if isinstance(i.init, CallExpr) and isinstance(i.init.name, DeclRefExpr) and i.init.name.name != "malloc":
                    # add also an init with a concrete value s.t. the transient is always initialized
                    newbody.append(
                        BinOp(op="=",
                                lvalue=DeclRefExpr(name=i.name),
                                rvalue=IntLiteral(value="0")))
                '''

                newbody.append(
                    BinOp(op="=",
                            lvalue=DeclRefExpr(name=i.name),
                            rvalue=i.init))

        return BasicBlock(body=newbody)


class CallExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[CallExpr] = []

    def visit_ForStmt(self, node: ForStmt):
        return

    def visit_CallExpr(self, node: CallExpr):
        if node.name.name not in ["malloc", "expf", "powf", "sqrt", "cbrt"]:
            self.nodes.append(node)
        return self.generic_visit(node)

    def visit_BasicBlock(self, node: BasicBlock):
        return


class CallExtractor(NodeTransformer):
    def __init__(self, count=0):
        self.count = count

    def visit_CallExpr(self, node: CallExpr):

        #if isinstance(node.index,IntLiteral):
        #    return node
        if not hasattr(self, "count"):
            self.count = 0
        else:
            self.count = self.count + 1
        tmp = self.count
        if node.name.name in ["malloc", "expf", "powf", "sqrt", "cbrt"]:
            return node
        return DeclRefExpr(name="tmp_call_" + str(tmp - 1))

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            # res = [node for node in Node.walk(child) if isinstance(node, ArraySubscriptExpr)]
            lister = CallExtractorNodeLister()
            lister.visit(child)
            res = lister.nodes
            for i in res:
                if i == child:
                    res.pop(res.index(i))
            temp = self.count
            if res is not None:
                for i in range(0, len(res)):
                    print("CALL:", res[i].name)
                    newbody.append(
                        DeclStmt(vardecl=[
                            VarDecl(name="tmp_call_" + str(temp),
                                    type=res[i].type)
                        ]))
                    newbody.append(
                        BinOp(op="=",
                              lvalue=DeclRefExpr(name="tmp_call_" + str(temp),
                                                 type=res[i].type),
                              rvalue=res[i]))
            if isinstance(child, CallExpr):
                new_args = []
                for i in child.args:
                    new_args.append(self.visit(i))
                new_child = CallExpr(type=child.type,
                                     name=child.name,
                                     args=new_args)
                newbody.append(new_child)
            else:
                newbody.append(self.visit(child))

        return BasicBlock(body=newbody)


class PowerOptimization(NodeTransformer):
    def visit_CallExpr(self, node: CallExpr):
        if node.name.name != "pow":
            return node

        if len(node.args) != 2:
            return node

        if not isinstance(node.args[1], IntLiteral):
            return node

        value = node.args[1].value[0]

        if value != '2':
            return node

        lnode = copy.deepcopy(node.args[0])
        rnode = copy.deepcopy(node.args[0])

        return BinOp(op="*", lvalue=lnode, rvalue=rnode)


class CondExtractorNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[Node] = []

    def visit_ForStmt(self, node: ForStmt):
        return

    def visit_IfStmt(self, node: IfStmt):
        self.nodes.append(node.cond[0])

    def visit_BasicBlock(self, node: BasicBlock):
        return


class CondExtractor(NodeTransformer):
    def __init__(self, count=0):
        self.count = count

    def visit_IfStmt(self, node: IfStmt):

        if not hasattr(self, "count"):
            self.count = 0
        else:
            self.count = self.count + 1
        tmp = self.count

        cond = [
            BinOp(op="!=",
                  lvalue=DeclRefExpr(name="tmp_if_" + str(tmp - 1)),
                  rvalue=IntLiteral(value="0"))
        ]
        body_if = [self.visit(node.body_if[0])]
        if hasattr(node, "body_else"):
            body_else = [self.visit(node.body_else[0])]
            return IfStmt(cond=cond, body_if=body_if, body_else=body_else)
        else:
            return IfStmt(cond=cond, body_if=body_if)

    def visit_BasicBlock(self, node: BasicBlock):
        newbody = []

        for child in node.body:
            lister = CondExtractorNodeLister()
            lister.visit(child)
            res = lister.nodes
            temp = self.count
            if res is not None:
                for i in range(0, len(res)):
                    newbody.append(
                        DeclStmt(vardecl=[
                            VarDecl(name="tmp_if_" + str(temp), type=Int())
                        ]))
                    newbody.append(
                        BinOp(op="=",
                              lvalue=DeclRefExpr(name="tmp_if_" + str(temp)),
                              rvalue=res[i]))
            newbody.append(self.visit(child))

        return BasicBlock(body=newbody)


class ForDeclarerNodeLister(NodeVisitor):
    def __init__(self):
        self.nodes: List[Node] = []

    def visit_ForStmt(self, node: ForStmt):
        if isinstance(node.init[0], BinOp):# for(int i=0;) for (i=0;)
            self.nodes.append(node.init[0])

    def visit_BasicBlock(self, node: BasicBlock):
        return


class ForDeclarer(NodeTransformer):
    def __init__(self):
        self.count = 0
        self.name_mapping = {}

    def visit_BasicBlock(self, node: BasicBlock):
        # make sure name mapping gets reverted properly when exiting contexts
        prev = self.name_mapping.copy()
        newbody = []
        for child in node.body:
            lister = ForDeclarerNodeLister()
            lister.visit(child)
            res = lister.nodes
            if res is not None:
                for i in range(0, len(res)):
                    #print("FOREXTRABINOP")
                    newbody.append(res[i])
            newbody.append(self.visit(child))
        self.name_mapping = prev
        return BasicBlock(body=newbody)

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        if self.name_mapping.get(node.name) is None:
            return node
        else:
            return DeclRefExpr(name=self.name_mapping[node.name])

    def visit_ForStmt(self, node: ForStmt):
        if isinstance(node.body, list):
            node.body = node.body[0]
        if isinstance(node.init[0], BinOp):
            self.count = self.count + 1
            assert isinstance(
                node.init[0].lvalue,
                DeclRefExpr), "expecting lvalue of binop to be a declRefExpr"

            self.name_mapping[node.init[0].lvalue.name] = "tmp_for_" + str(
                self.count)
            return ForStmt(init=[
                DeclStmt(vardecl=[
                    VarDecl(name="tmp_for_" + str(self.count),
                            type=Int(),
                            init=node.init[0].rvalue)
                ])
            ],
                           cond=[self.generic_visit(node.cond[0])],
                           body=[self.generic_visit(node.body)],
                           iter=[self.generic_visit(node.iter[0])])
        elif isinstance(node.init[0], DeclStmt):
            return self.generic_visit(node)


class UnaryToBinary(NodeTransformer):
    def visit_UnOp(self, node: UnOp):
        if node.op == "++":
            return BinOp(op="=",
                         lvalue=node.lvalue,
                         rvalue=BinOp(op="+",
                                      lvalue=node.lvalue,
                                      rvalue=IntLiteral(value="1")))
        elif node.op == "--":
            return BinOp(op="=",
                         lvalue=node.lvalue,
                         rvalue=BinOp(op="-",
                                      lvalue=node.lvalue,
                                      rvalue=IntLiteral(value="1")))
        else:
            return self.generic_visit(node)


class CompoundToBinary(NodeTransformer):
    def visit_CompoundAssignOp(self, node: CompoundAssignOp):
        newop = (node.op).replace("=", "")
        return BinOp(op="=",
                     lvalue=node.lvalue,
                     rvalue=BinOp(op=newop,
                                  lvalue=node.lvalue,
                                  rvalue=node.rvalue))


class UnaryReferenceAndPointerRemover(NodeTransformer):
    def visit_ParenExpr(self, node: ParenExpr):
        # check for unpacked structs that we are dereferencing
        if not isinstance(node.expr, UnOp):
            return self.generic_visit(node)

        if not isinstance(node.expr.lvalue, list):
            return self.generic_visit(node)
        
        return list(map(lambda x: ParenExpr(expr=self.generic_visit(x), type=x.type), node.expr.lvalue))

    def visit_UnOp(self, node: UnOp):
        if node.op == "*" or node.op == "&":
            return self.generic_visit(node.lvalue)
        else:
            return self.generic_visit(node)


class FindOutputNodesVisitor(NodeVisitor):
    def __init__(self):
        self.nodes: List[DeclRefExpr] = []

    def visit_ParenExpr(self, node: ParenExpr):
        self.visit(node.expr)

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        self.nodes.append(node)

    def visit_UnOp(self, node: UnOp):
        if node.op == "*":
            self.visit(node.lvalue)

    def visit_ArraySubscriptExpr(self, node: ArraySubscriptExpr):
        self.visit(node.unprocessed_name)

    def visit_BinOp(self, node: BinOp):
        if node.op == "=":
            self.visit(node.lvalue)
            if isinstance(node.rvalue, BinOp):
                self.visit(node.rvalue)

    #def visit_TernaryExpr(self, node: TernaryExpr):
    #    used_vars_condition = [node for node in walk(node.cond) if isinstance(node, DeclRefExpr)]
    #    used_vars_left = [node for node in walk(node.left) if isinstance(node, DeclRefExpr)]
    #    used_vars_right = [node for node in walk(node.right) if isinstance(node, DeclRefExpr)]
    #    self.nodes = self.nodes + used_vars_condition


class FindInputNodesVisitor(NodeVisitor):
    def __init__(self):
        self.nodes: List[DeclRefExpr] = []

    def visit_ParenExpr(self, node: ParenExpr):
        self.visit(node.expr)

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        self.nodes.append(node)

    def visit_BinOp(self, node: BinOp):
        if node.op == "=":
            if isinstance(node.lvalue, DeclRefExpr):
                pass
            if isinstance(node.lvalue, ArraySubscriptExpr):
                tmp = node.lvalue
                while isinstance(tmp, ArraySubscriptExpr):
                    self.visit(tmp.index)
                    tmp = tmp.unprocessed_name
                if isinstance(tmp, DeclRefExpr):
                    pass
        else:
            self.visit(node.lvalue)
        self.visit(node.rvalue)


class FunctionLister(NodeVisitor):
    def __init__(self) -> None:
        self.function_names: Set[str] = set()
        self.defined_function_names: Set[str] = set()
        self.undefined_function_names: Set[str] = set()
        self.function_is_void: Set[str] = set()

    def visit_AST(self, node: AST):
        self.generic_visit(node)
        self.undefined_function_names = self.function_names.difference(
            self.defined_function_names)

    def visit_FuncDecl(self, node: FuncDecl):
        self.function_names.add(node.name)

        if node.body is not None and node.body != []:
            self.defined_function_names.add(node.name)

        if isinstance(node.result_type, Void):
            self.function_is_void.add(node.name)

    def is_defined(self, function_name: str) -> bool:
        return function_name in self.defined_function_names

    def is_declared(self, function_name: str) -> bool:
        return function_name in self.function_names

    def is_void(self, function_name: str) -> bool:
        return function_name in self.function_is_void


class MoveReturnValueToArguments(NodeTransformer):
    """
    expects: no class method calls
    """
    def visit_AST(self, node: AST) -> None:
        self.function_lister = FunctionLister()
        self.function_lister.visit(node)
        return self.generic_visit(node)

    def visit_FuncDecl(self, node: FuncDecl):
        if self.function_lister.is_defined(node.name):
            if not isinstance(node.result_type, Void):
                #node.args.append(ParmDecl(name = "c2d_retval", type = Pointer(pointee_type = node.result_type), lineno = node.lineno))
                node.args.append(
                    ParmDecl(name="c2d_retval",
                             type=node.result_type,
                             lineno=node.lineno))
                node.result_type = Void()

        return self.generic_visit(node)

    def visit_CallExpr(self, node: CallExpr):
        if self.function_lister.is_defined(node.name.name):
            if not self.function_lister.is_void(node.name.name):
                node.args.append(
                    DeclRefExpr(name="NULL",
                                type=Pointer(pointee_type=Void())))
                node.type = Void()

        return self.generic_visit(node)

    def visit_BinOp(self, node: BinOp):
        if isinstance(node.rvalue, CallExpr):
            if self.function_lister.is_defined(node.rvalue.name.name):
                #reference = UnOp(lvalue = node.lvalue, op = "&", postfix = False, type = Pointer(pointee_type = node.lvalue.type))
                reference = node.lvalue
                node.rvalue.args.append(reference)
                return self.generic_visit(node.rvalue)

        return self.generic_visit(node)

    def visit_RetStmt(self, node: RetStmt):
        if hasattr(node, "ret_expr"):
            return_type = node.ret_expr.type
            #left = UnOp(op = "*", postfix = False, type = Pointer(pointee_type = return_type),
            #            lvalue = DeclRefExpr(name = "c2d_retval", type = return_type))
            # TODO: implement using pointers
            left = DeclRefExpr(name="c2d_retval", type=return_type)
            assignment = BinOp(op="=",
                               lvalue=left,
                               rvalue=node.ret_expr,
                               type=return_type)
            return [assignment, RetStmt()]

        return self.generic_visit(node)


class FlattenStructs(NodeTransformer):
    def __init__(self) -> None:
        self.structdefs: Dict[str, StructDecl] = {}

    def struct_is_defined(self, struct_name):
        return struct_name in self.structdefs.keys()

    def visit_AST(self, node: AST):
        self.structdefs = {sd.name: sd for sd in node.structdefs}
        return self.generic_visit(node)

    def visit_StructDecl(self, node: StructDecl):
        replacement_fields = []
        for field in node.fields:
            if field.type.is_struct_like():
                nested_struct_name = field.type.get_chain_end().name
                if self.struct_is_defined(nested_struct_name):
                    nested_struct_fields = self.structdefs[
                        nested_struct_name].fields

                    for nested_field in nested_struct_fields:
                        replacement_fields.append(
                            FieldDecl(name=field.name + "_" +
                                      nested_field.name,
                                      type=field.type.inject_type(
                                          nested_field.type)))
                else:
                    replacement_fields.append(field)

            else:
                replacement_fields.append(field)

        return StructDecl(name=node.name, fields=replacement_fields)


class ReplaceStructDeclStatements(NodeTransformer):
    def __init__(self):
        self.structdefs: Dict[str, StructDecl] = {}

    def struct_is_defined(self, struct_name):
        return struct_name in self.structdefs.keys()

    def get_struct(self, struct_name: str):
        return self.structdefs.get(struct_name)

    def get_field_replacement_name(self, struct_type_name: str,
                                   struct_variable_name: str, field_name: str):
        return "c2d_struct_" + struct_type_name + "_" + struct_variable_name + "_" + field_name

    def split_struct_type(self, struct_like_type,
                          var_name) -> Dict[str, Tuple[str, Type]]:
        if isinstance(struct_like_type, Struct):
            if not self.struct_is_defined(struct_like_type.name):
                return None
            defined_struct = self.get_struct(struct_like_type.name)
            return {
                field.name:
                (self.get_field_replacement_name(defined_struct.name, var_name,
                                                 field.name), field.type)
                for field in defined_struct.fields
            }

        if isinstance(struct_like_type, ConstantArray):
            splits = self.split_struct_type(struct_like_type.element_type,
                                            var_name)
            if splits is not None:
                return {
                    field_name: (split_name + "_arr",
                                 ConstantArray(size=struct_like_type.size,
                                               element_type=split_type))
                    for (field_name, (split_name,
                                      split_type)) in splits.items()
                }
            else:
                return None

        if isinstance(struct_like_type, Pointer):
            splits = self.split_struct_type(struct_like_type.pointee_type,
                                            var_name)
            if splits is not None:
                return {
                    field_name:
                    (split_name + "_ptr", Pointer(pointee_type=split_type))
                    for (field_name, (split_name,
                                      split_type)) in splits.items()
                }
            else:
                return None

        raise Exception("split_struct_type expects struct like type")

    def replace_container_expr(self, container_expr: Expression,
                               desired_field: str):
        if isinstance(container_expr, DeclRefExpr):
            replacement_name, replacement_type = self.split_struct_type(
                container_expr.type, container_expr.name)[desired_field]

            if isinstance(replacement_type, Pointer):
                # need to wrap in ParenExpr when deallocating pointer
                return ParenExpr(
                        expr=UnOp(
                            op="*",
                            postfix=False,
                            type=replacement_type,
                            lvalue=DeclRefExpr(name=replacement_name, type=replacement_type)
                        ),
                        type=replacement_type
                    )

            return DeclRefExpr(name=replacement_name, type=replacement_type)
        if isinstance(container_expr, ArraySubscriptExpr):
            replacement = copy.deepcopy(container_expr)
            replacement.unprocessed_name = self.replace_container_expr(
                container_expr.unprocessed_name, desired_field)
            if isinstance(replacement.unprocessed_name.type, Array):
                replacement.type = replacement.unprocessed_name.type.element_type
            elif isinstance(replacement.unprocessed_name.type, Pointer):
                replacement.type = replacement.unprocessed_name.type.pointee_type
            else:
                print("unsupported type in array subscript expr ", replacement.unprocessed_name.type)
                raise

            return replacement
        if isinstance(container_expr, MemberRefExpr):
            replacement = copy.deepcopy(container_expr)
            replacement.name += "_" + desired_field
            return self.visit(replacement)
        if isinstance(container_expr, UnOp):
            replacement = copy.deepcopy(container_expr)
            replacement.lvalue = self.replace_container_expr(
                container_expr.lvalue, desired_field)
            replacement.type = container_expr.type.inject_type(
                replacement.lvalue.type)
            return replacement
        if isinstance(container_expr, ParenExpr):
            expr = copy.deepcopy(container_expr.expr)
            out_expr = self.replace_container_expr(expr, desired_field)
            return out_expr

        raise Exception("cannot replace container expression: ",
                        container_expr, " at line ", container_expr.lineno)

    def visit_AST(self, node: AST):
        self.structdefs = {sd.name: sd for sd in node.structdefs}
        return self.generic_visit(node)

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        if hasattr(node, "type") and node.type.is_struct_like():
            splits = self.split_struct_type(node.type, node.name)
            if splits is not None:
                return [
                    DeclRefExpr(name=n, type=t) for (n, t) in splits.values()
                ]

        return self.generic_visit(node)

    def visit_BinOp(self, node: BinOp):
        if hasattr(node.lvalue, "type") and hasattr(node.rvalue, "type"):
            if node.lvalue.type.is_struct_like():
                if node.lvalue.type == node.rvalue.type:
                    # assign a struct to another
                    struct = node.lvalue.type.get_chain_end()

                    if node.op == "=":
                        replacement_statements = []

                        fields = self.get_struct(struct.name).fields
                        if fields is not None:
                            for f in fields:
                                l_member_ref = MemberRefExpr(
                                    name=f.name,
                                    type=node.lvalue.type.inject_type(f.type),
                                    containerexpr=node.lvalue)
                                r_member_ref = MemberRefExpr(
                                    name=f.name,
                                    type=node.rvalue.type.inject_type(f.type),
                                    containerexpr=node.rvalue)
                                binop = BinOp(op="=",
                                              type=f.type,
                                              lvalue=l_member_ref,
                                              rvalue=r_member_ref)
                                replacement_statements.append(binop)
                            return [
                                self.visit(s) for s in replacement_statements
                            ]
                elif isinstance(node.rvalue, CallExpr) and node.rvalue.name.name == "malloc":
                    # call malloc on every field
                    mallocCall = CallExpr(
                            type=Pointer(pointee_type=Void()),
                            args=[IntLiteral(value="1")],
                            name=DeclRefExpr(name="malloc", type=Pointer(pointee_type=Void()))
                        )

                    struct = node.lvalue.type.get_chain_end()

                    if node.op == "=":
                        replacement_statements = []

                        fields = self.get_struct(struct.name).fields
                        if fields is not None:
                            for f in fields:
                                l_member_ref = MemberRefExpr(
                                    name=f.name,
                                    type=node.lvalue.type.inject_type(f.type),
                                    containerexpr=node.lvalue)
                                binop = BinOp(op="=",
                                              type=f.type,
                                              lvalue=l_member_ref,
                                              rvalue=copy.deepcopy(mallocCall))
                                replacement_statements.append(binop)
                            return [
                                self.visit(s) for s in replacement_statements
                            ]
                    return []

        return self.generic_visit(node)

    def visit_VarDecl(self, node: VarDecl):
        # only process structs
        if not node.type.is_struct_like():
            return self.generic_visit(node)

        # only valid and defined structs
        splits = self.split_struct_type(node.type, node.name)
        if splits is None:
            return self.generic_visit(node)

        if not hasattr(node, "init"):
            return self.generic_visit(node)

        # null initialization to pointer
        if isinstance(node.init, IntLiteral) and node.init.value == ['0']:
            return [VarDecl(name=n, type=t, init=IntLiteral(value="0")) for (n, t) in splits.values()]

        # check if the struct is initialized as malloc
        if not isinstance(node.init, CallExpr):
            return [VarDecl(name=n, type=t) for (n, t) in splits.values()]

        if node.init.name.name != "malloc":
            return [VarDecl(name=n, type=t) for (n, t) in splits.values()]

        mallocCall =CallExpr(
                type=Pointer(pointee_type=Void()),
                args=[IntLiteral(value="1")],
                name=DeclRefExpr(name="malloc", type=Pointer(pointee_type=Void()))
            )

        return [VarDecl(name=n, type=t, init=mallocCall) for (n, t) in splits.values()]


    def visit_DeclStmt(self, node: DeclStmt):
        replacement_stmts = []

        for var_decl in node.vardecl:
            replacement_var_decls = self.as_list(self.visit(var_decl))
            replacement_stmts += [
                DeclStmt(vardecl=[vd]) for vd in replacement_var_decls
            ]

        return replacement_stmts

    def visit_MemberRefExpr(self, node: MemberRefExpr):
        if hasattr(node.containerexpr,
                   "type") and node.containerexpr.type.is_struct_like():
            struct_like_type = node.containerexpr.type.get_chain_end()
            if self.struct_is_defined(struct_like_type.name):
                return self.replace_container_expr(node.containerexpr,
                                                   node.name)

        return self.generic_visit(node)

    def visit_ParmDecl(self, node: ParmDecl):
        if node.type.is_struct_like():
            splits = self.split_struct_type(node.type, node.name)

            return [ParmDecl(name=n, type=t) for (n, t) in splits.values()]

        return self.generic_visit(node)


class CXXClassToStruct(NodeTransformer):
    def __init__(self):
        self.replacement_structs: List[StructDecl] = []
        self.exported_functions: List[FuncDecl] = []

    def get_class_type_replacement_name(self, class_name):
        return "c2d_class_as_struct_" + class_name

    def get_class_type_replacement(self, class_like_type) -> Type:

        if isinstance(class_like_type, Class):
            return Struct(name=self.get_class_type_replacement_name(
                class_like_type.name))

        if isinstance(class_like_type, Pointer):
            t = self.get_class_type_replacement(class_like_type.pointee_type)
            if t is not None:
                return Pointer(pointee_type=t)

        if isinstance(class_like_type, ConstantArray):
            t = self.get_class_type_replacement(class_like_type.element_type)
            if t is not None:
                return ConstantArray(size=class_like_type.size, element_type=t)

        return None

    def is_class_like(self, type) -> bool:
        if isinstance(type, ConstantArray):
            return self.is_class_like(type.element_type)
        if isinstance(type, Pointer):
            return self.is_class_like(type.pointee_type)
        return isinstance(type, Class)

    def get_class_variable_replacement_name(self, var_name):
        return var_name

    def get_method_replacement_name(self, method_name, class_name):
        return "c2d_" + class_name + "_method_" + method_name

    def visit_AST(self, node: AST):
        transformed_ast = self.generic_visit(node)
        transformed_ast.structdefs += self.replacement_structs
        transformed_ast.funcdefs += self.exported_functions

        return transformed_ast

    def visit_ClassDecl(self, node: ClassDecl):
        replacement_struct = StructDecl(
            name=self.get_class_type_replacement_name(node.name),
            fields=node.fields)
        self.replacement_structs.append(replacement_struct)

        self.generic_visit(node)
        return None

    def visit_CXXMethod(self, node: CXXMethod):
        if node.body is None:
            return None

        node = self.generic_visit(node)

        replacement_struct_name = self.get_class_type_replacement_name(
            node.parent_class_type.name)
        this_arg = ParmDecl(
            name="c2d_this",
            type=Pointer(pointee_type=Struct(name=replacement_struct_name)))

        replacement_function_name = self.get_method_replacement_name(
            node.name, node.parent_class_type.name)
        replacement_function = FuncDecl(name=replacement_function_name,
                                        args=[this_arg] + node.args,
                                        body=node.body)
        self.exported_functions.append(replacement_function)
        return None

    def visit_MemberRefExpr(self, node: MemberRefExpr):
        """
        Replace any class field accesses with accesses to their replacement struct
        """
        node = self.generic_visit(node)

        return node

    def visit_DeclRefExpr(self, node: DeclRefExpr):
        """
        Replace any references to a class declaration with references to their replacement struct
        """
        if hasattr(node, "type") and self.is_class_like(node.type):
            replacement_struct_type = self.get_class_type_replacement(
                node.type)
            return DeclRefExpr(name=self.get_class_variable_replacement_name(
                node.name),
                               type=replacement_struct_type)

        return self.generic_visit(node)

    def visit_VarDecl(self, node: VarDecl):

        if self.is_class_like(node.type):
            replacement_struct_type = self.get_class_type_replacement(
                node.type)
            return VarDecl(name=self.get_class_variable_replacement_name(
                node.name),
                           type=replacement_struct_type)

        return self.generic_visit(node)

    def visit_CXXThisExpr(self, node: CXXThisExpr):
        replacement_struct_name = self.get_class_type_replacement_name(
            node.type.pointee_type.name)
        return DeclRefExpr(
            name="c2d_this",
            type=Pointer(pointee_type=Struct(name=replacement_struct_name)))

    def visit_CallExpr(self, node: CallExpr):
        """
        Replace any method calls with calls to their exported counterparts.
        """

        if isinstance(node.name, MemberRefExpr):
            mrefexpr = node.name
            containerexpr = mrefexpr.containerexpr

            #direct member ref expression:   class_var.method()
            if hasattr(containerexpr, "type") and isinstance(
                    containerexpr.type, Class):
                replacement_function_name = self.get_method_replacement_name(
                    mrefexpr.name, containerexpr.type.name)
                first_argument = UnOp(lvalue=self.visit(containerexpr),
                                      op="&",
                                      postfix=False)
                return CallExpr(name=replacement_function_name,
                                args=[first_argument] +
                                [self.visit(a) for a in node.args])

            #indirect member ref expression:   class_var->method()
            if hasattr(containerexpr, "type") and isinstance(
                    containerexpr.type, Pointer):
                if isinstance(containerexpr.type.pointee_type, Class):
                    replacement_function_name = self.get_method_replacement_name(
                        mrefexpr.name, containerexpr.type.pointee_type.name)
                    first_argument = self.visit(containerexpr)
                    return CallExpr(name=replacement_function_name,
                                    args=[first_argument] +
                                    [self.visit(a) for a in node.args])

        return self.generic_visit(node)


class PrinterVisitor(NodeVisitor):
    def visit_VarDecl(self, node: VarDecl):
        print(node.name + " = ", end='')
        if hasattr(node, "init"):
            print(node.init)
        else:
            print()

    def visit_BasicBlock(self, node: BasicBlock):
        for child in node.body:
            if not isinstance(child, DeclStmt):
                print(child)
            self.visit(child)
        return node