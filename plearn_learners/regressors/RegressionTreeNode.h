// -*- C++ -*-

// RegressionTreeNode.h
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* ********************************************************************************    
 * $Id: RegressionTreeNode.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout     *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#ifndef RegressionTreeNode_INC
#define RegressionTreeNode_INC

#include "RegressionTreeLeave.h"
#include "RegressionTreeMulticlassLeave.h"

namespace PLearn {
using namespace std;

class RegressionTreeNode: public Object
{
    typedef Object inherited;
  
private:

/*
  Build options: they have to be set before building
*/

    int  missing_is_valid;
    real loss_function_weight;
    int verbosity; 
    PP<RegressionTreeLeave> leave_template; 
    PP<RegressionTreeRegisters> train_set;
    PP<RegressionTreeLeave> leave;
 
/*
  Learnt options: they are sized and initialized if need be, in initNode(...)
*/
 
    int length;
    int inputsize;
    int leave_id;
    Vec leave_output;
    Vec leave_error;
    int split_col;
    int split_balance;
    real split_feature_value;
    real after_split_error;
    PP<RegressionTreeNode> missing_node;
    int missing_leave_id;
    PP<RegressionTreeLeave> missing_leave;
    Vec missing_output;
    Vec missing_error;
    PP<RegressionTreeNode> left_node;
    int left_leave_id;
    PP<RegressionTreeLeave> left_leave;
    Vec left_output;
    Vec left_error;
    PP<RegressionTreeNode> right_node;
    int right_leave_id;
    PP<RegressionTreeLeave> right_leave;
    Vec right_output;
    Vec right_error;
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/  

    int row;
    int next_row;
    int col;  
    int work_balance;
    real work_error;    

public:  
    RegressionTreeNode();
    virtual              ~RegressionTreeNode();
    
    PLEARN_DECLARE_OBJECT(RegressionTreeNode);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         initNode(PP<RegressionTreeRegisters> train_set, PP<RegressionTreeLeave> leave, PP<RegressionTreeLeave> leave_template);
    void         lookForBestSplit();
    void         compareSplit(int col, real left_leave_last_feature, real right_leave_first_feature);
    int          expandNode();
    int          getSplitBalance();
    real         getErrorImprovment();
    TVec< PP<RegressionTreeNode> >  getNodes();
    void         computeOutput(const Vec& inputv, Vec& outputv);
    
private:
    void         build_();
    void         verbose(string msg, int level);
};

DECLARE_OBJECT_PTR(RegressionTreeNode);

} // end of namespace PLearn

#endif


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
