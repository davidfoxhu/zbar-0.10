/*------------------------------------------------------------------------
 *  Copyright 2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include "zbarmodule.h"

static char enumitem_doc[] = PyDoc_STR("simple enumeration item.\n\nassociates an int value with a name for printing.");

static zbarEnumItem* enumitem_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	int val = 0;
	PyObject *name = NULL;
	static char *kwlist[] = { "value", "name", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iS", kwlist, &val, &name))
	{
		return(NULL);
	}

	zbarEnumItem* self = (zbarEnumItem*)type->tp_alloc(type, 0);

	if (!self)
	{
		return(NULL);
	}
	/********************************������********************/
	/*self->val.ob_ival = val;*/
	self->val.ob_digit[0] = val;
	self->name = name;

	return(self);
}

static void enumitem_dealloc(zbarEnumItem *self)
{
	Py_CLEAR(self->name);
	((PyObject*)self)->ob_type->tp_free((PyObject*)self);
}

static PyObject* enumitem_str(zbarEnumItem *self)
{
	Py_INCREF(self->name);

	return(self->name);
}

static int enumitem_print(zbarEnumItem *self, FILE *fp, int flags)
{
	return(self->name->ob_type->tp_print(self->name, fp, flags));
}

static PyObject* enumitem_repr(zbarEnumItem *self)
{
	PyObject *name = PyObject_Repr(self->name);

	if (!name)
	{
		return(NULL);
	}

	/********************************������********************/
	/*char *	namestr = PyString_AsString(name);
	PyObject*	repr = PyString_FromFormat("%s(%ld, %s)", ((PyObject*)self)->ob_type->tp_name, self->val.ob_ival, namestr);*/
	char*		namestr = PyBytes_AsString(name);
	PyObject*	repr = PyBytes_FromFormat("%s(%ld, %s)", ((PyObject*)self)->ob_type->tp_name, self->val.ob_digit[0], namestr);

	Py_DECREF(name);

	return((PyObject*)repr);
}

PyTypeObject zbarEnumItem_Type =
{
	PyObject_HEAD_INIT(NULL)
	.tp_name        = "zbar.EnumItem",
	.tp_doc         = enumitem_doc,
	.tp_basicsize   = sizeof(zbarEnumItem),
	.tp_flags       = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new         = (newfunc)enumitem_new,
	.tp_dealloc     = (destructor)enumitem_dealloc,
	.tp_str         = (reprfunc)enumitem_str,
	.tp_print       = (printfunc)enumitem_print,
	.tp_repr        = (reprfunc)enumitem_repr,
};

zbarEnumItem* zbarEnumItem_New(PyObject* byname, PyObject* byvalue, int val, const char* name)
{
	zbarEnumItem* self = PyObject_New(zbarEnumItem, &zbarEnumItem_Type);

	if (NULL == self)
	{
		return(NULL);
	}
	/********************************������********************/
	/*self->val.ob_ival = val;
	self->name = PyString_FromString(name);
	if(!self->name ||
	   (byname && PyDict_SetItem(byname, self->name, (PyObject*)self)) ||
	   (byvalue && PyDict_SetItem(byvalue, (PyObject*)self, (PyObject*)self)))*/
	self->val.ob_digit[0] = val;
	self->name = PyBytes_FromString(name);
	if( (NULL == self->name) || 
		(NULL != byname  && PyDict_SetItem(byname, self->name, (PyObject*)self)) ||
		(NULL != byvalue && PyDict_SetItem(byvalue, PyLong_FromLong(val), (PyObject*)self))
		)
	{
		Py_DECREF((PyObject*)self);
		return(NULL);
	}

	return(self);
}

static char enum_doc[] = PyDoc_STR("enumeration container for EnumItems.\n\nexposes items as read-only attributes");

/* FIXME add iteration */

static int enum_traverse(zbarEnum *self, visitproc visit, void *arg)
{
	Py_VISIT(self->byname);
	Py_VISIT(self->byvalue);

	return(0);
}

static int enum_clear(zbarEnum *self)
{
	Py_CLEAR(self->byname);
	Py_CLEAR(self->byvalue);

	return(0);
}

static void enum_dealloc(zbarEnum *self)
{
	enum_clear(self);
	((PyObject*)self)->ob_type->tp_free((PyObject*)self);
}

PyTypeObject zbarEnum_Type =
{
	PyObject_HEAD_INIT(NULL)
	.tp_name        = "zbar.Enum",
	.tp_doc         = enum_doc,
	.tp_basicsize   = sizeof(zbarEnum),
	.tp_flags       = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
	.tp_dictoffset  = offsetof(zbarEnum, byname),
	.tp_traverse    = (traverseproc)enum_traverse,
	.tp_clear       = (inquiry)enum_clear,
	.tp_dealloc     = (destructor)enum_dealloc,
};

zbarEnum* zbarEnum_New()
{
	zbarEnum* self = PyObject_GC_New(zbarEnum, &zbarEnum_Type);

	if (!self)
	{
		return(NULL);
	}
	self->byname = PyDict_New();
	self->byvalue = PyDict_New();
	if(NULL == self->byname || NULL == self->byvalue)
	{
		Py_DECREF(self);
		return(NULL);
	}

	return(self);
}

int zbarEnum_Add(zbarEnum* self, int val, const char* name)
{
	zbarEnumItem* item = zbarEnumItem_New(self->byname, self->byvalue, val, name);

	if (NULL == item)
	{
		return(-1);
	}

	return(0);
}
