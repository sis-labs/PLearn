// -*- C++ -*-

// LearningNetwork.h
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file LearningNetwork.h */


#ifndef LearningNetwork_INC
#define LearningNetwork_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/EXPERIMENTAL/MatrixModule.h>
#include <plearn_learners/online/EXPERIMENTAL/NetworkConnection.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class LearningNetwork : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    TVec< PP<OnlineLearningModule> > modules;
    TVec< PP<NetworkConnection> > connections;

    PP<OnlineLearningModule> input_module;
    PP<OnlineLearningModule> target_module;
    PP<OnlineLearningModule> weight_module;
    PP<OnlineLearningModule> output_module;
    PP<OnlineLearningModule> cost_module;

    string input_port;
    string target_port;
    string weight_port;
    string output_port;
    string cost_port;

    int batch_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LearningNetwork();

    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

    //! Computes the output from the input.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(LearningNetwork);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Simple module used to initialize the network's inputs.
    PP<MatrixModule> store_inputs;

    //! Simple module used to initialize the network's targets.
    PP<MatrixModule> store_targets;

    //! Simple module used to initialize the network's weights.
    PP<MatrixModule> store_weights;

    //! Simple module that will contain the network's outputs at the end of a
    //! fprop step.
    PP<MatrixModule> store_outputs;

    //! Simple module that will contain the network's costs at the end of a
    //! fprop step.
    PP<MatrixModule> store_costs;

    //! Contains all modules (i.e. those in the 'modules' list, and the modules
    //! storing the inputs, targets, etc).
    TVec< PP<OnlineLearningModule> > all_modules;

    //! Contains all connections (i.e. those in the 'connections' list, and
    //! those added to connect the modules storing the inputs, targets, etc).
    TVec< PP<NetworkConnection> > all_connections;

    //! Ordered list of modules used when doing a fprop (the integer values
    //! correspond to indices in 'all_modules').
    TVec<int> fprop_path;

    //! Ordered list of modules used when doing a bprop (the integer values
    //! correspond to indices in 'all_modules').
    TVec<int> bprop_path;

    //! The i-th element is the list of Mat* pointers being provided to the
    //! i-th module in a fprop step.
    TVec< TVec<Mat*> > fprop_data;

    //! The i-the elment is the list of matrices that need to be resized to
    //! empty matrices prior to calling fprop() on the i-th module in a fprop
    //! step.
    //! The resizing is needed to ensure we correctly compute the desired
    //! outputs.
    TVec< TVec<int> > fprop_toresize;
    
    //! The i-th element is the list of Mat* pointers being provided to the
    //! i-th module in a bprop step.
    TVec< TVec<Mat*> > bprop_data;

    //! The i-th element is the list of matrices that need to be resized to
    //! empty matrices prior to calling bpropUpdate() on the i-th module in a
    //! bprop step.
    //! The resizing is needed to ensure we correctly compute the desired
    //! gradients.
    TVec< TVec<int> > bprop_toresize;

    //! A list of all matrices used to store the various computation results in
    //! the network (i.e. the outputs of each module).
    TVec<Mat> all_mats;
    
    //#####  Protected Options  ###############################################

    int mbatch_size;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Perform one training step for the given batch inputs, targets and
    //! weights.
    void trainingStep(const Mat& inputs, const Mat& targets,
                      const Vec& weights);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LearningNetwork);

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