// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: SelectColumnsVMatrix.h,v 1.8 2004/06/29 20:00:58 tihocan Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef SelectColumnsVMatrix_INC
#define SelectColumnsVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;
 
//!  selects variables (columns) from a source matrix
//!  according to given vector of indices.  NC: removed the unused field
//!  raw_sample.
class SelectColumnsVMatrix: public SourceVMatrix
{
  
private:
    
 typedef SourceVMatrix inherited;
  
public:

  //! Public build options
  TVec<int> indices;

public:

  //! Default constructor.
  SelectColumnsVMatrix();
  
  //! The appropriate fieldinfos are copied upon construction
  //! Here the indices will be shared for efficiency. But you should not modify them afterwards!
  SelectColumnsVMatrix(VMat the_source, TVec<int> the_indices);

  //! Here the indices will be copied locally into an integer vector
  SelectColumnsVMatrix(VMat the_source, Vec the_indices);

  PLEARN_DECLARE_OBJECT(SelectColumnsVMatrix);

  static void declareOptions(OptionList &ol);
  virtual void build();
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  void getNewRow(int i, Vec& v) const { getSubRow(i, 0, v); }
  virtual void reset_dimensions() 
  { 
    source->reset_dimensions(); length_=source->length(); 
    for (int i=0;indices.length();i++)
      if (indices[i]>=source->width())
        PLERROR("SelectColumnsVMatrix::reset_dimensions, underlying source not wide enough (%d>=%d)",
            indices[i],source->width());
  }

private:
  void build_();

};

DECLARE_OBJECT_PTR(SelectColumnsVMatrix);

} // end of namespcae PLearn
#endif
