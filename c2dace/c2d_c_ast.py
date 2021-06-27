import copy
from typing import List, Tuple


class Formatting:
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

    @staticmethod
    def format_string(s, fmt):
        if isinstance(fmt, list):
            return "".join(fmt) + s + (Formatting.END * len(fmt))
        return fmt + s + Formatting.END

    @staticmethod
    def format_bool(b):
        if b:
            return Formatting.format_string(str(b), Formatting.GREEN)
        return Formatting.format_string(str(b), Formatting.RED)


# === TYPES ============================================================================
class Type(object):
    _attributes = ()
    """
    This class represents a C type in the AST
    """
    def __init__(self, *args, **kwargs):  # real signature unknown
        for k, v in kwargs.items():
            setattr(self, k, v)
        super().__init__()

    def __repr__(self) -> str:
        s = [
            attribute + "=" + str(getattr(self, attribute))
            for attribute in self._attributes
        ]
        return Formatting.format_string(
            type(self).__name__, Formatting.YELLOW) + "(" + ", ".join(s) + ")"

    def __eq__(self, o: object) -> bool:
        if type(self) is type(o):
            self_attr_vals = list(
                map(lambda name: getattr(self, name, None), self._attributes))
            o_attr_vals = list(
                map(lambda name: getattr(o, name, None), o._attributes))
            return self_attr_vals == o_attr_vals
        return False

    def get_chain(self):
        return [self]

    def get_chain_end(self):
        return self.get_chain()[-1]

    def get_pointer_chain_end(self) -> Tuple:
        return (0, self)

    def is_struct_like(self) -> bool:
        return isinstance(self.get_chain_end(), Struct)

    def inject_type(self, type):
        return type


class Unknown(Type):
    _attributes = ()


class Pointer(Type):
    _attributes = ("pointee_type", )

    def get_chain(self):
        return [self] + self.pointee_type.get_chain()

    def get_pointer_chain_end(self) -> Tuple[int, Type]:
        i, t = self.pointee_type.get_pointer_chain_end()
        return (i + 1, t)

    def inject_type(self, type):
        new = copy.deepcopy(self)
        new.pointee_type = self.pointee_type.inject_type(type)
        return new


class FunctionProto(Type):
    _attributes = ()


class Int(Type):
    _attributes = ()


class Long(Type):
    _attributes = ()


class LongLong(Type):
    _attributes = ()


class ULongLong(Type):
    _attributes = ()


class Special(Type):
    _attributes = ()


class Double(Type):
    _attributes = ()


class Float(Type):
    _attributes = ()


class Char(Type):
    _attributes = ()


class Bool(Type):
    _attributes = ()


class Void(Type):
    _attributes = ()


class Struct(Type):
    _attributes = ("name", )


class Class(Type):
    _attributes = ("name", )


class Array(Type):
    pass


class ConstantArray(Array):
    _attributes = (
        "element_type",
        "size",
    )

    def get_chain(self):
        return [self] + self.element_type.get_chain()

    def inject_type(self, type):
        new = copy.deepcopy(self)
        new.element_type = self.element_type.inject_type(type)
        return new


class IncompleteArray(Array):
    _attributes = (
        "element_type",
        "size",
    )


class VariableArray(Array):
    _attributes = (
        "element_type",
        "size",
    )


# === NODES ============================================================================
class Node(object):
    def __init__(self, *args, **kwargs):  # real signature unknown
        self.integrity_exceptions = []
        for k, v in kwargs.items():
            setattr(self, k, v)

    _attributes = ("lineno", )
    _fields = ()
    integrity_exceptions: List

    def __eq__(self, o: object) -> bool:
        if type(self) is type(o):
            # check that all fields and attributes match
            self_field_vals = list(
                map(lambda name: getattr(self, name, None), self._fields))
            self_attr_vals = list(
                map(lambda name: getattr(self, name, None), self._attributes))
            o_field_vals = list(
                map(lambda name: getattr(o, name, None), o._fields))
            o_attr_vals = list(
                map(lambda name: getattr(o, name, None), o._attributes))

            return self_field_vals == o_field_vals and self_attr_vals == o_attr_vals
        return False


class AST(Node):
    _attributes = ()
    _fields = (
        "typedefs",
        "funcdefs",
        "structdefs",
        "classdefs",
        "cxxmethods",
        "vardefs",
        "decldefs",
    )


class Statement(Node):
    _attributes = ('col_offset', )
    _fields = ()


class Expression(Statement):
    _attributes = ('type', )
    _fields = ()


class RetStmt(Statement):
    _attributes = ()
    _fields = ('ret_expr', )


class ForStmt(Statement):
    _attributes = ()
    _fields = (
        'init',
        'cond',
        'body',
        'iter',
    )


class WhileStmt(Statement):
    _attributes = ()
    _fields = (
        'cond',
        'body',
    )


class DoStmt(Statement):
    _attributes = ()
    _fields = (
        'cond',
        'body',
    )


class IfStmt(Statement):
    _attributes = ()
    _fields = (
        'cond',
        'body_if',
        'body_else',
    )


class TypeRef(Node):
    _attributes = (
        'name',
        'type',
    )
    _fields = ()


class TypeDecl(Statement):
    _attributes = (
        'name',
        'type',
    )
    _fields = ('typeref', )


class FuncDecl(Statement):
    _attributes = (
        'name',
        'type',
        'keywords',
        'result_type',
    )
    _fields = (
        'args',
        'body',
    )


class BasicBlock(Statement):
    _attributes = ()
    _fields = ('body', )


class ParmDecl(Statement):
    _attributes = (
        'name',
        'type',
    )
    _fields = (
        'sizes',
        'typeref',
    )


class VarDecl(Statement):
    _attributes = (
        'name',
        'type',
    )
    _fields = (
        'sizes',
        'typeref',
        'init',
    )


class Literal(Node):
    pass


class IntLiteral(Literal):
    _attributes = ('value', )
    _fields = ()


class BoolLiteral(Literal):
    _attributes = ('value', )
    _fields = ()


class FloatLiteral(Literal):
    _attributes = ('value', )
    _fields = ()


class StringLiteral(Literal):
    _attributes = ('value', )
    _fields = ()


class CharLiteral(Literal):
    _attributes = ('value', )
    _fields = ()


class DeclRefExpr(Expression):
    _attributes = (
        'name',
        'type',
    )
    _fields = ()


class StructDecl(Node):
    _attributes = ('name', )
    _fields = ('fields', )


class CXXMethod(Node):
    _attributes = (
        'name',
        'parent_class_type',
    )
    _fields = (
        'args',
        'body',
    )


class InitListExpr(Expression):
    _attributes = ('type', )
    _fields = ('body', )


class CXXThisExpr(Expression):
    _attributes = ('type', )
    _fields = ()


class CXXAccessSpecDecl(Node):
    _attributes = ()
    _fields = ()


class ClassDecl(Node):
    _attributes = ('name', )
    _fields = (
        'fields',
        'methods',
    )


class FieldDecl(Node):
    _attributes = (
        'name',
        'type',
    )
    _fields = ()


class MemberRefExpr(Expression):
    _attributes = (
        'name',
        'type',
    )
    _fields = ('containerexpr', )


class CxxCastExpr(Expression):
    _attributes = ('type', )
    _fields = ('expr', )


class CXXUnaryExpr(Expression):
    _attributes = (
        'size',
        'tokens',
    )
    _fields = ()


class CCastExpr(Node):
    _attributes = ('type', )
    _fields = ('expr', )


class ParenExpr(Expression):
    _attributes = ('type', )
    _fields = ('expr', )


class CallExpr(Node):
    _attributes = ('type', )
    _fields = (
        'name',
        'args',
    )


class ArraySubscriptExpr(Expression):
    _attributes = (
        'name',
        'indices',
        'type',
    )
    _fields = (
        'unprocessed_name',
        'index',
    )


class DeclStmt(Statement):
    _attributes = ()
    _fields = ('vardecl', )


class BreakStmt(Statement):
    _attributes = ()
    _fields = ()


class ContinueStmt(Statement):
    _attributes = ()
    _fields = ()


class BinOp(Expression):
    _attributes = (
        'op',
        'type',
    )
    _fields = (
        'lvalue',
        'rvalue',
    )


class CompoundAssignOp(Expression):
    _attributes = (
        'op',
        'type',
    )
    _fields = (
        'lvalue',
        'rvalue',
    )


class UnOp(Expression):
    _attributes = (
        'op',
        'postfix',
        'type',
    )
    _fields = ('lvalue', )


class TernaryExpr(Expression):
    _attributes = ('type')
    _fields = (
        'cond',
        'left',
        'right',
    )


def iter_fields(node):
    """
    Yield a tuple of ``(fieldname, value)`` for each field in ``node._fields``
    that is present on *node*.
    """
    if not hasattr(node, "_fields"):
        a = 1
    for field in node._fields:
        try:
            yield field, getattr(node, field)
        except AttributeError:
            pass


def iter_child_nodes(node):
    """
    Yield all direct child nodes of *node*, that is, all fields that are nodes
    and all items of fields that are lists of nodes.
    """
    #print("CLASS: ",node.__class__)
    #if isinstance(node,DeclRefExpr):
    #print("NAME: ", node.name)

    for name, field in iter_fields(node):
        #print("NASME:",name)
        if isinstance(field, Node):
            yield field
        elif isinstance(field, list):
            for item in field:
                if isinstance(item, Node):
                    yield item


def walk(node):
    """
    Recursively yield all descendant nodes in the tree starting at *node*
    (including *node* itself), in no specified order.  This is useful if you
    only want to modify nodes in place and don't care about the context.
    """
    from collections import deque
    todo = deque([node])
    while todo:
        node = todo.popleft()
        #print(node.__class__.__name__)
        todo.extend(iter_child_nodes(node))
        yield node


class NodeVisitor(object):
    def visit(self, node):
        # print(node.__class__.__name__)
        method = 'visit_' + node.__class__.__name__
        visitor = getattr(self, method, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        """Called if no explicit visitor function exists for a node."""
        for field, value in iter_fields(node):
            if isinstance(value, list):
                for item in value:
                    if isinstance(item, Node):
                        self.visit(item)
            elif isinstance(value, Node):
                self.visit(value)


class NodeTransformer(NodeVisitor):
    """
    A :class:`NodeVisitor` subclass that walks the abstract syntax tree and
    allows modification of nodes.

    The `NodeTransformer` will walk the AST and use the return value of the
    visitor methods to replace or remove the old node.  If the return value of
    the visitor method is ``None``, the node will be removed from its location,
    otherwise it is replaced with the return value.  The return value may be the
    original node in which case no replacement takes place.

    Here is an example transformer that rewrites all occurrences of name lookups
    (``foo``) to ``data['foo']``::

       class RewriteName(NodeTransformer):

           def visit_Name(self, node):
               return copy_location(Subscript(
                   value=Name(id='data', ctx=Load()),
                   slice=Index(value=Str(s=node.id)),
                   ctx=node.ctx
               ), node)

    Keep in mind that if the node you're operating on has child nodes you must
    either transform the child nodes yourself or call the :meth:`generic_visit`
    method for the node first.

    For nodes that were part of a collection of statements (that applies to all
    statement nodes), the visitor may also return a list of nodes rather than
    just a single node.

    Usually you use the transformer like this::

       node = YourTransformer().visit(node)
    """
    def as_list(self, x):
        if isinstance(x, list):
            return x
        if x is None:
            return []
        return [x]

    def generic_visit(self, node):
        for field, old_value in iter_fields(node):
            if isinstance(old_value, list):
                new_values = []
                for value in old_value:
                    if isinstance(value, Node):
                        value = self.visit(value)
                        if value is None:
                            continue
                        elif not isinstance(value, Node):
                            new_values.extend(value)
                            continue
                    new_values.append(value)
                old_value[:] = new_values
            elif isinstance(old_value, Node):
                new_node = self.visit(old_value)
                if new_node is None:
                    delattr(node, field)
                else:
                    setattr(node, field, new_node)
        return node
