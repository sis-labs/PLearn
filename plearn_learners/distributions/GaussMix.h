// -*- C++ -*-

// GaussMix.h
// 
// Copyright (C) 2004-2005 University of Montreal
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

/*! \file GaussMix.h */
#ifndef GaussMix_INC
#define GaussMix_INC

#include "PDistribution.h"

namespace PLearn {
using namespace std;

class GaussMix: public PDistribution
{

private:

    typedef PDistribution inherited;

    //! Storage vectors to save memory allocations.
    Vec sample_row, log_likelihood_post;
    mutable Vec x_minus_mu_x, mu_target, log_likelihood_dens;

protected:

    // *********************
    // * protected options *
    // *********************

    mutable real conditional_updating_time;
    Mat eigenvalues;
    TVec<Mat> eigenvectors;
    int D;
    mutable Mat diags;
    Vec log_coeff;
    Vec log_p_j_x;
    Vec log_p_x_j_alphaj;
    mutable Mat mu_y_x;
    int n_eigen_computed;
    TVec<int> n_tries;
    int nsamples;
    Vec p_j_x;
    real training_time;

    // Fields below are not options.

    TVec<Mat> cov_x;            //!< The covariance of x.
    TVec<Mat> cov_y_x;          //!< The covariance of y|x.
    mutable Mat eigenvalues_x;  //!< The eigenvalues of the covariance of X.
    mutable Mat eigenvalues_y_x;//!< The eigenvalues of the covariance of Y|x.
    TVec<Mat> eigenvectors_x;   //!< The eigenvectors of the covariance of X.
    TVec<Mat> eigenvectors_y_x; //!< The eigenvectors of the covariance of Y|x.
    TVec<Mat> full_cov;         //!< The full covariance matrix.
    TVec<Mat> y_x_mat;          //!< The product K2 * K1^-1 to compute E[Y|x].

    //! Mean and standard deviation of the training set.
    Vec mean_training, stddev_training;

    //! The posterior probabilities P(j | s_i), where j is the index of a Gaussian
    //! and i is the index of a sample.
    Mat posteriors;

    //! The initial weights of the samples s_i in the training set, copied for
    //! efficiency concerns.
    Vec initial_weights;

    //! A matrix whose j-th line is a Vec with the weights of each sample s_i,
    //! multiplied by the posterior P(j | s_i).
    Mat updated_weights;

    //! Storage for the (weighted) covariance matrix of the dataset.
    //! It is only used with when type == "general".
    Mat covariance;

public:

    // ************************
    // * public build options *
    // ************************

    real alpha_min;
    real epsilon;
    int kmeans_iterations;
    int L;
    int n_eigen;
    real sigma_min;
    string type;

    // Currently learnt options, but may be build options in the future.
    Vec alpha;
    mutable Mat mu;  
    Vec sigma;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    GaussMix();

    // ******************
    // * Object methods *
    // ******************

protected:

    //! Given the posteriors, fill the centers and covariance of each Gaussian.
    virtual void computeMeansAndCovariances();

    //! Compute log p(y | x,j), where j < L is the index of a component of the mixture.
    //! If 'is_input' is set to true, then it will instead compute log p(x | j)
    //! (here x = the method argument y), the probability of input part x given j.
    virtual real computeLogLikelihood(const Vec& y, int j, bool is_input = false) const;

    //! Compute the posteriors P(j | s_i) for each sample point and each Gaussian.
    virtual void computePosteriors();

    //! Compute the weight of each mixture (the coefficient alpha).
    //! If a mixture has a too low coefficient, it will be removed, and the method
    //! will return 'true' (otherwise it will return 'false').
    virtual bool computeWeights();

    //! Generate a sample s from the given Gaussian. If 'given_gaussian' is equal
    //! to -1, then a random Gaussian will be chosen according to the weights alpha.
    virtual void generateFromGaussian(Vec& s, int given_gaussian) const;

    //! Precompute stuff specific to each Gaussian, given its current parameters.
    //! This method is called after each training step.
    virtual void precomputeStuff();

    //! Replace the j-th Gaussian with another one (probably because that one is
    //! not appropriate). The new one is centered on a random point sampled from
    //! the Gaussian with highest weight alpha, and has the same covariance.
    virtual void replaceGaussian(int j);

    //! Resize everything before training.
    void resizeStuffBeforeTraining();

    //! Update the sample weights according to their initial weights and the current
    //! posterior probabilities.
    void updateSampleWeights();

public:

    //! Overridden.
    virtual void generate(Vec& s) const;

private: 

    //! This does the actual building. 
    void build_();

protected: 

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Perform K-means.
    void kmeans(VMat samples, int nclust, TVec<int> & clust_idx, Mat & clust, int maxit=9999);

    //! Overridden so as to compute specific GaussMix outputs.
    virtual void unknownOutput(char def, const Vec& input, Vec& output, int& k) const;

public:

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    virtual void forget();

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(GaussMix);

    // ********************
    // * PLearner methods *
    // ********************

    //! Trains the model.
    virtual void train();

    //! Overridden to take into account new outputs computed.
    virtual int outputsize() const;

    // *************************
    // * PDistribution methods *
    // *************************

    //! Set the value for the input part of the conditional probability.
    virtual void setInput(const Vec& input) const;

    //! This method updates the internal data given a new sorting of the variables
    //! defined by the conditional flags.
    virtual void updateFromConditionalSorting() const;

    //! return density p(y | x)
    virtual real log_density(const Vec& y) const;

    //! return survival fn = P(Y>y | x)
    virtual real survival_fn(const Vec& y) const;

    //! return survival fn = P(Y<y | x)
    virtual real cdf(const Vec& y) const;

    //! Compute E[Y | x] 
    virtual void expectation(Vec& mu) const;

    //! Compute Var[Y | x]
    virtual void variance(Mat& cov) const;

    //! "Get" methods.
    int getNEigenComputed() const;
    Mat getEigenvectors(int j) const;
    Vec getEigenvals(int j) const;
    Vec getLogLikelihoodDens() const { return log_likelihood_dens; }

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussMix);

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
