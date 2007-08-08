// -*- C++ -*-

// PythonExtension.h
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

#include <plearn/python/PythonExtension.h>
#include <plearn/python/PythonObjectWrapper.h>
#include <plearn/base/RemoteDeclareMethod.h>
#include <plearn/base/HelpSystem.h>
#include <plearn/base/TypeFactory.h>
#include <plearn/base/PMemPool.h>
#include <plearn/base/stringutils.h>

namespace PLearn {

// Trampoline for global PLearn 'remote' functions
PyObject* pythonGlobalFuncTramp(PyObject* self, PyObject* args)
{
    RemoteTrampoline* t= 
        static_cast<RemoteTrampoline*>(PyCObject_AsVoidPtr(self));
    int nas= PyTuple_GET_SIZE(args);
    TVec<PythonObjectWrapper> as;
    for(int i= 0; i < nas; ++i)
        as.push_back(PythonObjectWrapper(PyTuple_GET_ITEM(args, i)));
    try
    {
        PythonObjectWrapper returned_value= t->call(0, as);
        PyObject* to_return= returned_value.getPyObject();
        Py_XINCREF(to_return);
        return to_return;
    }
    catch(const PLearnError& e) 
    {
        PyErr_SetString(PyExc_Exception, e.message().c_str());
        return 0;
    }
    catch(const std::exception& e) 
    {
        PyErr_SetString(PyExc_Exception, e.what());
        return 0;
    }
    catch(...) 
    {
        PyErr_SetString(PyExc_Exception,
                        "Caught unknown C++ exception");
        return 0;
    }
}

// the global funcs (storage never reclaimed)
static PObjectPool<PyMethodDef> pyfuncs(50);
static TVec<string> funcs_help;

void injectPLearnGlobalFunctions(PyObject* env)
{
    const RemoteMethodMap::MethodMap& global_funcs= 
        getGlobalFunctionMap().getMap();

    for(RemoteMethodMap::MethodMap::const_iterator it=
            global_funcs.begin();
        it != global_funcs.end(); ++it)
    {
        PyObject* self= 
            PyCObject_FromVoidPtr(it->second, NULL);
    
        PyMethodDef* py_method= pyfuncs.allocate();
        py_method->ml_name= 
            const_cast<char*>(it->first.first.c_str());
        py_method->ml_meth= pythonGlobalFuncTramp;
        py_method->ml_flags= METH_VARARGS;
        funcs_help.push_back(
            HelpSystem::helpOnFunction(it->first.first.c_str(), 
                                       it->first.second));
        py_method->ml_doc= const_cast<char*>(funcs_help.last().c_str());
    
        /* module= env if env is a module; NULL otherwise */
        PyObject* module= 0;
        if(PyModule_Check(env))
            module= env;

        // N.B.: module == NULL works on python2.3, 2.4 and 2.5, but is not
        // documented
        PyObject* pyfunc= 
            PyCFunction_NewEx(py_method, self, module);
	    
        if(pyfunc) 
            PyObject_SetAttrString(env, 
                                   py_method->ml_name, 
                                   pyfunc);
        else
            PLERROR("Cannot inject global function "
                    "'%s' into python.",
                    py_method->ml_name);
    }
}


void injectPLearnClasses(PyObject* module)
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    PythonObjectWrapper::initializePython();

    if(!PyModule_Check(module))
        PLERROR("In injectPLearnClasses : "
                "module param. should be a python module.");
    // import python class for wrapping PLearn objects
    string importcode= "\nfrom plearn.pybridge.wrapped_plearn_object "
        "import *\n";
    PyObject_SetAttrString(module, "__builtins__", PyEval_GetBuiltins());
    PyObject* res= PyRun_String(importcode.c_str(), Py_file_input, 
                                PyModule_GetDict(module), PyModule_GetDict(module));
    if(!res)
    {
        Py_DECREF(module);
        if(PyErr_Occurred()) PyErr_Print();
        PLERROR("in injectPLearnClasses : cannot import plearn.pybridge.wrapped_plearn_object.");
    }
    Py_DECREF(res);

    string wrapper_name= "WrappedPLearnObject";
    // now find the class in the env.
    typedef map<string, PyObject*> env_t;
    env_t env= PythonObjectWrapper(
        PyModule_GetDict(module), 
        PythonObjectWrapper::transfer_ownership).as<env_t>();
    env_t::iterator clit= env.find(wrapper_name);
    if(clit == env.end())
        PLERROR("in injectPLearnClasses : "
                "class %s not defined "
                "in plearn.pybridge.wrapped_plearn_object",
                wrapper_name.c_str());
    PyObject* wrapper= clit->second;

    //inject unref and newCPPObj methods
    PyMethodDef* py_method= &PythonObjectWrapper::m_unref_method_def;
    py_method->ml_name  = "_unref";
    py_method->ml_meth  = PythonObjectWrapper::python_del;
    py_method->ml_flags = METH_VARARGS;
    py_method->ml_doc   = "Injected unref function from PythonObjectWrapper; "
        "DO NOT USE THIS FUNCTION! IT MAY DEALLOCATE THE PLEARN OBJECT!";
    PyObject* py_funcobj= PyCFunction_NewEx(py_method, NULL, NULL);
    PyObject* py_methobj= PyMethod_New(py_funcobj, NULL, wrapper);
    Py_XDECREF(py_funcobj);
    if(!py_funcobj || !py_methobj) 
    {
        Py_DECREF(module);
        Py_XDECREF(py_methobj);
        PLERROR("in injectPLearnClasses : "
                "can't inject method '%s' (i.e. __del__)", 
                py_method->ml_name);
    }
    PyObject_SetAttrString(wrapper, py_method->ml_name, py_methobj);
    Py_DECREF(py_methobj);

    // inject 'newCPPObj' and 'refCPPObj' class methods
    TVec<PyMethodDef*> classmethods(2);
    classmethods[0]= &PythonObjectWrapper::m_newCPPObj_method_def;
    classmethods[0]->ml_name  = "_newCPPObj";
    classmethods[0]->ml_meth  = PythonObjectWrapper::newCPPObj;
    classmethods[0]->ml_flags = METH_VARARGS;
    classmethods[0]->ml_doc   = "Injected new function from PythonObjectWrapper; "
        "DO NOT USE THIS FUNCTION!";
    classmethods[1]= &PythonObjectWrapper::m_refCPPObj_method_def;
    classmethods[1]->ml_name  = "_refCPPObj";
    classmethods[1]->ml_meth  = PythonObjectWrapper::refCPPObj;
    classmethods[1]->ml_flags = METH_VARARGS;
    classmethods[1]->ml_doc   = "Injected new function from PythonObjectWrapper; "
        "DO NOT USE THIS FUNCTION!";

    for(TVec<PyMethodDef*>::iterator mit= classmethods.begin();
        mit != classmethods.end(); ++mit)
    {
        py_method= *mit;
        py_funcobj= PyCFunction_NewEx(py_method, NULL, NULL);
        py_methobj= PyMethod_New(py_funcobj, NULL, wrapper);
        Py_XDECREF(py_funcobj);
        if(!py_funcobj || !py_methobj) 
        {
            Py_DECREF(module);
            Py_XDECREF(py_methobj);
            PLERROR("in injectPLearnClasses : "
                    "can't inject method '%s' (i.e. C++'s new)", 
                    py_method->ml_name);
        }
        PyObject_SetAttrString(wrapper, py_method->ml_name, py_methobj);
        Py_DECREF(py_methobj);
        
        string classmethodname= wrapper_name+"."+py_method->ml_name;
        res= PyRun_String((classmethodname
                           +"= classmethod("+classmethodname+".im_func)").c_str(), 
                          Py_file_input, 
                          PyModule_GetDict(module), PyModule_GetDict(module));
        Py_DECREF(res);
        if (PyErr_Occurred()) 
        {
            Py_DECREF(module);
            PyErr_Print();
            PLERROR("in injectPLearnClasses : error making "
                    "newCPPObj a classmethod.");
        }
    }

    if(0 != PyType_Ready(reinterpret_cast<PyTypeObject*>(wrapper)))
        PLERROR("in injectPLearnClasses : "
                "failed PyType_Ready on wrapper class.");

    if(!PyCallable_Check(wrapper))
        PLERROR("in injectPLearnClasses : "
                "%s is not callable [not a class?]",
                wrapper_name.c_str());

    PyObject* the_pyclass= 0;
    const TypeMap& tp_map= TypeFactory::instance().getTypeMap();
    for(TypeMap::const_iterator tit= tp_map.begin();
        tit != tp_map.end(); ++tit)
    {
        if(!tit->second.constructor)
            continue; //skip abstract classes

        // create new python type deriving from WrappedPLearnObject
        string classname= tit->first;
        string pyclassname= classname;
        search_replace(pyclassname, " ", "_");
        search_replace(pyclassname, "<", "_");
        search_replace(pyclassname, ">", "_");
        string derivcode= string("\nclass ")
            + pyclassname + "(" + wrapper_name + "):\n"
            "  \"\"\" ... \"\"\"\n"
            "  def __new__(cls,*args,**kwargs):\n"
            "    #print '** "+pyclassname+".__new__',kwargs\n"
            "    obj= object.__new__(cls,*args,**kwargs)\n"
            "    if '_cptr' not in kwargs:\n"
            "      obj._cptr= cls._newCPPObj('"+classname+"')\n"
            "      cls._refCPPObj(obj)\n"
            "    return obj\n";
        PyObject* res= PyRun_String(derivcode.c_str(), 
                                    Py_file_input, 
                                    PyModule_GetDict(module), PyModule_GetDict(module));
        Py_XDECREF(res);
        env= PythonObjectWrapper(
            PyModule_GetDict(module), 
            PythonObjectWrapper::transfer_ownership).as<env_t>();
        clit= env.find(pyclassname);
        if(clit == env.end())
            PLERROR("in injectPLearnClasses : "
                    "Cannot create new python class deriving from "
                    "%s (%s).", 
                    wrapper_name.c_str(),
                    classname.c_str());

        //set option names
        OptionList& options= tit->second.getoptionlist_method();
        unsigned int nopts= options.size();
        TVec<string> optionnames(nopts);
        for(unsigned int i= 0; i < nopts; ++i)
            optionnames[i]= options[i]->optionname();

        the_pyclass= clit->second;
        if(-1==PyObject_SetAttrString(the_pyclass, "_optionnames", 
                                      PythonObjectWrapper(optionnames).getPyObject()))
        {
            Py_DECREF(module);
            if (PyErr_Occurred()) PyErr_Print();
            PLERROR("in injectPLearnClasses : "
                    "cannot set attr _optionnames for class %s",
                    classname.c_str());
        }

        // inject all declared methods
        const RemoteMethodMap* methods= &tit->second.get_remote_methods();

        PP<PObjectPool<PyMethodDef> > meth_def_pool= 
            new PObjectPool<PyMethodDef>(methods->size()+1);

        PythonObjectWrapper::m_pypl_classes.insert(
            make_pair(classname, PLPyClass(the_pyclass, meth_def_pool)));
        TVec<string>& methods_help= 
            PythonObjectWrapper::m_pypl_classes.find(classname)->second.methods_help;

        while(methods)
        {
            for(RemoteMethodMap::MethodMap::const_iterator it= methods->begin();
                it != methods->end(); ++it)
            {
                //get the RemoteTrampoline
                PyObject* tramp= PyCObject_FromVoidPtr(it->second, NULL);
            
                // Create a Python Function Object
                PyMethodDef* py_method= meth_def_pool->allocate();
                py_method->ml_name  = const_cast<char*>(it->first.first.c_str());
                py_method->ml_meth  = PythonObjectWrapper::trampoline;
                py_method->ml_flags = METH_VARARGS;
                methods_help.push_back(HelpSystem::helpOnMethod(classname,
                                                                it->first.first.c_str(),
                                                                it->first.second));
                py_method->ml_doc   = const_cast<char*>(methods_help.last().c_str());
    
                PyObject* py_funcobj= PyCFunction_NewEx(py_method, tramp, module);

                // create an unbound method from the function
                PyObject* py_methobj= PyMethod_New(py_funcobj, NULL, the_pyclass);

                Py_DECREF(tramp);
                Py_XDECREF(py_funcobj);
                if(!py_funcobj || !py_methobj) 
                {
                    Py_DECREF(module);
                    Py_XDECREF(py_methobj);
                    PLERROR("in injectPLearnClasses : "
                            "can't inject method '%s'", py_method->ml_name);
                }

                if(-1==PyObject_SetAttrString(the_pyclass, py_method->ml_name, py_methobj))
                {
                    Py_DECREF(py_methobj);
                    if (PyErr_Occurred()) PyErr_Print();
                    PLERROR("in injectPLearnClasses : "
                            "cannot set attr %s for class %s",
                            py_method->ml_name,
                            classname.c_str());
                }
                Py_DECREF(py_methobj);
            }
            methods= methods->inheritedMethods();//get parent class methods
        }
        if(0 != PyType_Ready(reinterpret_cast<PyTypeObject*>(the_pyclass)))
            PLERROR("in injectPLearnClasses : "
                    "failed PyType_Ready on class %s.",classname.c_str());
        if(module)
            if(-1==PyObject_SetAttrString(module, 
                                          const_cast<char*>(pyclassname.c_str()), 
                                          the_pyclass))
            {
                if (PyErr_Occurred()) PyErr_Print();
                PLERROR("in injectPLearnClasses : "
                        "cannot inject class %s.",
                        classname.c_str());
            }
    }
}

// Init func for python module.
// init module, then inject global funcs
void initPythonExtensionModule(char* module_name)
{
    PythonObjectWrapper::initializePython();
    PyObject* plext= Py_InitModule(module_name, NULL);
    setPythonModuleAndInject(plext);
}

void setPythonModuleAndInject(PyObject* module)
{
    injectPLearnGlobalFunctions(module);
    injectPLearnClasses(module);
    the_PLearn_python_module= module;
}

PyObject* the_PLearn_python_module= 0;

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
