// -*- C++ -*-

// WPLS.cc
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
 * $Id: WPLS.cc 5370 2006-04-12 15:27:55Z tihocan $ 
 ******************************************************* */

// Authors: Olivier Delalleau, Charles Dugas

/*! \file WPLS.cc */

#include "WPLS.h"
#include <plearn/math/plapack.h>
#include <plearn/vmat/ShiftAndRescaleVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/math/TMat_maths.h>    //!< For dist.
#include <plearn/math/pl_math.h>       // !< for isnan
#include <plearn/vmat/VMat_linalg.h>
#include <plearn/io/load_and_save.h>
#include <plearn_learners/generic/VPLCombinedLearner.h>
#include <plearn_learners/generic/VPLPreprocessedLearner2.h>

namespace PLearn {
using namespace std;

WPLS::WPLS() 
    : m(-1),
      p(-1),
      w(0),
      method("kernel"),
      precision(1e-6),
      output_the_score(0),
      output_the_target(1),
      parent_filename("tmp"),
      parent_sub(0)
{}

PLEARN_IMPLEMENT_OBJECT(WPLS,
                        "Partial Least Squares Regression (WPLSR).",
                        "You can use this learner to perform regression, and / or dimensionality\n"
                        "reduction.\n"
                        "WPLS regression assumes the target Y and the data X are linked through:\n"
                        " Y = T.Q' + E\n"
                        " X = T.P' + F\n"
                        "The underlying coefficients T (the 'scores') and the loading matrices\n"
                        "Q and P are seeked. It is then possible to compute the prediction y for\n"
                        "a new input x, as well as its score vector t (its representation in\n"
                        "lower-dimensional coordinates).\n"
                        "The available algorithms to perform WPLS (chosen by the 'method' option) are:\n"
                        "\n"
                        " ====  WPLS1  ====\n"
                        "The classical WPLS algorithm, suitable only for a 1-dimensional target. The\n"
                        "following algorithm is taken from 'Factor Analysis in Chemistry', with an\n"
                        "additional loop that (I believe) was missing:\n"
                        " (1) Let X (n x p) = the centered and normalized input data\n"
                        "     Let y (n x 1) = the centered and normalized target data\n"
                        "     Let k be the number of components extracted\n"
                        " (2) s = y\n"
                        " (3) lx' = s' X, s = X lx (normalized)\n"
                        " (4) If s has changed by more than 'precision', loop to (3)\n"
                        " (5) ly = s' y\n"
                        " (6) lx' = s' X\n"
                        " (7) Store s, lx and ly in the columns of respectively T, P and Q\n"
                        " (8) X = X - s lx', y = y - s ly, loop to (2) k times\n"
                        " (9) Set W = (T P')^(+) T, where the ^(+) is the right pseudoinverse\n"
                        "\n"
                        " ==== Kernel ====\n"
                        "The code implements a NIPALS-WPLS-like algorithm, which is a so-called\n"
                        "'kernel' algorithm (faster than more classical implementations).\n"
                        "The algorithm, inspired from 'Factor Analysis in Chemistry' and above all\n"
                        "www.statsoftinc.com/textbook/stwpls.html, is the following:\n"
                        " (1) Let X (n x p) = the centered and normalized input data\n"
                        "     Let Y (n x m) = the centered and normalized target data\n"
                        "     Let k be the number of components extracted\n"
                        " (2) Initialize A_0 = X'Y, M_0 = X'X, C_0 = Identity(p), and h = 0\n"
                        " (3) q_h = largest eigenvector of B_h = A_h' A_h, found by the NIPALS method:\n"
                        "       (3.a) q_h = a (normalized) randomn column of B_h\n"
                        "       (3.b) q_h = B_h q_h\n"
                        "       (3.c) normalize q_h\n"
                        "       (3.d) if q_h has changed by more than 'precision', go to (b)\n"
                        " (4) w_h = C_h A_h q_h, normalize w_h and store it in a column of W (p x k)\n"
                        " (5) p_h = M_h w_h, c_h = w_h' p_h, p_h = p_h / c_h and store it in a column\n"
                        "     of P (p x k)\n"
                        " (6) q_h = A_h' w_h / c_h, and store it in a column of Q (m x k)\n"
                        " (7) A_h+1 = A_h - c_h p_h q_h'\n"
                        "     M_h+1 = M_h - c_h p_h p_h',\n"
                        "     C_h+1 = C_h - w_h p_h\n"
                        " (8) h = h+1, and if h < k, go to (3)\n"
                        "\n"
                        "The result is then given by:\n"
                        " - Y = X B, with B (p x m) = W Q'\n"
                        " - T = X W, where T is the score (reduced coordinates)\n"
                        "\n"
                        "You can choose to have the score (T) and / or the target (Y) in the output\n"
                        "of the learner (default is target only, i.e. regression)."
    );

////////////////////
// declareOptions //
////////////////////
void WPLS::declareOptions(OptionList& ol)
{
    // Build options.

    declareOption(ol, "method", &WPLS::method, OptionBase::buildoption,
                  "The WPLS algorithm used ('wpls1' or 'kernel', see help for more details).\n");

    declareOption(ol, "output_the_score", &WPLS::output_the_score, OptionBase::buildoption,
                  "If set to 1, then the score (the low-dimensional representation of the input)\n"
                  "will be included in the output (before the target).");

    declareOption(ol, "output_the_target", &WPLS::output_the_target, OptionBase::buildoption,
                  "If set to 1, then (the prediction of) the target will be included in the\n"
                  "output (after the score).");

    declareOption(ol, "parent_filename", &WPLS::parent_filename, OptionBase::buildoption,
                  "For hyper-parameter selection purposes: use incremental learning to speed-up process");
    
    declareOption(ol, "parent_sub", &WPLS::parent_sub, OptionBase::buildoption,
                  "Tells which of the sublearners (of the combined learner) should be used.");
    
    declareOption(ol, "precision", &WPLS::precision, OptionBase::buildoption,
                  "The precision to which we compute the eigenvectors.");


    // Learnt options.

    declareOption(ol, "B", &WPLS::B, OptionBase::learntoption,
                  "The regression matrix in Y = X.B + E.");

    declareOption(ol, "m", &WPLS::m, OptionBase::learntoption,
                  "Used to store the target size.");

    declareOption(ol, "mean_input", &WPLS::mean_input, OptionBase::learntoption,
                  "The mean of the input data X.");

    declareOption(ol, "mean_target", &WPLS::mean_target, OptionBase::learntoption,
                  "The mean of the target data Y.");

    declareOption(ol, "p", &WPLS::p, OptionBase::learntoption,
                  "Used to store the input size.");

    declareOption(ol, "P", &WPLS::P, OptionBase::learntoption,
                  "Matrix that maps features to observed inputs: X = T.P' + E.");

    declareOption(ol, "Q", &WPLS::Q, OptionBase::learntoption,
                  "Matrix that maps features to observed outputs: Y = T.P' + F.");

    declareOption(ol, "stddev_input", &WPLS::stddev_input, OptionBase::learntoption,
                  "The standard deviation of the input data X.");

    declareOption(ol, "stddev_target", &WPLS::stddev_target, OptionBase::learntoption,
                  "The standard deviation of the target data Y.");

    declareOption(ol, "w", &WPLS::p, OptionBase::learntoption,
                  "Used to store the weight size (0 or 1).");
    
    declareOption(ol, "W", &WPLS::W, OptionBase::learntoption,
                  "The regression matrix in T = X.W.");
    
    

    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void WPLS::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void WPLS::build_()
{
    PLASSERT(precision > 0);

    if (train_set) {
        this->m = train_set->targetsize();
        this->p = train_set->inputsize();
        this->w = train_set->weightsize();
        // Check method consistency.
        if (method == "wpls1") {
            // Make sure the target is 1-dimensional.
            if (m != 1) {
                PLERROR("In WPLS::build_ - With the 'wpls1' method, target should be 1-dimensional");
            }
        } else if (method == "kernel") {
            PLERROR("In WPLS::build_ - option 'method=kernel' not implemented yet");
        } else {
            PLERROR("In WPLS::build_ - Unknown value for option 'method'");
        }
    }
    if (!output_the_score && !output_the_target) {
        // Weird, we don't want any output ??
        PLWARNING("In WPLS::build_ - There will be no output");
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void WPLS::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                  const Vec& target, Vec& costs) const
{
    costs.resize(1);
    costs[0] = powdistance(output,target,2.0);
// No cost computed.
}

///////////////////
// computeOutput //
///////////////////
void WPLS::computeOutput(const Vec& input, Vec& output) const
{
    static Vec input_copy;
    if (W.width()==0)
        PLERROR("WPLS::computeOutput but model was not trained!");
    // Compute the output from the input
    int nout = outputsize();
    output.resize(nout);
    // First normalize the input.
    input_copy.resize(this->p);
    input_copy << input;
    input_copy -= mean_input;
    input_copy /= stddev_input;
    int target_start = 0;
    if (output_the_score) {
        transposeProduct(output.subVec(0, this->nstages), W, input_copy);
        target_start = this->nstages;
    }
    if (output_the_target) {
        if (this->m > 0) {
            Vec target = output.subVec(target_start, this->m);
            transposeProduct(target, B, input_copy);
            target *= stddev_target;
            target += mean_target;
        } else {
            // This is just a safety check, since it should never happen.
            PLWARNING("In WPLS::computeOutput - You ask to output the target but the target size is <= 0");
        }
    }
}

////////////
// forget //
////////////
void WPLS::forget()
{
    stage= 0;
    // Free memory.
    B = Mat();
    W = Mat();
    P = Mat();
    Q = Mat();
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> WPLS::getTestCostNames() const
{
    // No cost computed.
    TVec<string> t;
    t.append("mse");
    return t;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> WPLS::getTrainCostNames() const
{
    // No cost computed.
    TVec<string> t;
    return t;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void WPLS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    deepCopyField(B, copies);
    deepCopyField(mean_input, copies);
    deepCopyField(mean_target, copies);
    deepCopyField(stddev_input, copies);
    deepCopyField(stddev_target, copies);
    deepCopyField(W, copies);
    deepCopyField(P, copies);
    deepCopyField(Q, copies);
}

///////////////////////
// NIPALSEigenvector //
///////////////////////
void WPLS::NIPALSEigenvector(const Mat& m, Vec& v, real precision) {
    int n = v.length();
    Vec wtmp(n);
    v << m.column(0);
    normalize(v, 2.0);
    bool ok = false;
    while (!ok) {
        wtmp << v;
        product(v, m, wtmp);
        normalize(v, 2.0);
        ok = true;
        for (int i = 0; i < n && ok; i++) {
            if (fabs(v[i] - wtmp[i]) > precision) {
                ok = false;
            }
        }
    }
}

////////////////
// outputsize //
////////////////
int WPLS::outputsize() const
{
    int os = 0;
    if (output_the_score) {
        os += this->nstages;
    }
    if (output_the_target && m >= 0) {
        // If m < 0, this means we don't know yet the target size, thus we
        // shouldn't report it here.
        os += this->m;
    }
    return os;
}

// Unbiased estimators of mean and variance are (all sums taken over index i)
// xbar = [ sum(wi*xi) ] / [ sum(wi) ]
// var  = [ sum(wi*xi*xi) - xbar * xbar * sum(wi) ] / [ sum(wi)  - sum(wi*wi) / sum(wi) ] 
void computeWeightedInputOutputMeansAndStddev(const VMat& d, Vec& means, Vec& stddev)
{
    PLASSERT( d->inputsize() >= 0 );
    int n = d->length();
    int p = d->inputsize();
    int m = d->targetsize();
    means.resize(p+m);
    stddev.resize(p+m);
    Vec input(p+m), target(p+m);
    real weight;
    real sum_wi = 0.0;
    real sum_wi2 = 0.0;
    Vec sum_wixi(p+m), sum_wixi2(p+m);
    sum_wixi.fill(0.0);
    sum_wixi2.fill(0.0);
    for (int i = 0; i < n; i++) {
        d->getExample(i, input, target, weight);
        sum_wi += weight;
        sum_wi2 += weight * weight;
        for (int j = 0; j<p; j++) {  
            sum_wixi[j]  += weight*input[j];
            sum_wixi2[j] += weight*input[j]*input[j];
        }
        for (int j = 0; j<m; j++) {  
            sum_wixi[p+j]  += weight*target[j];
            sum_wixi2[p+j] += weight*target[j]*target[j];
        }
    }
  
    real adjust = sqrt( sum_wi - sum_wi2 /sum_wi );
    real xbar;
    real var;
    for (int j = 0; j<p+m; j++) 
    {
        xbar      = sum_wixi[j]/sum_wi;
        means[j]  = xbar; 
        //make sure we do sqrt of a number >= 0
        var= sum_wixi2[j] - xbar * xbar * sum_wi;
        if(var < 0.)
        {
            PLWARNING("In WPLS::computeWeightedInputOutputMeansAndStddev: var < 0; setting it to 0.");
            var= 0.;
        }
        stddev[j] = sqrt(var) / adjust;
        if (stddev[j] < 1e-10)
            stddev[j] = 1.0;
    }
}

void multiplyColumns(Mat& m, Vec& v)
{
    int n = m.length();
    int p = m.width();
    real vi;
    if(v.length() != n)
        PLERROR("Matrix and vector lengths do not match");
    for(int i=0; i<n; i++) {
        vi = v[i];
        for(int j=0; j<p; j++)
            m(i,j) *= vi;
    }
}

///////////
// train //
///////////
void WPLS::train()
{
    if (stage == nstages) {
        // Already trained.
        if (verbosity >= 1)
            pout << "Skipping WPLS training" << endl;
        return;
    }
    if (verbosity >= 1)
        pout << "WPLS training started" << endl;

    int n    = train_set->length();
    int wlen = train_set->weightsize();
    VMat d = new SubVMatrix(train_set,0,0,train_set->length(), train_set->width());
    d->defineSizes(train_set->inputsize(), train_set->targetsize(), train_set->weightsize(), 0);
    Vec means, stddev;
    computeWeightedInputOutputMeansAndStddev(d, means, stddev);
    if (verbosity >= 2) {
        pout << "means = " << means << endl;
        pout << "stddev = " << stddev << endl;
    }
    normalize(d, means, stddev);
    mean_input  = means.subVec(0, p);
    mean_target = means.subVec(p, m);
    stddev_input  = stddev.subVec(0, p);
    stddev_target = stddev.subVec(p, m);

    Vec shift_input(p), scale_input(p), shift_target(m), scale_target(m);
    shift_input << mean_input;
    scale_input << stddev_input;
    shift_target << mean_target;
    scale_target << stddev_target;
    negateElements(shift_input);
    invertElements(scale_input);
    negateElements(shift_target);
    invertElements(scale_target);

    VMat input_part = new SubVMatrix(train_set,
                                     0, 0,
                                     train_set->length(),
                                     train_set->inputsize());
    PP<ShiftAndRescaleVMatrix> X_vmat =
        new ShiftAndRescaleVMatrix(input_part, shift_input, scale_input, true);
    X_vmat->verbosity = this->verbosity;
    VMat X_vmatrix = static_cast<ShiftAndRescaleVMatrix*>(X_vmat);
    Mat X = X_vmatrix->toMat();
    
    VMat target_part = new SubVMatrix( train_set,
                                       0, train_set->inputsize(),
                                       train_set->length(),
                                       train_set->targetsize());
    PP<ShiftAndRescaleVMatrix> Y_vmat =
        new ShiftAndRescaleVMatrix(target_part, shift_target, scale_target, true);
    Y_vmat->verbosity = this->verbosity;
    VMat Y_vmatrix = static_cast<ShiftAndRescaleVMatrix*>(Y_vmat);
    Vec Y(n);
    Y << Y_vmatrix->toMat();
        
    VMat weight_part = new SubVMatrix( train_set,
                                       0, train_set->inputsize() + train_set->targetsize(),
                                       train_set->length(),
                                       train_set->weightsize());
    PP<ShiftAndRescaleVMatrix> WE_vmat;
    VMat WE_vmatrix;
    Vec WE(n);
    Vec sqrtWE(n);
    if (wlen > 0) {
        WE_vmat = new ShiftAndRescaleVMatrix(weight_part);
        WE_vmat->verbosity = this->verbosity;
        WE_vmatrix = static_cast<ShiftAndRescaleVMatrix*>(WE_vmat);
        WE << WE_vmatrix->toMat();
        sqrtWE << sqrt(WE);
        multiplyColumns(X,sqrtWE);
        Y *= sqrtWE;
    } else
        WE.fill(1.0);
    
    // Some common initialization.
    W.resize(p, nstages);
    P.resize(p, nstages);
    Q.resize(m, nstages);
    
    if (method == "kernel") {
        PLERROR("You shouldn't be here... !?");       
    } else if (method == "wpls1") {
        Vec s(n);
        Vec old_s(n);
        Vec lx(p);
        Vec ly(1);
        Mat T(n,nstages);
        Mat tmp_np(n,p), tmp_pp(p,p);

        PP<ProgressBar> pb;
        if(report_progress) {
            pb = new ProgressBar("Computing the components", nstages);
        }
        bool finished;
        real dold;

        if (parent_filename != "") {
            string expdir = getExperimentDirectory();
            int pos = expdir.find("Split");
            string the_split = expdir.substr(pos,6);
            int pos2 = parent_filename.find("Split");
            int nremain = parent_filename.length() - 6 - pos2;
            parent_filename = parent_filename.substr(0,pos2) + the_split + parent_filename.substr(pos2+6,nremain);
            PP<VPLCombinedLearner> combined_parent;
            PLearn::load(parent_filename, combined_parent);
            //if (VPLPreprocessedLearner2* vplpl = dynamic_cast<VPLPreprocessedLearner2*>(
            //        combined_parent->sublearners_[parent_sub]))
            //{
            //    parent = vplpl->learner_;
            //}
            //else
            //    PLERROR("Unsupported type for sublearners of the combined
            //    learner");
            
            PP<VPLPreprocessedLearner2> vplpl = (PP<VPLPreprocessedLearner2>)(combined_parent->sublearners_[parent_sub]);
            PP<WPLS> parent = (PP<WPLS>)(vplpl->learner_);
            //VPLPreprocessedLearner2* vplpl = (VPLPreprocessedLearner2*)(combined_parent->sublearners_[parent_sub]);
            //WPLS* parent (WPLS*)(vplpl->learner_);
            int k = parent->nstages;
            Mat tmp_nk(n,k);
            if (parent->stage < nstages) {
                Mat tmp_n1(n,1);
                product(tmp_n1, X, parent->B);
                for (int i=0; i<n; i++)
                    Y[i] -= tmp_n1(i,0);
                product(tmp_nk,X,parent->W);
                productTranspose(tmp_np,tmp_nk,parent->P);
                X -= tmp_np;
                stage = k;
            } else {
                product(tmp_nk,X,parent->W);
                T = tmp_nk.subMat(0,0,n,nstages);
                P = (parent->P).subMat(0,0,p,nstages);
                Q = (parent->Q).subMat(0,0,m,nstages);
                stage = nstages;
            }
        }
        while (stage < nstages) {
            if (verbosity >= 1)
                pout << "stage=" << stage << endl;
            s << Y;
            normalize(s, 2.0);  
            finished = false;
            int count = 0;
            while (!finished) {
                count++;
                old_s << s;
                transposeProduct(lx, X, s);
                product(s, X, lx);
                normalize(s, 2.0);
                dold = norm(old_s -s);
                if(isnan(dold))
                    PLERROR("dold is nan");
                if (dold < precision)
                    finished = true;
                else {
                    if (verbosity >= 2)
                        pout << "dold = " << dold << endl;
                    if (count%100==0 && verbosity>=1)
                        pout << "loop counts = " << count << endl;
                }
            }
            transposeProduct(lx, X, s);
            ly[0] = dot(s, Y);
            T.column(stage) << s;
            P.column(stage) << lx;
            Q.column(stage) << ly;
            externalProduct(tmp_np,s,lx);
            X -= tmp_np;
            Y -= ly[0] * s;
            if (report_progress)
                pb->update(stage);
            stage++;
        }
        productTranspose(tmp_np, T, P);
        
        if (verbosity >= 2) {
            pout << "T = " << endl << T << endl;
            pout << "P = " << endl << P << endl;
            pout << "Q = " << endl << Q << endl;
            pout << "tmp_np = " << endl << tmp_np << endl;
            pout << endl;
        }
        Mat U, Vt;
        Vec D;
        real safeguard = 1.1;
        SVD(tmp_np, U, D, Vt, 'S', safeguard);
        if (verbosity >= 2) {
            pout << "U = " << endl << U << endl;  
            pout << "D = " << endl << D << endl;
            pout << "Vt = " << endl << Vt << endl;
            pout << endl;
        }
        
        Mat invDmat(p,p);
        invDmat.fill(0.0);
        for (int i = 0; i < D.length(); i++) {
            if (abs(D[i]) < precision)
                invDmat(i,i) = 0.0;
            else
                invDmat(i,i) = 1.0 / D[i];
        }
        
        product(tmp_pp,invDmat,Vt);
        product(tmp_np,U,tmp_pp);
        transposeProduct(W, tmp_np, T);
        B.resize(p,1);
        productTranspose(B, W, Q);
        if (verbosity >= 2) {
            pout << "W = " << W << endl;
            pout << "B = " << B << endl;
        }
        if (verbosity >= 1)
            pout << "WPLS training ended" << endl;
    }
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
