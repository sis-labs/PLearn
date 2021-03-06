// -*- C++ -*-

// BaggingLearner.h
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file BaggingLearner.h */


#ifndef BaggingLearner_INC
#define BaggingLearner_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/Splitter.h>

namespace PLearn {

/**
 * Learner that trains several sub-learners on 'bags'.
 */
class BaggingLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    PP<Splitter> splitter; //!< splitter used to get bags(=splits)
    PP<PLearner> template_learner; //!< to deep-copy once for each bag
    //! functions used to combine outputs from all learners
    TVec<string> stats;
    //! for computeOutput: remove the highest and lowest 
    //! outputs before averaging (nb. to exclude at each end)
    int exclude_extremes; 
    //! Wether computeOutput should append sub-learners outputs to output.
    bool output_sub_outputs;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    BaggingLearner(PP<Splitter> splitter_= 0, 
                   PP<PLearner> template_learner_= 0,
                   TVec<string> stats_= TVec<string>(1,"E"),
                   int exclude_extremes_= 0,
                   bool output_sub_outputs_= false);

    //#####  PLearner Member Functions  #######################################
    virtual int outputsize() const;

    virtual void forget();

    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    virtual TVec<std::string> getTestCostNames() const;

    virtual TVec<std::string> getTrainCostNames() const;

    virtual int nTestCosts() const;
    virtual int nTrainCosts() const;
    virtual void resetInternalState();
    virtual bool isStatefulLearner() const;

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    virtual TVec<string> getOutputNames() const;

    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //#####  PLearn::Object Protocol  #########################################
    PLEARN_DECLARE_OBJECT(BaggingLearner);

    virtual void build();

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void setExperimentDirectory(const PPath& the_expdir);

protected:
    //#####  Protected Options  ###############################################

    TVec<PP<PLearner> > learners;

protected:
    //#####  Protected Member Functions  ######################################

    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    void build_();

private:
    //#####  Private Data Members  ############################################

protected:
    mutable VecStatsCollector stcol;
    mutable Mat learners_outputs;
    mutable Mat outputs;
    mutable Vec learner_costs;
    mutable Vec last_test_input;


    TVec<string> addStatNames(const TVec<string>& names) const
    {
        TVec<string> outputnames;
        for(TVec<string>::iterator it= names.begin(); it != names.end(); ++it)
            for(TVec<string>::const_iterator jt= stats.begin();
                jt != stats.end(); ++jt)
                outputnames.push_back(*jt + '[' + *it + ']');
        return outputnames;
    }


};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(BaggingLearner);

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
