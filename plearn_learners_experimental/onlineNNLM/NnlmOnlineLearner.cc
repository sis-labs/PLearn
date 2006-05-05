// -*- C++ -*-

// NnlmOnlineLearner.cc
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOnlineLearner.cc */


#include "NnlmOnlineLearner.h"

#include <plearn/math/PRandom.h>
#include <plearn/math/TMat_maths.h>
#include <plearn_learners/online/OnlineLearningModule.h>

#include <plearn/vmat/VMat.h>
// necessary?
#include <plearn_learners_experimental/onlineNNLM/NnlmWordRepresentationLayer.h>
#include <plearn_learners/online/GradNNetLayerModule.h> // only diff is in the prandom seed -> 1
#include <plearn_learners/online/TanhModule.h>
#include <plearn_learners_experimental/onlineNNLM/NnlmOutputLayer.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmOnlineLearner,
    "Trains a Naive Bayes Neural Network Language Model (NBNNLM).",
    "MULTI-LINE \nHELP");

NnlmOnlineLearner::NnlmOnlineLearner()
    :   PLearner(),
        vocabulary_size( -1 ),
        word_representation_size( -1 ),
        context_size( -1 )
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
}

void NnlmOnlineLearner::declareOptions(OptionList& ol)
{
// NOT an option... computed from the train_set's dictionary
/*
    declareOption(ol, "vocabulary_size",
                  &NnlmOnlineLearner::vocabulary_size,
                  OptionBase::buildoption,
                  "size of vocabulary used - defines the virtual input size");
*/
    declareOption(ol, "word_representation_size",
                  &NnlmOnlineLearner::word_representation_size,
                  OptionBase::buildoption,
                  "size of the real distributed word representation");

// NOT an option... it's the train_set's input size
/*    declareOption(ol, "context_size",
                  &NnlmOnlineLearner::context_size,
                  OptionBase::buildoption,
                  "size of word context");
*/

    declareOption(ol, "modules", &NnlmOnlineLearner::modules,
                  OptionBase::buildoption,
                  "Layers of the learner");

    declareOption(ol, "output_module", &NnlmOnlineLearner::output_module,
                  OptionBase::buildoption,
                  "Output layer");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NnlmOnlineLearner::buildLayers()
{

    // NnlmWordRep + GradNNet + Tanh
    nmodules = 3;

    modules.resize( nmodules );

    // *** Word representation layer

    PP< NnlmWordRepresentationLayer > p_wrl = new NnlmWordRepresentationLayer();
  
// This is done in the NnlmWordRepresentationLayer  
// BUT in the build... so we need to do it here, because size compality check
// is performed before the build
    p_wrl->input_size = inputsize();
    p_wrl->output_size = context_size * word_representation_size;
    p_wrl->vocabulary_size = vocabulary_size;
    p_wrl->word_representation_size = word_representation_size;
    p_wrl->context_size = context_size;

    modules[0] = p_wrl;


    // *** GradNNetLayer

    PP< GradNNetLayerModule > p_nnl = new GradNNetLayerModule();
  
    p_nnl->input_size = context_size * word_representation_size;
    // HARDCODED FOR NOW
    p_nnl->output_size = 500;
    p_nnl->start_learning_rate = 0.01;

    modules[1] = p_nnl;


    // *** Tanh layer

    PP< TanhModule > p_thm = new TanhModule();
  
    // HARDCODED FOR NOW
    p_thm->input_size = 500;
    p_thm->output_size = 500;

    modules[2] = p_nnl;


////////////////////////////////////////////////////////////

    // ***  Check on layer size compatibilities and resize values and gradients
    // variables


    values.resize( nmodules+1 );
    gradients.resize( nmodules+1 );

    // first values will be "input" values
    int size = inputsize();

    values[0].resize( size );
    gradients[0].resize( size );


    for( int i=0 ; i<nmodules ; i++ )
    {
        PP<OnlineLearningModule> p_module = modules[i];

        if( p_module->input_size != size )
        {
            PLWARNING( "StackedModulesLearner::buildLayers(): module '%d'\n"
                       "has an input size of '%d', but previous layer's output"
                       " size\n"
                       "is '%d'. Resizing module '%d'.\n",
                       i, p_module->input_size, size, i);
            p_module->input_size = size;
        }

        p_module->estimate_simpler_diag_hessian = true;

        p_module->build();

        size = p_module->output_size;
        values[i+1].resize( size );
        gradients[i+1].resize( size );
    }



    // ***  Cost module
    output_module = new NnlmOutputLayer();
    
    output_module->vocabulary_size= vocabulary_size;
    output_module->word_representation_size = word_representation_size;
    output_module->context_size = context_size;
    
    output_module->build();


}

void NnlmOnlineLearner::build_()
{

    // if train_set is not set, we don't know inputsize or targetsize
    if( train_set )
    {

/*        // * PRETREAT TRAIN_SET
        int ds = (train_set->getDictionary(0))->size();

        for( int i = 0; i < train_set->length(); i++ )  {
          for( int j = 0; j < inputsize(); j++ )  {
            // replace nan input by missing input tag
            if( is_missing( train_set(i,j) ) )  {
              train_set(i,j) = ( ds + 1.0 );
            }
          }
          // replace nan target by OOV
          if( is_missing( train_set(i, inputsize()) ) ) {
            train_set( i, inputsize() ) = 0.0;
          }

        }
 */
/*
            train_set->getExample( sample, input, target, weight );
            //DO THIS IN PRETREATMENT!!!!!!!!!!
            //DO THIS IN PRETREATMENT!!!!!!!!!!
            //replace nan by '(train_set->getDictionary(0))->size()+1' the
            //missing value tag
            for( int i=0 ; i < inputsize() ; i++ ) {
              if( is_missing(input[i]) )  {
                input[i] = (train_set->getDictionary(0))->size()+1;
              }
            }

            // Replace a 'nan' in the target by OOV
            // this nan should not be missing data (seeing the train_set is a
            // ProcessSymbolicSequenceVMatrix)
            // but the word "nan", ie "Mrs Nan said she would blabla"
            // *** Problem however - current vocabulary is full for train_set,
            // ie we train OOV on nan-word instances.
            // *** Problem however - current vocabulary is full for train_set,
            // ie we train OOV on nan-word instances.
            // DO a pretreatment to replace Nan by *Nan* or something like it
              if( is_missing(target[0]) ) {
                  target[0] = 0;
              }
*/

        // *** Sanity Checks
        // on context_size, word_representation_size


        // *** Set some variables

        // vocabulary size (voc +1 for OOV +1 for missing)
        vocabulary_size = (train_set->getDictionary(0))->size()+2;

        context_size = inputsize();


        // *** Build modules and output_module
        buildLayers();


    }




}

// ### Nothing to add here, simply calls build_
void NnlmOnlineLearner::build()
{
    inherited::build();
    build_();
}


void NnlmOnlineLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
    deepCopyField(output_module, copies);

    deepCopyField(values, copies);
    deepCopyField(gradients, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NnlmOnlineLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//! This is the output size of the layer before the cost layer
int NnlmOnlineLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).

    if( nmodules < 0 || values.length() <= nmodules )
        return -1;
    else
        return values[ nmodules ].length();
}

void NnlmOnlineLearner::forget()
{
    inherited::forget();

   // reset inputs
    values[0].clear();
    gradients[0].clear();

    // reset modules and outputs
    for( int i=0 ; i<nmodules ; i++ )
    {
        modules[i]->forget();
        values[i+1].clear();
        gradients[i+1].clear();
    }

    output_module->forget();

    stage = 0;

}

void NnlmOnlineLearner::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */

    Vec input( inputsize() );
    Vec target( targetsize() );
    Vec output( outputsize() );
    real weight;
    Vec train_costs( 1 );

    int nsamples = train_set->length();

    if( !initTrain() )
        return;

    // *** For stages
    for( ; stage < nstages ; stage++ )
    {

        // * clear stats of previous epoch
        train_stats->forget();

        // * for examples
        for( int sample=0 ; sample < nsamples ; sample++ )
        //int sample=0 ;
        //for( int s=0 ; s < nsamples ; s++ )
        {

            if(sample%10000 == 0)
              cout << "stage " << stage << " sample " << sample << endl;

            train_set->getExample( sample, input, target, weight );


            //DO THIS IN PRETREATMENT!!!!!!!!!!
            //DO THIS IN PRETREATMENT!!!!!!!!!!
            //replace nan by '(train_set->getDictionary(0))->size()+1' the
            //missing value tag
            for( int i=0 ; i < inputsize() ; i++ ) {
              if( is_missing(input[i]) )  {
                input[i] = (train_set->getDictionary(0))->size()+1;
              }
            }

            // Replace a 'nan' in the target by OOV
            // this nan should not be missing data (seeing the train_set is a
            // ProcessSymbolicSequenceVMatrix)
            // but the word "nan", ie "Mrs Nan said she would blabla"
            // *** Problem however - current vocabulary is full for train_set,
            // ie we train OOV on nan-word instances.
            // *** Problem however - current vocabulary is full for train_set,
            // ie we train OOV on nan-word instances.
            // DO a pretreatment to replace Nan by *Nan* or something like it
              if( is_missing(target[0]) ) {
                  target[0] = 0;
              }


            // - fprop
            computeOutput(input, output);

            output_module->setCurrentWord( (int) target[0]);
            output_module->fprop( output, train_costs);

            // bprop
            Vec out_gradient(1,1); // the gradient wrt the cost is '1'

            // bpropUpdate
            output_module->bpropUpdate( output,
                                          train_costs.subVec(0,1),
                                          gradients[ nmodules ],
                                          out_gradient );

            for( int i=nmodules-1 ; i>0 ; i-- )
                modules[i]->bpropUpdate( values[i], values[i+1],
                                         gradients[i], gradients[i+1] );

            //
            modules[0]->bpropUpdate( values[0], values[1], gradients[1] );

            train_stats->update( train_costs );


            // output result
            if( sample < 15)  {
              pout << "*train cost " << train_costs << endl;
/*              cout << "----- VALUE -----" << endl;
              for( int i=0 ; i<=nmodules ; i++ ) {
                  pout << "*�age " << i << endl;
                  pout << values[i] << endl;
              }
              pout << "*train cost " << train_costs << endl;

              cout << endl;

              cout << "----- GRADIENTS -----" << endl;
              for( int i=0 ; i<=nmodules ; i++ ) {
                  pout << "*�age " << i << endl;
                  pout << gradients[i] << endl;
              }

              pout << endl << endl << endl << endl;*/
            }

        }

        train_stats->finalize(); // finalize statistics for this epoch
    }



}

/*
void NBnnlmLearner::test(VMat testset) {

    Vec input( inputsize() );
    Vec target( targetsize() );
    Vec output( outputsize() );
    real weight;
    Vec test_costs( 1 );

    int nsamples = testset->length();

    // * for examples

    real entropy = 0.0;
    real perplexity = 0.0;

    //cout << "testing 30 first samples" << endl;
    for( int sample=0 ; sample < nsamples ; sample++ )
    //for( int sample=0 ; sample < 30 ; sample++ )
    {

        if(sample%10000 == 0)
          cout << "*";

        testset->getExample( sample, input, target, weight );


        //DO THIS IN PRETREATMENT!!!!!!!!!!
        //DO THIS IN PRETREATMENT!!!!!!!!!!
        //replace nan by '(train_set->getDictionary(0))->size()+1' the missing
        //value tag
        for( int i=0 ; i < inputsize() ; i++ ) {
          if( is_missing(input[i]) )  {
            input[i] = (train_set->getDictionary(0))->size()+1;
          }
        }

        // Replace a 'nan' in the target by OOV
        // this nan should not be missing data (seeing the train_set is a
        // ProcessSymbolicSequenceVMatrix)
        // but the word "nan", ie "Mrs Nan said she would blabla"
        // *** Problem however - current vocabulary is full for train_set, ie
        // we train OOV on nan-word instances.
        // *** Problem however - current vocabulary is full for train_set, ie
        // we train OOV on nan-word instances.
        // DO a pretreatment to replace Nan by *Nan* or something like it
        if( is_missing(target[0]) ) {
            target[0] = 0;
        }


        // - fprop
        computeOutput(input, output);

        test_output_module->setCurrentWord( (int) target[0]);
        test_output_module->setPreviousWord( (int) input[inputsize()-1]);

        test_output_module->fprop( output, test_costs);

        if(sample<15)
          cout << "test cost for sample " << sample << " " << test_costs[0] <<
endl;

        entropy += test_costs[0];

    }


    entropy /= nsamples;
    perplexity = safeexp(entropy);

    cout << "entropy: " << entropy << " perplexity " << perplexity << endl;



}
*/


void NnlmOnlineLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);

    values[0] << input;


    // fprop
    for( int i=0 ; i<nmodules ; i++ )
        modules[i]->fprop( values[i], values[i+1] );

    output.resize( outputsize() );
    output << values[ nmodules ];

}

void NnlmOnlineLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output.
// ...
    output_module->setCurrentWord( (int)target[0] );
    output_module->fprop( output, costs);

}

TVec<string> NnlmOnlineLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ...
return cost_funcs;
}

TVec<string> NnlmOnlineLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    // ...
return cost_funcs;
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
