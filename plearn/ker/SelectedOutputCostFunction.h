// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: SelectedOutputCostFunction.h,v 1.4 2004/04/07 23:15:58 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SelectedOutputCostFunction_INC
#define SelectedOutputCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



//!  This allows to apply a costfunction on a single output element (and correponding target element)
//!  of a larger output vector, rather than on the whole vector
class SelectedOutputCostFunction: public Kernel
{
    typedef Kernel inherited;
		
protected:
  Ker costfunc;
  int outputindex;

public:
    SelectedOutputCostFunction() : costfunc(), outputindex() {}
    SelectedOutputCostFunction(Ker the_costfunc, int the_outputindex)
        : costfunc(the_costfunc), outputindex(the_outputindex) {}

    PLEARN_DECLARE_OBJECT(SelectedOutputCostFunction);

  virtual string info() const
        { return "selected_output[" + tostring(outputindex) + "] "+costfunc->info(); }

  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real evaluate(const Vec& output, const Vec& target) const; 

protected:
    //!  recognized option are "outputindex" and "costfunc"
    static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(SelectedOutputCostFunction);

} // end of namespace PLearn

#endif

