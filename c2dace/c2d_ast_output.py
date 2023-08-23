from c2d_c_ast import *
from c2d_clang2c_ast import type_bool

def get_pseudocode(node: Node) -> str:
	header = '''
// ======================================================
// C2DACE
// AUTOMATICALLY GENERATED PSEUDOCODE FOR DEBUGGING
// NOT INTENDED FOR COMPILATION!
// ======================================================\n'''
	return header + pseudocode_node(node)


def pseudocode_type(type: Type):
	if type.__class__ in pseudocode_type_functions.keys():
		return pseudocode_type_functions[type.__class__](type)
	return pseudocode_type_default(type)

def type_as_tuple(type_pc: str):
	if '[' in type_pc:
		i = type_pc.index('[')
		p1 = type_pc[:i]
		p2 = type_pc[i:]
		return (p1, p2)
	return (type_pc, "")

def pseudocode_type_default(type: Type):
	return type.__class__.__name__.lower()

def pseudocode_type_pointer(type: Pointer):
	return "%s*" % pseudocode_type(type.pointee_type)

def pseudocode_type_struct(type: Struct):
	return pseudocode_type_default(type) + " " + type.name

def pseudocode_type_class(type: Class):
	return pseudocode_type_default(type) + " " + type.name

def pseudocode_type_constantarray(type: ConstantArray):
	return "%s[%s]" % (pseudocode_type(type.element_type), str(type.size))

pseudocode_type_functions = {
	Pointer: pseudocode_type_pointer,
	Struct: pseudocode_type_struct,
	Class: pseudocode_type_class,
	ConstantArray: pseudocode_type_constantarray,
}


# === NODES ============================================================================
def pseudocode_node(node: Node):
	if node is None:
		return ""
	if isinstance(node, list):
		return "".join([pseudocode_node(n) for n in node])
	if node.__class__ in pseudocode_node_functions.keys():
		return pseudocode_node_functions[node.__class__](node)
	return pseudocode_node_default(node)

def pseudocode_node_default(node: Node):
	field_vals = [getattr(node, name, None) for name in node._fields]
	code = ""
	for v in field_vals:
		code += pseudocode_node(v)
	return code

def pseudocode_node_retstmt(node: RetStmt):
	return "return %s" % pseudocode_node_default(node)

def pseudocode_node_forstmt(node: ForStmt):
	return "for (%s; %s; %s) %s" % (pseudocode_node(node.init), pseudocode_node(node.cond), pseudocode_node(node.iter), pseudocode_node(node.body))

def pseudocode_node_whilestmt(node: WhileStmt):
	return "while (%s) %s" % (pseudocode_node(node.cond), pseudocode_node(node.body))

def pseudocode_node_funcdecl(node: FuncDecl):
	args_pseudocode = ", ".join([pseudocode_node(a) for a in node.args])
	if node.body is None:
		return "%s %s (%s);\n" % (pseudocode_type(node.result_type), node.name, args_pseudocode)
	return "%s %s (%s) %s\n\n" % (pseudocode_type(node.result_type), node.name, args_pseudocode, pseudocode_node(node.body))

def pseudocode_node_callexpr(node: CallExpr):
	args_pseudocode = ", ".join([pseudocode_node(a) for a in node.args])
	return "%s(%s)" % (pseudocode_node(node.name), args_pseudocode)

def pseudocode_node_basicblock(node: BasicBlock):
	return "{\n\t%s;\n}" % ";\n\t".join([pseudocode_node(n) for n in node.body])

def pseudocode_node_declstmt(node: DeclStmt):
	return "; ".join(pseudocode_node(vd) for vd in node.vardecl)

def pseudocode_node_vardecl(node: VarDecl):
	type_p1, type_p2 = type_as_tuple(pseudocode_type(node.type))
	code = "%s %s %s" % (type_p1, node.name, type_p2)
	if hasattr(node, "init"):
		code += " = " + pseudocode_node(node.init)
	return code

def pseudocode_node_parmdecl(node: ParmDecl):
	type_p1, type_p2 = type_as_tuple(pseudocode_type(node.type))
	return "%s %s %s" % (type_p1, node.name, type_p2)

def pseudocode_node_intliteral(node: IntLiteral):
	if len(node.value) == 1:
		return str(node.value[0])
	return "IntLiteral(%s)" % str(node.value)

def pseudocode_node_floatliteral(node: FloatLiteral):
	return str(node.value[0])

def pseudocode_node_declrefexpr(node: DeclRefExpr):
	return node.name

def pseudocode_node_structdecl(node: StructDecl):
	return "struct %s {\n\t%s};\n\n" % (node.name, "\t".join([pseudocode_node(f) for f in node.fields]))

def pseudocode_node_fielddecl(node: FieldDecl):
	type_p1, type_p2 = type_as_tuple(pseudocode_type(node.type))
	return "%s %s %s;\n" % (type_p1, type_p2, node.name)

def pseudocode_node_memberrefexpr(node: MemberRefExpr):
	symb = "."
	if hasattr(node.containerexpr, "type") and isinstance(node.containerexpr.type, Pointer):
		symb = "->"
	return "%s%s%s" % (pseudocode_node(node.containerexpr), symb, node.name)

def pseudocode_node_binop(node: BinOp):
	return "%s %s %s" % (pseudocode_node(node.lvalue), node.op, pseudocode_node(node.rvalue))

def pseudocode_node_unop(node: UnOp):
	if node.postfix:
		return pseudocode_node(node.lvalue) + node.op
	return node.op + pseudocode_node(node.lvalue)

def pseudocode_node_ifstmt(node: IfStmt):
	code = "if (%s) %s" % (pseudocode_node(node.cond), pseudocode_node(node.body_if))
	if hasattr(node, "body_else"):
		code += " else %s" % pseudocode_node(node.body_else)
	return code

def pseudocode_node_arraysubscriptexpr(node: ArraySubscriptExpr):
	return "%s[%s]" % (pseudocode_node(node.unprocessed_name), pseudocode_node(node.index))

def pseudocode_node_parenexpr(node: ParenExpr):
	return "(%s)" % pseudocode_node(node.expr)

def pseudocode_node_stringliteral(node: StringLiteral):
	return node.value[0]

def pseudocode_node_charliteral(node: CharLiteral):
	return node.value[0]

def pseudocode_node_initlistexpr(node: InitListExpr):
	return "{%s}" % ", ".join(pseudocode_node(v) for v in node.body)

def pseudocode_node_breakstmt(node: BreakStmt):
	return "break"

def pseudocode_node_compoundassignop(node: CompoundAssignOp):
	return "%s %s %s" % (pseudocode_node(node.lvalue), node.op, pseudocode_node(node.rvalue))

def pseudocode_node_ternaryexpr(node: TernaryExpr):
	return "%s ? %s : %s" % (pseudocode_node(node.cond), pseudocode_node(node.left), pseudocode_node(node.right))

# def pseudocode_node_constructordecl(node: ConstructorDecl):
#     args_pseudocode = ", ".join([pseudocode_node(a) for a in node.args])
#     return "%s::%s (%s) %s\n\n" % (
#         node.parent_class_type, 
#         node.name, 
#         args_pseudocode, 
#         pseudocode_node(node.body)
#     )



pseudocode_node_functions = {
	RetStmt: pseudocode_node_retstmt,
	ForStmt: pseudocode_node_forstmt,
	WhileStmt: pseudocode_node_whilestmt,
	FuncDecl: pseudocode_node_funcdecl,
	CallExpr: pseudocode_node_callexpr,
	BasicBlock: pseudocode_node_basicblock,
	DeclStmt: pseudocode_node_declstmt,
	VarDecl: pseudocode_node_vardecl,
	ParmDecl: pseudocode_node_parmdecl,
	IntLiteral: pseudocode_node_intliteral,
	FloatLiteral: pseudocode_node_floatliteral,
	DeclRefExpr: pseudocode_node_declrefexpr,
	StructDecl: pseudocode_node_structdecl,
	FieldDecl: pseudocode_node_fielddecl,
	MemberRefExpr: pseudocode_node_memberrefexpr,
	BinOp: pseudocode_node_binop,
	UnOp: pseudocode_node_unop,
	IfStmt: pseudocode_node_ifstmt,
	ArraySubscriptExpr: pseudocode_node_arraysubscriptexpr,
	ParenExpr: pseudocode_node_parenexpr,
	StringLiteral: pseudocode_node_stringliteral,
	CharLiteral: pseudocode_node_charliteral,
	InitListExpr: pseudocode_node_initlistexpr,
	BreakStmt: pseudocode_node_breakstmt,
	CompoundAssignOp: pseudocode_node_compoundassignop,
	TernaryExpr: pseudocode_node_ternaryexpr,
	# ConstructorDecl: pseudocode_node_constructordecl
}