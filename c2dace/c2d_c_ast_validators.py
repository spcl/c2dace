from c2d_c_ast import Node, Type
import c2d_c_ast_transforms
from typing import List

class ASTIntegrityException(Exception):
    pass

class ASTValidator(c2d_c_ast_transforms.NodeTransformer):
    exceptions: List[ASTIntegrityException]

    def __init__(self) -> None:
        self.exceptions = []
        super().__init__()

    def visit(self, node):
        node = self.validate(node)
        return super().visit(node)

    def did_find_exceptions(self) -> bool:
        return len(self.exceptions) > 0

    def validate(self, node: Node) -> Node:
        pass

    def mark_exception(self, node: Node, message: str) -> Node:
        exception = ASTIntegrityException("[" + type(self).__name__ + "]: " + message)
        self.exceptions.append(exception)
        node.integrity_exceptions.append(exception)
        return node

    def report(self) -> str:
        return "[" + type(self).__name__ + "]: found " + str(len(self.exceptions)) + " integrity exceptions"

    def raise_exceptions(self):
        if self.did_find_exceptions():
            raise ASTIntegrityException(self.report())


class ValidateAllAttributesPresent(ASTValidator):
    # checks that every node actually has the attributes marked in _fields and _attributes.
    def validate(self, node: Node) -> Node:
        for a in node._fields + node._attributes:
            if not hasattr(node, a):
                node = self.mark_exception(node, type(node).__name__ + " missing attribute \"" + a + "\"")
        return node

class ValidateNodeTypes(ASTValidator):
    # checks that any all types are actually instances of c2d_c_ast.Type
    def validate(self, node: Node) -> Node:
        if hasattr(node, "type"):
            if not isinstance(node.type, Type):
                node = self.mark_exception(node, "node type is not an instance of " + Type.__module__.__name__ + "." + Type.__name__)
        return node