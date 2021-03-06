// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
//

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

// Authors: Pascal Vincent & Yoshua Bengio

#define PL_LOG_MODULE_NAME "plerror"

#include <cstdarg>
//#include <cstdlib>
#include <iostream>

#include "plerror.h"
#include <plearn/io/pl_log.h>
#include <plearn/io/PPath.h>

#if USING_MPI
#include <plearn/sys/PLMPI.h>

#endif //USING_MPI


//extern ofstream debug_stream;

namespace PLearn {
using namespace std;

ostream* error_stream = &cerr;

#define ERROR_MSG_SIZE 4096
#ifndef USER_SUPPLIED_ERROR
void errormsg2(const char* filename,const int linenumber,const char* msg, ...){
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];
    
    snprintf(message, ERROR_MSG_SIZE, "In file: \"%s\" at line %d\n",
            PPath(filename).basename().c_str(), linenumber);
    PLASSERT(ERROR_MSG_SIZE>=strlen(message)+strlen(msg));
    strncat(message,msg,ERROR_MSG_SIZE);
    verrormsg(message, args);

    va_end(args);

}

void errormsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    verrormsg(msg, args);
    va_end(args);
}

void verrormsg(const char* msg, va_list args)
{
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message, ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif
    //output to module log: can be useful when throwing exceptions
    DBG_MODULE_LOG << endl << "-------------- PLERROR:" << message << endl << "--------------" << endl;
#ifndef USE_EXCEPTIONS
#if USING_MPI
    *error_stream <<" ERROR from rank=" << PLMPI::rank << ": " <<message<<endl;
#else //USING_MPI
    *error_stream <<" ERROR: "<<message<<endl;
#endif //USING_MPI
    exit(1);
#else
// Commented out as one error message seems to be enough.
//  IMP_LOG << "Throwing PLearnError exception: " << message << endl;
    throw PLearnError(message);                
#endif
}
#endif


void warningmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    vwarningmsg(msg, args);
    va_end(args);
}

void vwarningmsg(const char* msg, va_list args)
{
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    // *error_stream <<" WARNING: "<<message<<endl;
    NORMAL_LOG << " WARNING: " << message << endl;
}

void warn_err(bool warn, const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    if(warn) vwarningmsg(msg,args);
    else verrormsg(msg, args);
    va_end(args);
}

void warn_err2(const char* filename, const int linenumber, bool warn, const char* msg,...)
{
    va_list args;
    va_start(args,msg);

    if(warn) vwarningmsg(msg,args);
    else{
        char message[ERROR_MSG_SIZE];
    
        snprintf(message, ERROR_MSG_SIZE, "In file: \"%s\" at line %d\n",
                 PPath(filename).basename().c_str(), linenumber);
        PLASSERT(ERROR_MSG_SIZE>=strlen(message)+strlen(msg));
        strncat(message,msg,ERROR_MSG_SIZE);
        verrormsg(message, args);
    }
    va_end(args);
}

void  deprecationmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

    // *error_stream <<" DEPRECATION_WARNING: "<<message<<endl;
    NORMAL_LOG << " DEPRECATION_WARNING: " << message << endl;
}

void exitmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

    *error_stream << message << endl;
    exit(1);
}

//! Return a typical error message.
string get_error_message(const char* type, const char* expr,
        const char* function, const char* file, unsigned line,
        const string& message)
{
    // Allocate buffer.
    size_t size = strlen(type) + strlen(expr) + strlen(function) + strlen(file)
                    + message.size() + 150;
    char* msg = new char[size];
    // Format string.
    snprintf(msg, size, 
            "%s failed: %s\n"
            "Function: %s\n"
            "    File: %s\n"
            "    Line: %u"
            "%s%s",
            type, expr, function, file, line,
            (!message.empty()? "\n Message: " : ""),
            message.c_str());
    // Return as an STL string.
    string result(msg);
    delete[] msg;
    return result;
}

void pl_assert_fail(const char* expr, const char* file, unsigned line,
                    const char* function, const string& message)
{
    // Note that in this function, just like in 'pl_check_fail' below, it is
    // important that the PLERROR statement fits on a single line. This is
    // because otherwise some tests may fail on some compilers, as the line
    // number of a multi-line statement may be ambiguous.
    string msg = get_error_message("Assertion",
            expr, function, file, line, message);
    PLERROR(msg.c_str());
}

void pl_check_fail(const char* expr, const char* file, unsigned line,
        const char* function, const string& message)
{
    string msg = get_error_message("Check",
            expr, function, file, line, message);
    PLERROR(msg.c_str());
}

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
