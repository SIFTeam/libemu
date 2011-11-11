/* example.i */
%module libemu
%{
#include "lib/emu/EmuMessages.h"
#include "lib/emu/EmuClient.h"
static void PythonCallBack(char *message, void *clientdata);
%}

// Grab a Python function object as a Python object.
%typemap(in) PyObject *pyfunc
{
	if (!PyCallable_Check($input))
	{
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
		return NULL;
	}
	$1 = $input;
}

%include "lib/emu/EmuMessages.h"
%include "lib/emu/EmuClient.h"

%{
/* This function matches the prototype of the normal C callback
   function for our widget. However, we use the clientdata pointer
   for holding a reference to a Python callable object. */

static void PythonCallBack(char *message, void *clientdata)
{
	PyObject *func, *arglist;
	
	SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	func = (PyObject *) clientdata;               // Get Python function
	arglist = Py_BuildValue("(s)",message);       // Build argument list
	PyEval_CallObject(func,arglist);              // Call Python
	Py_DECREF(arglist);                           // Trash arglist
	SWIG_PYTHON_THREAD_END_BLOCK;
}
%}

// Attach a new method to our plot widget for adding Python functions
%extend EmuMessages
{
	void setPyErrorCallback(PyObject *pyfunc) {
		self->setErrorCallback(PythonCallBack, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}
}
