// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau

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


/* *******************************************************      
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "LiftOutputVariable.h"

namespace PLearn {
using namespace std;

/** LiftOutputVariable **/

PLEARN_IMPLEMENT_OBJECT(
    LiftOutputVariable,
    "The result is the output if the target is 1, and the opposite of the output "
    "otherwise. This variable is to be used with a LiftStatsCollector, in a"
    "stochastic gradient descent.",
    "NO HELP"
    );

////////////////////////
// LiftOutputVariable //
////////////////////////
LiftOutputVariable::LiftOutputVariable(Variable* netout, Variable* target)
    : inherited(netout,target,1,1)
{
    build_();
}

void
LiftOutputVariable::build()
{
    inherited::build();
    build_();
}

void
LiftOutputVariable::build_()
{
    if (input1 && input2) {
        // input1 and input2 are (respectively) netout and target from the constructor
        if(input1->size() != input2->size())
            PLERROR("In LiftOutputVariable: netout and target must have the same size");
    }
}

///////////////////
// recomputeSize //
///////////////////
void LiftOutputVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }

///////////
// fprop //
///////////
void LiftOutputVariable::fprop()
{
    real output = input1->valuedata[0];
    real target = input2->valuedata[0];
    if (target == 1.0) {
        if (output == 0.0) {
            // We need to make sure the output is positive.
            output = 1e-10;
        }
        valuedata[0] = output;
    } else {
        valuedata[0] = -output;
    }
}

void LiftOutputVariable::bprop()
{
    // TODO Not really sure what to do here, bprop shouldn't be used anyway.
    // Actually it IS called, should investigate why and what it does.
    // PLWARNING("In LiftOutputVariable::bprop - You should not backprop on a LiftOutputVariable");
    real gr = *gradientdata;
    for (int i=0; i<input1->size(); i++)
    {
        input1->gradientdata[i] += gr;
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
