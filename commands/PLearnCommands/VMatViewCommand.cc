// -*- C++ -*-

// VMatViewCommand.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file VMatViewCommand.cc */


#include "VMatViewCommand.h"
#include <plearn/db/getDataSet.h>
#include <plearn/misc/viewVMat.h>
#include <plearn/vmat/BinaryOpVMatrix.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'VMatViewCommand' command in the command registry
PLearnCommandRegistry VMatViewCommand::reg_(new VMatViewCommand);

VMatViewCommand::VMatViewCommand()
    : PLearnCommand(
        "vmat_view",
        "interactive display of a vmatrix (curses-based)",
        "vmat view [--add,--sub,--diff,--mult,--div] <vmat> ...\n"
        "will interactively display contents of the \n"
        "specified vmatrix (any recognized file format)\n"
        )
{}

//! The actual implementation of the 'VMatViewCommand' command
void VMatViewCommand::run(const vector<string>& args)
{
    string op;
    if(args[0]=="--add")
        op="add";
    else if(args[0]=="--sub" || args[0]=="--diff")
        op="sub";
    else if(args[0]=="--mult")
        op="mult";
    else if(args[0]=="--div")
        op="div";
    if(!op.empty()){
        if(args.size()!=3)
            PLERROR("Usage: vmat_view [--add,--sub,--diff,--mult,--div] <source1> ... \n"
                    "If an option is used their must be two sources matrix.");
        VMat vm = new BinaryOpVMatrix(getDataSet(args[1]),getDataSet(args[2]),op);
        viewVMat(vm, op);
        return;
    }

    for(uint i=0;i<args.size();i++){
        PPath vmat_view_dataset(args[i]);
        if(args.size()>1)
            pout<<vmat_view_dataset<<endl;
        VMat vm = getDataSet(vmat_view_dataset);
        viewVMat(vm, vmat_view_dataset);
    }
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
