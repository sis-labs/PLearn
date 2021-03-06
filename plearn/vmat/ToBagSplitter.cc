// -*- C++ -*-

// ToBagSplitter.cc
//
// Copyright (C) 2004 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file ToBagSplitter.cc */


#include "ToBagSplitter.h"
#include <plearn/var/SumOverBagsVariable.h>  //!< For SumOverBagsVariable::TARGET_COLUMN_LAST.
#include <plearn/vmat/SelectColumnsVMatrix.h>
#include <plearn/vmat/SelectRowsVMatrix.h>

namespace PLearn {
using namespace std;

///////////////////
// ToBagSplitter //
///////////////////
ToBagSplitter::ToBagSplitter():
    expected_size_of_bag(10),
    provide_target(false),
    remove_bag(false)
{}

PLEARN_IMPLEMENT_OBJECT(ToBagSplitter,
        "A Splitter that makes any existing splitter operate on bags only.",
        "The dataset provided must contain bag information, as described in\n"
        "SumOverBagsVariable."
);

////////////////////
// declareOptions //
////////////////////
void ToBagSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "expected_size_of_bag",
                  &ToBagSplitter::expected_size_of_bag,
                  OptionBase::buildoption,
        "The expected size of each bag (optional).");

    declareOption(ol, "sub_splitter",
                  &ToBagSplitter::sub_splitter, OptionBase::buildoption,
        "The underlying splitter we want to make operate on bags.");

    declareOption(ol, "remove_bag",
                  &ToBagSplitter::remove_bag, OptionBase::buildoption,
        "If true, then the bag column will be removed from the data splits.");

    declareOption(ol, "provide_target",
                  &ToBagSplitter::provide_target, OptionBase::buildoption,
        "If true, then the target (without the bag info) of a bag will be\n"
        "provided to the underlying splitter. This target is obtained from\n"
        "the first sample in each bag.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ToBagSplitter::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ToBagSplitter::build_()
{
    if (dataset) {
        // Prepare the bags index list.
        int max_ninstances = 1;
        // The first column in bags_store gives the number of instances in the bag,
        // and the following columns give the indices of the corresponding rows in
        // the original dataset.
        Mat bags_store(dataset->length() / expected_size_of_bag + 1, expected_size_of_bag + 1);
        int num_bag = 0;
        int num_instance = 0;
        int bag_signal_column = dataset->inputsize() + dataset->targetsize() - 1; // Bag signal in the last target column.
        for (int i = 0; i < dataset->length(); i++) {
            while (num_instance + 1 >= bags_store.width()) {
                // Need to resize bags_store.
                bags_store.resize(bags_store.length(), bags_store.width() * 2, 0, true);
            }
            if (num_instance >= max_ninstances) {
                max_ninstances = num_instance + 1;
            }
            bags_store(num_bag, num_instance + 1) = i;
            num_instance++;
            if (int(dataset->get(i, bag_signal_column)) & SumOverBagsVariable::TARGET_COLUMN_LAST) {
                // Last element of a bag.
                bags_store(num_bag, 0) = num_instance; // Store the number of instances in this bag.
                num_bag++;
                num_instance = 0;
                if (num_bag >= bags_store.length()) {
                    // Need to resize bags_store.
                    bags_store.resize(bags_store.length() * 2, bags_store.width(), 0, true);
                }
            }
        }
        // Resize to the minimum size needed.
        bags_store.resize(num_bag, max_ninstances + 1, 0, true);
        int bags_store_is = max_ninstances + 1;
        int bags_store_ts = 0;
        if (provide_target) {
            if (dataset->targetsize() <= 1)
                PLWARNING("In ToBagSplitter::build_ - 'provide_target' is true,"
                        " but the dataset does not seem to have any target "
                        "besides the bag information: no target provided to "
                        "the underlying splitter");
            else {
                bags_store_ts = dataset->targetsize() - 1;
                bags_store.resize(bags_store.length(),
                                  bags_store.width() + bags_store_ts,
                                  0, true);
                Vec input, target;
                real weight;
                for (int i = 0; i < bags_store.length(); i++) {
                    dataset->getExample(int(round(bags_store(i, 1))),
                                        input, target, weight);
                    bags_store(i).subVec(bags_store_is, bags_store_ts) <<
                        target.subVec(0, bags_store_ts);
                }
            }
        }
        bags_index = VMat(bags_store);
        bags_index->defineSizes(bags_store_is, bags_store_ts, 0);
        //bags_index->savePMAT("HOME:tmp/bid.pmat");
        // Provide this index to the sub_splitter.
        sub_splitter->setDataSet(bags_index);
    }
}

//////////////
// getSplit //
//////////////
TVec<VMat> ToBagSplitter::getSplit(int k)
{
    TVec<VMat> sub_splits = sub_splitter->getSplit(k);
    TVec<VMat> result;
    for (int i = 0; i < sub_splits.length(); i++) {
        // Get the list of corresponding indices in the original dataset.
        Mat indices = sub_splits[i].toMat();
        // Turn it into a TVec<int>.
        TVec<int> indices_int;
        for (int j = 0; j < indices.length(); j++) {
            for (int q = 0; q < indices(j, 0); q++) {
                int indice = int(indices(j, q + 1));
                indices_int.append(indice);
            }
        }
        VMat selected_rows = new SelectRowsVMatrix(dataset, indices_int);
        if (remove_bag) {
            // Remove the bag column.
            int bag_column = selected_rows->inputsize() +
                             selected_rows->targetsize() - 1;
            TVec<int> removed_col(1, bag_column);
            PP<SelectColumnsVMatrix> new_sel =
                new SelectColumnsVMatrix(selected_rows, removed_col, false);
            new_sel->inverse_fields_selection = true;
            new_sel->defineSizes(selected_rows->inputsize(),
                                 selected_rows->targetsize() - 1,
                                 selected_rows->weightsize(),
                                 selected_rows->extrasize());
            new_sel->build();
            selected_rows = get_pointer(new_sel);
        }
        result.append(selected_rows);
    }
    return result;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ToBagSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(bags_index, copies);
    deepCopyField(sub_splitter, copies);

}

///////////////////
// nSetsPerSplit //
///////////////////
int ToBagSplitter::nSetsPerSplit() const
{
    // ### Return the number of sets per split
    return sub_splitter->nSetsPerSplit();
}

/////////////
// nsplits //
/////////////
int ToBagSplitter::nsplits() const
{
    return sub_splitter->nsplits();
}

////////////////
// setDataSet //
////////////////
void ToBagSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    // Need to recompute the bags index.
    build();
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
