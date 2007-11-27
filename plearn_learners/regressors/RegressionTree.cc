// -*- C++ -*-

// RegressionTree.cc
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
 * $Id: RegressionTree.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "RegressionTree.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTree,
                        "Regression tree algorithm", 
                        "Algorithm built to serve as a base regressor for the LocalMedianBoost algorithm.\n"
                        "It can also be used as a stand alone learner.\n"
                        "It can learn from a weighted train set to represent different distribution on the training set.\n"
                        "It can separate a confidence fonction from the output whenmaking a prediction.\n"
                        "At each node expansion, it splits the node to maximize the improvement of an objective function\n"
                        "with the mean square error and a facto of the confidence funtion.\n"
                        "At each node expansion, it creates 3 nodes, one to hold any samples with a missing value on the\n"
                        "splitting attribute, one for the samples with values less than the value of the splitting attribute\n"
                        "and one fr the others.\n"
    );

RegressionTree::RegressionTree()     
    : missing_is_valid(false),
      loss_function_weight(1.0),
      maximum_number_of_nodes(400),
      compute_train_stats(1),
      complexity_penalty_factor(0.0)
{
}

RegressionTree::~RegressionTree()
{
}

void RegressionTree::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "missing_is_valid", &RegressionTree::missing_is_valid, OptionBase::buildoption,
                  "If set to 1, missing values will be treated as valid, and missing nodes will be potential for splits.\n");
    declareOption(ol, "loss_function_weight", &RegressionTree::loss_function_weight, OptionBase::buildoption,
                  "The hyper parameter to balance the error and the confidence factor.\n");
    declareOption(ol, "maximum_number_of_nodes", &RegressionTree::maximum_number_of_nodes, OptionBase::buildoption,
                  "The maximum number of nodes for this tree.\n"
                  "(If less than nstages, nstages will be used).");
    declareOption(ol, "compute_train_stats", &RegressionTree::compute_train_stats, OptionBase::buildoption,
                  "If set to 1 (the default value) the train statistics are computed.\n"
                  "(When using the tree as a base regressor, we dont need the stats and it goes quicker when computations are suppressed).");
    declareOption(ol, "complexity_penalty_factor", &RegressionTree::complexity_penalty_factor, OptionBase::buildoption,
                  "A factor that is multiplied with the square root of the number of leaves.\n"
                  "If the error inprovement for the next split is less than the result, the algorithm proceed to an early stop."
                  "(When set to 0.0, the default value, it has no impact).");
    declareOption(ol, "multiclass_outputs", &RegressionTree::multiclass_outputs, OptionBase::buildoption,
                  "A vector of possible output values when solving a multiclass problem.\n"
                  "When making a prediction, the tree will adjust the output value of each leave to the closest value provided in this vector.");
    declareOption(ol, "leave_template", &RegressionTree::leave_template, OptionBase::buildoption,
                  "The template for the leave objects to create.\n");
    declareOption(ol, "sorted_train_set", &RegressionTree::sorted_train_set, OptionBase::buildoption, 
                  "The train set sorted on all columns\n"
                  "If it is not provided by a wrapping algorithm, it is created at stage 0.\n");
      
    declareOption(ol, "root", &RegressionTree::root, OptionBase::learntoption,
                  "The root node of the tree being built\n");
    declareOption(ol, "priority_queue", &RegressionTree::priority_queue, OptionBase::learntoption,
                  "The heap to store potential nodes to expand\n");
    declareOption(ol, "first_leave", &RegressionTree::first_leave, OptionBase::learntoption,
                  "The first leave built with the root containing all train set rows at the beginning\n");
    declareOption(ol, "first_leave_output", &RegressionTree::first_leave_output, OptionBase::learntoption,
                  "The vector to compute the ouput and the confidence function of the first leave.\n");
    declareOption(ol, "first_leave_error", &RegressionTree::first_leave_error, OptionBase::learntoption,
                  "The vector to compute the errors of the first leave.\n");
    declareOption(ol, "split_cols", &RegressionTree::split_cols, OptionBase::learntoption,
                  "contain in order of first to last the columns used to split the tree.\n");
    inherited::declareOptions(ol);
}

void RegressionTree::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(missing_is_valid, copies);
    deepCopyField(loss_function_weight, copies);
    deepCopyField(maximum_number_of_nodes, copies);
    deepCopyField(compute_train_stats, copies);
    deepCopyField(complexity_penalty_factor, copies);
    deepCopyField(multiclass_outputs, copies);
    deepCopyField(leave_template, copies);
    deepCopyField(sorted_train_set, copies);
    deepCopyField(root, copies);
    deepCopyField(priority_queue, copies);
    deepCopyField(first_leave, copies);
    deepCopyField(first_leave_output, copies);
    deepCopyField(first_leave_error, copies);
    deepCopyField(split_cols, copies);
}

void RegressionTree::build()
{
    inherited::build();
    build_();
}

void RegressionTree::build_()
{
    if(sorted_train_set)
    {
        length = sorted_train_set->length();
        
        if (length < 1) PLERROR("RegressionTree: the training set must contain at least one sample, got %d", length);
        inputsize = sorted_train_set->inputsize();
        targetsize = sorted_train_set->targetsize();
        weightsize = sorted_train_set->weightsize();
        if (inputsize < 1) PLERROR("RegressionTree: expected  inputsize greater than 0, got %d", inputsize);
        if (targetsize != 1) PLERROR("RegressionTree: expected targetsize to be 1, got %d", targetsize);
        if (weightsize != 1 && weightsize != 0)  PLERROR("RegressionTree: expected weightsize to be 1 or 0, got %d", weightsize);
        sample_input.resize(inputsize);
        sample_target.resize(targetsize);
    }
    else if (train_set)
    { 
        length = train_set->length();
        if (length < 1) PLERROR("RegressionTree: the training set must contain at least one sample, got %d", length);
        inputsize = train_set->inputsize();
        targetsize = train_set->targetsize();
        weightsize = train_set->weightsize();
        if (inputsize < 1) PLERROR("RegressionTree: expected  inputsize greater than 0, got %d", inputsize);
        if (targetsize != 1) PLERROR("RegressionTree: expected targetsize to be 1, got %d", targetsize);
        if (weightsize != 1 && weightsize != 0)  PLERROR("RegressionTree: expected weightsize to be 1 or 0, got %d", weightsize);
        sample_input.resize(inputsize);
        sample_target.resize(targetsize);
    }

    sample_output.resize(2);
    sample_costs.resize(4);
    if (loss_function_weight != 0.0)
    {
        l2_loss_function_factor = 2.0 / pow(loss_function_weight, 2);
        l1_loss_function_factor = 2.0 / loss_function_weight;
    }
    else
    {
        l2_loss_function_factor = 1.0;
        l1_loss_function_factor = 1.0;
    }
}

void RegressionTree::train()
{
    if (stage == 0) initialiseTree();
    PP<ProgressBar> pb;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTree : train stages: ", nstages);
    }
    for (; stage < nstages; stage++)
    {    
        if (stage > 0)
        {
            int split_col = expandTree();
            split_cols.append(split_col);
            if (split_col < 0) break;
        }
        if (report_progress) pb->update(stage);
    }
    if (compute_train_stats < 1) return;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTree : computing the statistics: ", length);
    } 
    train_stats->forget();
    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {  
        sorted_train_set->getExample(each_train_sample_index, sample_input, sample_target, sample_weight);
        computeOutput(sample_input, sample_output);
        computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_costs); 
        train_stats->update(sample_costs);
        if (report_progress) pb->update(each_train_sample_index);
    }
    train_stats->finalize();
    verbose("split_cols: "+tostring(split_cols),2);
}

void RegressionTree::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        pout << the_msg << endl;
}

void RegressionTree::forget()
{
    stage = 0;
}

void RegressionTree::initialiseTree()
{
    if (!sorted_train_set)
    {
        sorted_train_set = new RegressionTreeRegisters();
        sorted_train_set->setOption("report_progress", tostring(report_progress));
        sorted_train_set->setOption("verbosity", tostring(verbosity));
        sorted_train_set->initRegisters(train_set);
    }
    else
    {
        sorted_train_set->reinitRegisters();
    }
    //Set value common value of all leave
    // for optimisation, by default they aren't missing leave
    leave_template->setOption("missing_leave", "0");
    leave_template->setOption("loss_function_weight", tostring(loss_function_weight));
    leave_template->setOption("verbosity", tostring(verbosity));
    leave_template->initStats();

    first_leave_output.resize(2);
    first_leave_error.resize(3);
    first_leave = ::PLearn::deepCopy(leave_template);
    first_leave->setOption("id", tostring(sorted_train_set->getNextId()));
    first_leave->initLeave(sorted_train_set);

    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        first_leave->addRow(each_train_sample_index, first_leave_output, first_leave_error);
        first_leave->registerRow(each_train_sample_index);
    }
    root = new RegressionTreeNode();
    root->setOption("missing_is_valid", tostring(missing_is_valid));
    root->setOption("loss_function_weight", tostring(loss_function_weight));
    root->setOption("verbosity", tostring(verbosity));
    root->initNode(sorted_train_set, first_leave, leave_template);
    root->lookForBestSplit();
    if (maximum_number_of_nodes < nstages) maximum_number_of_nodes = nstages;
    priority_queue = new RegressionTreeQueue();
    priority_queue->setOption("verbosity", tostring(verbosity));
    priority_queue->setOption("maximum_number_of_nodes", tostring(maximum_number_of_nodes));
    priority_queue->initHeap();
    priority_queue->addHeap(root);
}

int RegressionTree::expandTree()
{
    if (priority_queue->isEmpty() <= 0)
    {
        verbose("RegressionTree: priority queue empty, stage: " + tostring(stage), 3);
        return -1;
    }
    node = priority_queue->popHeap();
    if (node->getErrorImprovment() < complexity_penalty_factor * sqrt((real)stage))
    {
        verbose("RegressionTree: early stopping at stage: " + tostring(stage)
                + ", error improvement: " + tostring(node->getErrorImprovment())
                + ", penalty: " + tostring(complexity_penalty_factor * sqrt((real)stage)), 3);
        return -1;
    }
    int split_col = node->expandNode();
    if (split_col < 0)
    {
        verbose("RegressionTree: expand is negative?", 3);
        return -1;
    }
    TVec< PP<RegressionTreeNode> > subnode = node->getNodes();
    priority_queue->addHeap(subnode[0]); 
    priority_queue->addHeap(subnode[1]);
    if (missing_is_valid) priority_queue->addHeap(subnode[2]);
    return split_col; 
}

void RegressionTree::setSortedTrainSet(PP<RegressionTreeRegisters> the_sorted_train_set)
{
    sorted_train_set = the_sorted_train_set;
    build();
}

int RegressionTree::outputsize() const
{
    return 2;
}

TVec<string> RegressionTree::getTrainCostNames() const
{
    TVec<string> return_msg(4);
    return_msg[0] = "mse";
    return_msg[1] = "base_confidence";
    return_msg[2] = "base_reward_l2";
    return_msg[3] = "base_reward_l1";
    return return_msg;
}

TVec<string> RegressionTree::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void RegressionTree::computeOutput(const Vec& inputv, Vec& outputv) const
{
    root->computeOutput(inputv, outputv);
    if (multiclass_outputs.length() <= 0) return;
    real closest_value=multiclass_outputs[0];
    real margin_to_closest_value=abs(outputv[0] - multiclass_outputs[0]);
    for (int value_ind = 1; value_ind < multiclass_outputs.length(); value_ind++)
    {
        real v=abs(outputv[0] - multiclass_outputs[value_ind]);
        if (v < margin_to_closest_value)
        {
            closest_value = multiclass_outputs[value_ind];
            margin_to_closest_value = v;
        }
    }
    outputv[0] = closest_value;
}

void RegressionTree::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    costsv[0] = pow((outputv[0] - targetv[0]), 2);
    costsv[1] = outputv[1];
    costsv[2] = 1.0 - (l2_loss_function_factor * costsv[0]);
    costsv[3] = 1.0 - (l1_loss_function_factor * abs(outputv[0] - targetv[0]));
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
