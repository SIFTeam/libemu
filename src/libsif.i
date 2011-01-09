/* example.i */
%module libsif
%{
#include "lib/ipkg/Ipkg.h"
#include "lib/ipkg/IpkgCategory.h"
#include "lib/ipkg/IpkgPackage.h"
#include "lib/emu/EmuMessages.h"
#include "lib/emu/EmuClient.h"
static double PythonCallBackA(double a, double b, void *clientdata);
static void PythonCallBackB(char *message, void *clientdata);
static void PythonCallBackC(bool ret, void *clientdata);
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

%include "lib/ipkg/Ipkg.h"
%include "lib/ipkg/IpkgCategory.h"
%include "lib/ipkg/IpkgPackage.h"
%include "lib/emu/EmuMessages.h"
%include "lib/emu/EmuClient.h"

%{
/* This function matches the prototype of the normal C callback
   function for our widget. However, we use the clientdata pointer
   for holding a reference to a Python callable object. */

static double PythonCallBackA(double a, double b, void *clientdata)
{
	PyObject *func, *arglist;
	PyObject *result;
	double    dres = 0;

	SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	func = (PyObject *) clientdata;               // Get Python function
	arglist = Py_BuildValue("(d,d)",a, b);        // Build argument list
	result = PyEval_CallObject(func,arglist);     // Call Python
	Py_DECREF(arglist);                           // Trash arglist
	if (result) {                                 // If no errors, return double
		dres = PyFloat_AsDouble(result);
	}
	Py_XDECREF(result);
	SWIG_PYTHON_THREAD_END_BLOCK;
	return dres;
}

static void PythonCallBackB(char *message, void *clientdata)
{
	PyObject *func, *arglist;
	
	SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	func = (PyObject *) clientdata;               // Get Python function
	arglist = Py_BuildValue("(s)",message);       // Build argument list
	PyEval_CallObject(func,arglist);              // Call Python
	Py_DECREF(arglist);                           // Trash arglist
	SWIG_PYTHON_THREAD_END_BLOCK;
}

static void PythonCallBackC(bool ret, void *clientdata)
{
	PyObject *func, *arglist;
	SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	func = (PyObject *) clientdata;               // Get Python function
	arglist = Py_BuildValue("(b)", ret);          // Build argument list
	PyEval_CallObject(func,arglist);              // Call Python
	Py_DECREF(arglist);                           // Trash arglist
	SWIG_PYTHON_THREAD_END_BLOCK;
}
%}

// Attach a new method to our plot widget for adding Python functions
%extend Ipkg
{
	// Set a Python function object as a callback function
	// Note : PyObject *pyfunc is remapped with a typempap
	void setPyProgressCallback(PyObject *pyfunc) {
		self->setProgressCallback(PythonCallBackA, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}
	
	void setPyNoticeCallback(PyObject *pyfunc) {
		self->setNoticeCallback(PythonCallBackB, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}

	void setPyErrorCallback(PyObject *pyfunc) {
		self->setErrorCallback(PythonCallBackB, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}
	
	void setPyEndCallback(PyObject *pyfunc) {
		self->setEndCallback(PythonCallBackC, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}
}

%extend EmuMessages
{
	void setPyErrorCallback(PyObject *pyfunc) {
		self->setErrorCallback(PythonCallBackB, (void *) pyfunc);
		Py_INCREF(pyfunc);
	}
}
