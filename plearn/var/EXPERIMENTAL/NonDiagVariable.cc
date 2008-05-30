// -*- C++ -*-

// DiagVariable.cc
//
// Copyright (C) 2008 Pascal Vincent
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

/*! \file NonDiagVariable.cc */

#include "NonDiagVariable.h"
// #include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** NonDiagVariable **/

PLEARN_IMPLEMENT_OBJECT(
        NonDiagVariable,
        "Extracts all non-diagonal elements from a matrix source variable as a column vector.",
        "Resulting variable is a column vector."
);

////////////////////
// NonDiagVariable //
////////////////////

NonDiagVariable::NonDiagVariable()
{}

NonDiagVariable::NonDiagVariable(Variable* input, bool call_build_):
    inherited(input, input->length()*input->width()-min(input->length(), input->width()), 1, call_build_)
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void NonDiagVariable::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NonDiagVariable::build_() {
    // Nothing to do here.
}

///////////////////
// recomputeSize //
///////////////////
void NonDiagVariable::recomputeSize(int& l, int& w) const
{
    if (input) 
    {
        l = input->length()*input->width()-min(input->length(), input->width());
        w = 1;
    } 
    else
        l = w = 0;
}


void NonDiagVariable::fprop()
{
    Mat V = input->matValue;
    int l = V.length();
    int w = V.width();
    int k=0;
    for(int i=0; i<l; i++)
        for(int j=0; j<w; j++)
            if(i!=j)
                value[k++] = V(i,j);
}

void NonDiagVariable::bprop()
{
    Mat G = input->matGradient;
    int l = G.length();
    int w = G.width();
    int k=0;
    for(int i=0; i<l; i++)
        for(int j=0; j<w; j++)
            if(i!=j)
                G(i,j) += gradient[k++];
}


void NonDiagVariable::bbprop()
{
    PLERROR("bbprop not implemented for this variable");
}


void NonDiagVariable::symbolicBprop()
{
    PLERROR("symbolic bprop not yet implemented for this variable");
}


void NonDiagVariable::rfprop()
{
    PLERROR("rfprop no yet implemented for this vairable");
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
