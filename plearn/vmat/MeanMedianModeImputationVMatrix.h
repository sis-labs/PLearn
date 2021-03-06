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


/* ******************************************************************      
   * $Id: MeanMedianModeImputationVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
   ****************************************************************** */

/*! \file MeanMedianModeImputationVMatrix.h */

#ifndef MeanMedianModeImputationVMatrix_INC
#define MeanMedianModeImputationVMatrix_INC

#include "ImputationVMatrix.h"
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/io/fileutils.h>                     //!<  For isfile()
#include <plearn/math/BottomNI.h>

namespace PLearn {
using namespace std;

class MeanMedianModeImputationVMatrix: public ImputationVMatrix
{
  typedef ImputationVMatrix inherited;
  
public:

  
  //! A referenced train set.
  //! The mean, median or mode is computed with the observed values in this data set.
  //! It is used in combination with the option number_of_train_samples_to_use.
  VMat                          train_set;
  
  //! The number of samples from the train set that will be examined to compute the required statistic for each variable.
  //! If equal to zero, all the samples from the train set are used to calculated the statistics.
  //! If it is a fraction between 0 and 1, this proportion of the samples are used.
  //! If greater or equal to 1, the integer portion is interpreted as the number of samples to use.
  real                          number_of_train_samples_to_use;
  
  //! Pairs of instruction of the form field_name : mean | median | mode | none.
  TVec< pair<string, string> >  imputation_spec;

  //!if true will generate an error if the imputation_spec reference a
  //! field not in the source. Otherwise generate a warning.
  bool                          missing_field_error;
  string                        default_instruction;

                        MeanMedianModeImputationVMatrix();
  virtual               ~MeanMedianModeImputationVMatrix();

  static void           declareOptions(OptionList &ol);

  virtual void          build();
  virtual void          makeDeepCopyFromShallowCopy(CopiesMap& copies);

  virtual real         get(int i, int j) const;
  virtual void         getSubRow(int i, int j, Vec v) const;
  virtual void         getRow(int i, Vec v) const;
  virtual void         getColumn(int i, Vec v) const;
          VMat         getMeanMedianModeFile();

private:
  
  TVec<string>         train_field_names;
  VMat                 mean_median_mode_file;


  //! The vector of variable means observed from the train set.
  Vec                           variable_mean;
  
  //! The vector of variable medians observed from the train set.
  Vec                           variable_median;
  
  //! The vector of variable modes observed from the train set.
  Vec                           variable_mode;
  
  //! The vector of coded instruction for each variables.
  TVec<int>                     variable_imputation_instruction;
  
          void         build_();
  virtual void setMetaDataDir(const PPath& the_metadatadir);
          void         createMeanMedianModeFile(PPath file_name); 
          void         loadMeanMedianModeFile(PPath file_name); 
          void         computeMeanMedianModeVectors();  
  static  void         sortColumn(Vec input_vec, int start, int end);
  static  void         sortSmallSubArray(Vec input_vec, int start_index, int end_index);
  static  void         swapValues(Vec input_vec, int index_i, int index_j);
  static  real         compare(real field1, real field2);
  
  PLEARN_DECLARE_OBJECT(MeanMedianModeImputationVMatrix);

};

DECLARE_OBJECT_PTR(MeanMedianModeImputationVMatrix);

} // end of namespcae PLearn
#endif
