typedef void *PyObject;

    int
PyMapping_SetItemString(o, key, value)
 PyObject *o;
 char *key;
 PyObject *value;
{
  PyObject *okey;
  int r;

  if( ! key) return Py_ReturnNullError(),-1;
  if (!(okey=PyString_FromString(key))) return -1;
  r = PyObject_SetItem(o,okey,value);
  if (--( okey )->ob_refcnt != 0) ; else (*(  okey  )->ob_type->tp_dealloc)((PyObject *)(  okey  ))  ;
  return r;
}

