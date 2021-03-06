// -*- C++ -*-

// ModuleStackModule.cc
//
// Copyright (C) 2007 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file ModuleStackModule.cc */



#include "ModuleStackModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ModuleStackModule,
    "Wraps a stack of layered OnlineLearningModule into a single one",
    "The OnlineLearningModule's are disposed like superposed layers:\n"
    "outputs of module i are the inputs of module (i+1), the last layer is\n"
    "the output layer.\n"
    );

ModuleStackModule::ModuleStackModule() :
    n_modules(0)
{
}

////////////////////
// declareOptions //
////////////////////
void ModuleStackModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "modules", &ModuleStackModule::modules,
                  OptionBase::buildoption,
                  "The underlying modules");

    declareOption(ol, "n_modules", &ModuleStackModule::n_modules,
                  OptionBase::learntoption,
                  "The number of modules");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "input_size", &ModuleStackModule::input_size,
            OptionBase::nosave,
            "Set at build time.");

    redeclareOption(ol, "output_size", &ModuleStackModule::output_size,
            OptionBase::nosave,
            "Set at build time.");

}

////////////
// build_ //
////////////
void ModuleStackModule::build_()
{
    n_modules = modules.length();

    if( n_modules > 0 )
    {
        values.resize( n_modules-1 );
        gradients.resize( n_modules-1 );
        diag_hessians.resize( n_modules-1 );
        values_m.resize(n_modules - 1);
        gradients_m.resize(n_modules - 1);

        input_size = modules[0]->input_size;
        output_size = modules[n_modules-1]->output_size;
    }
    else
    {
        input_size = -1;
        output_size = -1;
    }

    // If we have a random_gen and some modules do not, share it with them
    if( random_gen )
        for( int i=0; i<n_modules; i++ )
            if( !(modules[i]->random_gen) )
            {
                modules[i]->random_gen = random_gen;
                modules[i]->forget();
            }
}

///////////
// build //
///////////
void ModuleStackModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ModuleStackModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules,          copies);
    deepCopyField(values,           copies);
    deepCopyField(gradients,        copies);
    deepCopyField(diag_hessians,    copies);
    deepCopyField(values_m,         copies);
    deepCopyField(gradients_m,      copies);
}

///////////
// fprop //
///////////
void ModuleStackModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT( n_modules >= 2 );
    PLASSERT( input.size() == input_size );

    modules[0]->fprop( input, values[0] );
    for( int i=1 ; i<n_modules-1 ; i++ )
        modules[i]->fprop( values[i-1], values[i] );
    modules[n_modules-1]->fprop( values[n_modules-2], output );
}

void ModuleStackModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLASSERT( n_modules >= 2 );
    PLASSERT( inputs.width() == input_size );

    modules[0]->fprop( inputs, values_m[0] );
    for( int i=1 ; i<n_modules-1 ; i++ )
        modules[i]->fprop( values_m[i-1], values_m[i] );
    modules[n_modules-1]->fprop( values_m[n_modules-2], outputs );
}

/////////////////
// bpropUpdate //
/////////////////
void ModuleStackModule::bpropUpdate(const Vec& input, const Vec& output,
                                    Vec& input_gradient,
                                    const Vec& output_gradient,
                                    bool accumulate)
{
    PLASSERT( n_modules >= 2 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }

    // bpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bpropUpdate( values[n_modules-2], output,
                                       gradients[n_modules-2],
                                       output_gradient );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bpropUpdate( values[i-1], values[i],
                                 gradients[i-1], gradients[i] );

    modules[0]->bpropUpdate( input, values[0], input_gradient, gradients[0],
                             accumulate );
}

void ModuleStackModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
        Mat& input_gradients,
        const Mat& output_gradients,
        bool accumulate)
{
    PLASSERT( n_modules >= 2 );
    PLASSERT( inputs.width() == input_size );
    PLASSERT( outputs.width() == output_size );
    PLASSERT( output_gradients.width() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == inputs.length(),
                "Cannot resize input_gradients and accumulate into it" );
    } else
        input_gradients.resize(inputs.length(), input_size);

    // bpropUpdate should be called just after the corresponding fprop,
    // so 'values_m' should be up-to-date.
    modules[n_modules-1]->bpropUpdate( values_m[n_modules-2], outputs,
                                       gradients_m[n_modules-2],
                                       output_gradients );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bpropUpdate( values_m[i-1], values_m[i],
                                 gradients_m[i-1], gradients_m[i] );

    modules[0]->bpropUpdate( inputs, values_m[0], input_gradients, gradients_m[0],
            accumulate );
}

void ModuleStackModule::bpropUpdate(const Vec& input, const Vec& output,
                                    const Vec& output_gradient)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    // bpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bpropUpdate( values[n_modules-2], output,
                                       gradients[n_modules-2],
                                       output_gradient );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bpropUpdate( values[i-1], values[i],
                                 gradients[i-1], gradients[i] );

    modules[0]->bpropUpdate( input, values[0], gradients[0] );
}

//////////////////
// bbpropUpdate //
//////////////////
void ModuleStackModule::bbpropUpdate(const Vec& input, const Vec& output,
                                     Vec& input_gradient,
                                     const Vec& output_gradient,
                                     Vec& input_diag_hessian,
                                     const Vec& output_diag_hessian,
                                     bool accumulate)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );
    PLASSERT( output_diag_hessian.size() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
    }

    // bbpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bbpropUpdate( values[n_modules-2], output,
                                        gradients[n_modules-2], output_gradient,
                                        diag_hessians[n_modules-2],
                                        output_diag_hessian );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bbpropUpdate( values[i-1], values[i],
                                  gradients[i-1], gradients[i],
                                  diag_hessians[i-1], diag_hessians[i] );

    modules[0]->bbpropUpdate( input, values[0], input_gradient, gradients[0],
                              input_diag_hessian, diag_hessians[0],
                              accumulate );
}

void ModuleStackModule::bbpropUpdate(const Vec& input, const Vec& output,
                                     const Vec& output_gradient,
                                     const Vec& output_diag_hessian)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );
    PLASSERT( output_diag_hessian.size() == output_size );

    // bbpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bbpropUpdate( values[n_modules-2], output,
                                        gradients[n_modules-2], output_gradient,
                                        diag_hessians[n_modules-2],
                                        output_diag_hessian );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bbpropUpdate( values[i-1], values[i],
                                  gradients[i-1], gradients[i],
                                  diag_hessians[i-1], diag_hessians[i] );

    modules[0]->bbpropUpdate( input, values[0],
                              gradients[0], diag_hessians[0] );
}

////////////
// forget //
////////////
void ModuleStackModule::forget()
{
    values.clear();
    gradients.clear();
    diag_hessians.clear();

    if( !random_gen )
    {
        PLWARNING("ModuleStackModule: cannot forget() without random_gen");
        return;
    }
    for( int i=0 ; i<n_modules ; i++ )
    {
        // Ensure modules[i] can forget
        if( !(modules[i]->random_gen) )
            modules[i]->random_gen = random_gen;
        modules[i]->forget();
    }
}

//////////////
// finalize //
//////////////
void ModuleStackModule::finalize()
{
    for( int i=0 ; i<n_modules ; i++ )
        modules[i]->finalize();
}

//////////////////////
// bpropDoesNothing //
//////////////////////
bool ModuleStackModule::bpropDoesNothing()
{
    for( int i=0 ; i<n_modules ; i++ )
        if( !(modules[i]->bpropDoesNothing()) )
            return false;
    return true;
}

/////////////////////
// setLearningRate //
/////////////////////
void ModuleStackModule::setLearningRate(real dynamic_learning_rate)
{
    for( int i=0 ; i<n_modules ; i++ )
        modules[i]->setLearningRate( dynamic_learning_rate );
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
