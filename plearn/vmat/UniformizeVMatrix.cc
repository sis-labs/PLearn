// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


/* *******************************************************      
 * $Id$
 ******************************************************* */

#include "UniformizeVMatrix.h"

namespace PLearn {
using namespace std;


/** UniformizeVMatrix **/

PLEARN_IMPLEMENT_OBJECT(UniformizeVMatrix, "ONE LINE DESC", "NO HELP");

UniformizeVMatrix::UniformizeVMatrix()
    : a(0), b(1)
{
}

UniformizeVMatrix::UniformizeVMatrix(VMat the_distr, Mat the_bins, Vec the_index, real the_a,
                                     real the_b)
    : inherited(the_distr->length(), the_distr->width()),
      distr(the_distr), bins(the_bins), index(the_index), a(the_a), b(the_b)
{
    build();
}

void
UniformizeVMatrix::build()
{
    inherited::build();
    build_();
}

void
UniformizeVMatrix::build_()
{
    if (distr) {
        fieldinfos = distr->getFieldInfos();
  
        if (a >= b)
            PLERROR("In UniformizeVMatrix: a (%f) must be strictly smaller than b (%f)", a, b);
        if (index.length() != bins.length())
            PLERROR("In UniformizeVMatrix: the number of elements in index (%d) must equal the number of rows in bins (%d)", index.length(), bins.length());
        if (min(index)<0 || max(index)>distr->length()-1)
            PLERROR("In UniformizeVMatrix: all values of index must be in range [0,%d]",
                    distr->length()-1);
    }
}

void
UniformizeVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &UniformizeVMatrix::distr, OptionBase::buildoption, "");
    declareOption(ol, "bins", &UniformizeVMatrix::bins, OptionBase::buildoption, "");
    declareOption(ol, "index", &UniformizeVMatrix::index, OptionBase::buildoption, "");
    declareOption(ol, "a", &UniformizeVMatrix::a, OptionBase::buildoption, "");
    declareOption(ol, "b", &UniformizeVMatrix::b, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void UniformizeVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In UniformizeVMatrix::getNewRow OUT OF BOUNDS");
    if(v.length() != width())
        PLERROR("In UniformizeVMatrix::getNewRow v.length() must be equal to the VMat's width");
#endif

    distr->getRow(i, v);
    for(int j=0; j<v.length(); j++) {
        if (vec_find(index, (real)j) != -1) {
            Vec x_bin = bins(j);
            real xx = estimatedCumProb(v[j], x_bin);
            v[j] = xx*(b-a) - a;
        }
    }
}

} // end of namespcae PLearn


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
