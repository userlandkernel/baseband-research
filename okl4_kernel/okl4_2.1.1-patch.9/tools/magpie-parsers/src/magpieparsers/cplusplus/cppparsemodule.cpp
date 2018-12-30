#include <Python.h>

#include <antlr/CharInputBuffer.hpp>

#include "ASTNode.hpp"
#include "CPPLexer.hpp"
#include "CPPParser.hpp"

/* The Python-accessible version of the ASTNodes we use for C++ parsing */
typedef struct {
    PyObject_HEAD
	ASTNode *node;
} pyNode;

static void
pyNode_dealloc(pyNode *self)
{
	/* FIXME: We can't just delete this node, because Python may use it again */
	/* This means we (currently) leak the entire tree */
	/*if(self->node) {
		delete self->node;
		self->node = NULL;
	}*/
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
pyNode_get_name(pyNode *self, void *closure)
{
	return PyString_FromString(self->node->get_name());
}

static PyObject *
pyNode_get_leaf(pyNode *self, void *closure)
{
	return PyString_FromString(self->node->get_leaf());
}

static PyObject *
pyNode_get_value(pyNode *self, void *closure)
{
	return PyString_FromString(self->node->get_value());
}

static PyObject *
pyNode_get_children(pyNode *self, void *closure)
{
	/* Return a list of kids. */
	PyObject *pykids = PyList_New(self->node->num_children());

	for(int i = 0; i < self->node->num_children(); i++) {
		pyNode *newpynode;
		
		newpynode = PyObject_New(pyNode, self->ob_type);
		newpynode->node = self->node->get_child(i);

		PyList_SetItem(pykids, i, (PyObject *)newpynode);
	}

	return pykids;
}

static PyObject *
pyNode_get_source_line(pyNode *self, void *closure)
{
	/* Return the source line for this node */
	/* FIXME: Stub */
	return PyInt_FromLong(0);
}

static PyObject *
pyNode_get_source_file(pyNode *self, void *closure)
{
	/* FIXME: Stub */
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyNode_get_nonprinting(pyNode *self, void *closure)
{
	/* FIXME: Stub */
	Py_RETURN_FALSE;
}

static PyGetSetDef pyNode_getseters[] = {
    {"name", (getter)pyNode_get_name, NULL, "node name", NULL},
    {"leaf", (getter)pyNode_get_leaf, NULL, "node leaf", NULL},
    {"value", (getter)pyNode_get_value, NULL, "node value", NULL},
	{"children", (getter)pyNode_get_children, NULL, "node children", NULL},
	{"source_line", (getter)pyNode_get_source_line, NULL, "source line", NULL},
	{"source_file", (getter)pyNode_get_source_file, NULL, "source file", NULL},
	{"nonprinting", (getter)pyNode_get_nonprinting, NULL, "should this node display in print_tree?", NULL},
    {NULL}  /* Sentinel */
};

/* Callback to build the list of children */
struct callback_struct
{
	pyNode *self;
	PyObject *obj;
};

void
_pyNode_get_children_named_callback(ASTNode *node, void *v_callback)
{
	struct callback_struct *callback = (struct callback_struct *)v_callback;
	pyNode *newpynode;

	newpynode = PyObject_New(pyNode, callback->self->ob_type);
	newpynode->node = node;

	PyList_Append(callback->obj, (PyObject *)newpynode);
}

/* Return the list of children matching "name" */
static PyObject *
pyNode_get_children_named(pyNode *self, PyObject *args)
{
	char *child_name;
	struct callback_struct callback;

	if(!PyArg_ParseTuple(args, "s", &child_name))
		return NULL;

	callback.obj = PyList_New(0); /* Don't know size yet */
	callback.self = self;

	self->node->call_children(child_name, _pyNode_get_children_named_callback, (void *)(&callback));

	return callback.obj;
}

static PyObject *
pyNode_has_child(pyNode *self, PyObject *args)
{
	char *child_name;

	if(!PyArg_ParseTuple(args, "s", &child_name))
		return NULL;

	if(self->node->has_child(child_name)) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

static PyObject *
pyNode_print_tree(pyNode *self)
{
	self->node->print_tree();
	Py_RETURN_NONE;
}

static PyObject *
pyNode_get_child(pyNode *self, PyObject *args)
{
	char *child_name;
	ASTNode *child;

	if(!PyArg_ParseTuple(args, "s", &child_name))
		return NULL;

	child = self->node->get_child(child_name);
	if(child) {
		pyNode *newpynode;

		newpynode = PyObject_New(pyNode, self->ob_type);
		newpynode->node = child;
		return (PyObject *)newpynode;
	} else {
		Py_RETURN_NONE;
	}
}

static PyObject *
pyNode_get_only_child(pyNode *self)
{
	ASTNode *child;

	if (self->node->num_children() == 1) {
		child = self->node->get_child(0);
		pyNode *newpynode;

		newpynode = PyObject_New(pyNode, self->ob_type);
		newpynode->node = child;
		return (PyObject *)newpynode;
	} else {
		Py_RETURN_NONE;
	}
}

static PyMethodDef pyNode_methods[] = {
    {"get_children_named", (PyCFunction)pyNode_get_children_named, METH_VARARGS, "Get all children with a given name" },
	{"get_child", (PyCFunction)pyNode_get_child, METH_VARARGS, "Get the single child with the given name or return None"},
	{"has_child", (PyCFunction)pyNode_has_child, METH_VARARGS, "Return True if at least one child has the given name" },
	{"print_tree", (PyCFunction)pyNode_print_tree, METH_NOARGS, "Print the tree from here"},
	{"child", (PyCFunction)pyNode_get_only_child, METH_NOARGS, "Return the one and only child"},
    {NULL}  /* Sentinel */
};

static PyTypeObject pyNodeType= {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "cppparse.Node",           /*tp_name*/
    sizeof(pyNode),            /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)pyNode_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "CPPParser Node object",   /* tp_doc */
	0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    pyNode_methods,            /* tp_methods */
	0,                         /* tp_members */
	pyNode_getseters           /* tp_getset */
};



ASTNode *
do_native_parse(unsigned char *buffer, size_t length)
{
	ASTNode *result;

	antlr::CharInputBuffer input(buffer, length, false);

	CPPLexer lexer(input);
	lexer.setFilename("unknown");

	CPPParser parser(lexer);
	parser.setFilename("unknown");
	parser.init();
	result = parser.translation_unit();
	parser.uninit();

	return result;
}

PyObject *
do_parse(unsigned char *buffer, int buffer_length)
{
	ASTNode *node;
	pyNode *newpynode;

	node = do_native_parse(buffer, buffer_length);
	newpynode = PyObject_New(pyNode, &pyNodeType);
	newpynode->node = node;

	return (PyObject *)newpynode;
}

/* Line directive handler for parser (pretty cludgy...) */
void
process_line_directive(const char *includedFile, const char *includedLineNo)
{
}

extern "C" {

static PyObject *
cppparse_get_pt(PyObject *self, PyObject *args)
{
	const char *input;
	int input_length;
	PyObject *PTOut;

	if(!PyArg_ParseTuple(args, "s#", &input, &input_length))
		return NULL;

	PTOut = do_parse((unsigned char *)input, input_length);

	return Py_BuildValue("O", PTOut);
}
} // extern "C"

static PyMethodDef CPPParseMethods[] = {
    {"get_pt", cppparse_get_pt, METH_VARARGS, "Parse the supplied string to a parse tree"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initcppparse(void)
{
    PyObject* m;

    pyNodeType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyNodeType) < 0)
        return;

    m = Py_InitModule3("cppparse", CPPParseMethods, "C++ parser");

    Py_INCREF(&pyNodeType);
    PyModule_AddObject(m, "Node", (PyObject *)&pyNodeType);
}
