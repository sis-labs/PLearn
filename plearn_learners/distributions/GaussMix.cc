// -*- C++ -*-

// GaussMix.cc
// 
// Copyright (C) 2003 Julien Keable
// Copyright (C) 2004 Universit� de Montr�al
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
 * $Id: GaussMix.cc,v 1.26 2004/05/20 14:06:52 tihocan Exp $ 
 ******************************************************* */

/*! \file GaussMix.cc */
#include "ConcatColumnsVMatrix.h"
#include "GaussMix.h"
#include "pl_erf.h"   //!< For gauss_density().
#include "plapack.h"
#include "random.h"
#include "SubVMatrix.h"
#include "VMat_maths.h"

namespace PLearn {
using namespace std;

//////////////
// GaussMix //
//////////////
GaussMix::GaussMix() 
: PDistribution(),
  alpha_min(1e-5),
  epsilon(1e-6),
  kmeans_iterations(3),
  L(1),
  n_eigen(-1),
  sigma_min(1e-5),
  type("spherical")
{
  nstages = 10;
}


PLEARN_IMPLEMENT_OBJECT(GaussMix, 
                        "Gaussian Mixture, either set non-parametrically or trained by EM.", 
                        "GaussMix implements a mixture of L gaussians.\n"
                        "There are 4 possible parametrization types:\n"
                        " - spherical : gaussians have covar matrix = diag(sigma). Parameter used : sigma.\n"
                        " - diagonal  : gaussians have covar matrix = diag(sigma_i). Parameters used : diags.\n"
                        " - general   : gaussians have an unconstrained covariance matrix.\n"
                        "               The user specifies the number 'n_eigen' of eigenvectors kept when\n"
                        "               decomposing the covariance matrix. The remaining eigenvectors are\n"
                        "               considered as having a fixed eigenvalue equal to the next highest\n"
                        "               eigenvalue in the decomposition.\n"
                        " - factor    : (not implemented!) as in the general case, the gaussians are defined\n"
                        "               with K<=D vectors (through KxD matrix 'V'), but these need not be\n"
                        "               orthogonal/orthonormal.\n"
                        "               The covariance matrix used will be V(t)V + psi with psi a D-vector\n"
                        "               (through parameter diags).\n"
                        "2 parameters are common to all 4 types :\n"
                        " - alpha : the ponderation factor of the gaussians\n"
                        " - mu    : their centers\n"
);

////////////////////
// declareOptions //
////////////////////
void GaussMix::declareOptions(OptionList& ol)
{

  // Build options.

  declareOption(ol,"L", &GaussMix::L, OptionBase::buildoption,
                "Number of gaussians in the mixture.");
  
  declareOption(ol,"type", &GaussMix::type, OptionBase::buildoption,
                "A string :  'spherical', 'diagonal', 'general', 'factor',\n"
                "This is the type of covariance matrix for each Gaussian.\n"
                "   - spherical : spherical covariance matrix sigma * I\n"
                "   - diagonal  : diagonal covariance matrix\n"
                "   - general   : unconstrained covariance matrix (defined by its eigenvectors)\n"
                "   - factor    : (not implemented) represented by Ks[i] principal components\n");

  declareOption(ol,"n_eigen", &GaussMix::n_eigen, OptionBase::buildoption,
                "If type == 'general', the number of eigenvectors used for the covariance matrix.\n"
                "The remaining eigenvectors will be given an eigenvalue equal to the next highest\n"
                "eigenvalue. If set to -1, all eigenvectors will be kept.");
  
  declareOption(ol,"alpha_min", &GaussMix::alpha_min, OptionBase::buildoption,
                "The minimum weight for each Gaussian.");
  
  declareOption(ol,"sigma_min", &GaussMix::sigma_min, OptionBase::buildoption,
                "The minimum standard deviation allowed.");
  
  declareOption(ol,"epsilon", &GaussMix::epsilon, OptionBase::buildoption,
                "A small number to check for near-zero probabilities.");
  
  declareOption(ol,"kmeans_iterations", &GaussMix::kmeans_iterations, OptionBase::buildoption,
                "The maximum number of iterations performed in the initial K-means algorithm.");
  
  // Learnt options.

  declareOption(ol, "alpha", &GaussMix::alpha, OptionBase::learntoption,
                "Coefficients of each gaussian.\n"
                "They sum to 1 and are positive: they can be interpreted as prior P(gaussian i).\n");

  declareOption(ol, "eigenvalues", &GaussMix::eigenvalues, OptionBase::learntoption,
                "The eigenvalues associated to the principal eigenvectors (element (j,k) is the\n"
                "k-th eigenvalue of the j-th Gaussian.");

  declareOption(ol, "eigenvectors", &GaussMix::eigenvectors, OptionBase::learntoption,
                "The principal eigenvectors of each Gaussian (for the 'general' type). They are\n"
                "stored in rows of the matrices.");

  declareOption(ol,"D", &GaussMix::D, OptionBase::learntoption,
                "Number of dimensions in input space.");
    
  declareOption(ol,"diags", &GaussMix::diags, OptionBase::learntoption,
                "The element (i,j) is the stddev of Gaussian j on the i-th dimension.");
    
  declareOption(ol, "log_coeff", &GaussMix::log_coeff, OptionBase::learntoption,
                "The log of the constant part in the p(x) equation: log(1/sqrt(2*pi^D * Det(C))).");

  declareOption(ol, "mu", &GaussMix::mu, OptionBase::learntoption,
                "These are the centers of each Gaussian, stored in rows.");

  declareOption(ol, "n_eigen_computed", &GaussMix::n_eigen_computed, OptionBase::learntoption,
                "The actual number of principal components computed when type == 'general'.");

  declareOption(ol, "nsamples", &GaussMix::nsamples, OptionBase::learntoption,
                "The number of samples in the training set.");

  declareOption(ol, "posteriors", &GaussMix::posteriors, OptionBase::learntoption,   // TODO Remove from options (too much space on disk).
                "The posterior probabilities P(j | x_i), where j is the index of a Gaussian,\n"
                "and i is the index of a sample.");

  declareOption(ol, "sigma", &GaussMix::sigma, OptionBase::learntoption,
                "The variance in all directions, for type == 'spherical'.\n");

  // TODO What to do with these options.
  
/*  declareOption(ol, "V_idx", &GaussMix::V_idx, OptionBase::buildoption,
                "Used for general and factore gaussians : A vector of size L. V_idx[l] is the row index of the first vector of gaussian 'l' in the matrix 'V' (also used to index vector 'lambda')"); */

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussMix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void GaussMix::build_()
{}

////////////////////////////////
// computeMeansAndCovariances //
////////////////////////////////
void GaussMix::computeMeansAndCovariances() {
  VMat weighted_train_set;
  Vec sum_columns(L);
  columnSum(posteriors, sum_columns);
  for (int j = 0; j < L; j++) {
    // Build the weighted dataset.
    if (sum_columns[j] < epsilon) {
      PLWARNING("In GaussMix::computeMeansAndCovariances - A posterior is almost zero");
    }
    VMat weights(columnmatrix(updated_weights(j)));
    weighted_train_set = new ConcatColumnsVMatrix(
        new SubVMatrix(train_set, 0, 0, nsamples, D), weights);
    weighted_train_set->defineSizes(D, 0, 1);
    if (type == "spherical") {
      Vec variance(D);
      Vec center = mu(j);
      computeInputMeanAndVariance(weighted_train_set, center, variance);
      sigma[j] = sqrt(mean(variance));   // TODO See if harmonic mean is needed ?
#ifdef BOUNDCHECK
      if (isnan(sigma[j])) {
        PLWARNING("In GaussMix::computeMeansAndCovariances - A standard deviation is nan");
      }
#endif
    } else if (type == "diagonal") {
      Vec variance(D);
      Vec center = mu(j);
      computeInputMeanAndVariance(weighted_train_set, center, variance);
      for (int i = 0; i < D; i++) {
        diags(i,j) = sqrt(variance[i]);
      }
    } else if (type == "general") {
      Mat covar(D,D);
      Vec center = mu(j);
      computeInputMeanAndCovar(weighted_train_set, center, covar);
      Vec eigenvals = eigenvalues(j); // The eigenvalues vector of the j-th Gaussian.
      eigenVecOfSymmMat(covar, n_eigen_computed, eigenvals, eigenvectors[j]);
    } else {
      PLERROR("In GaussMix::computeMeansAndCovariances - Not implemented for this type of Gaussian");
    }
    // TODO Implement them all.
  }
}

/////////////////////
// computeLikehood //
/////////////////////
real GaussMix::computeLikehood(Vec& x, int j) {
  if (type == "spherical") {
    real p = 1.0;
    if (sigma[j] < sigma_min) {
      return 0;
    }
    for (int k = 0; k < D; k++) {
      p *= gauss_density(x[k], mu(j, k), sigma[j]);
#ifdef BOUNDCHECK
      if (isnan(p)) {
        PLWARNING("In GaussMix::computeLikehood - Density is nan");
      }
#endif
    }
    return p;
  } else if (type == "diagonal") {
    real p = 1.0;
    real sig;
    for (int k = 0; k < D; k++) {
      sig = diags(k,j);
      if (sig < sigma_min) {
        return 0;
      } else {
        p *= gauss_density(x[k], mu(j, k), sig);
      }
    }
    return p;
  } else if (type == "general") {
    // We store log(p(x | j)) in the variable t.
    static real t;
    static Vec x_centered; // Storing x - mu(j).
    static real squared_norm_x_centered;
    static real one_over_lambda0;
    t = log_coeff[j];
    x_centered.resize(D);
    x_centered << x;
    x_centered -= mu(j);
    squared_norm_x_centered = pownorm(x_centered);
    one_over_lambda0 = 1.0 / eigenvalues(j, n_eigen_computed - 1);
    // t -= 0.5  * 1/lambda_0 * ||x - mu||^2
    t -= 0.5 * one_over_lambda0 * squared_norm_x_centered;
    for (int k = 0; k < n_eigen_computed - 1; k++) {
      // t -= 0.5 * (1/lambda_k - 1/lambda_0) * ((x - mu)'.v_k)^2
      t -= 0.5 * (1 / eigenvalues(j, k) - one_over_lambda0) * square(dot(eigenvectors[j](k), x_centered));
    }
    return exp(t);
  } else {
    PLERROR("In GaussMix::computeLikehood - Not implemented for this type of Gaussian");
  }
  return 0;
}

///////////////////////
// computePosteriors //
///////////////////////
void GaussMix::computePosteriors() {
  static Vec sample;
  static Vec likehood;
  sample.resize(D);
  likehood.resize(L);
  real sum_likehood;
  for (int i = 0; i < nsamples; i++) {
    train_set->getSubRow(i, 0, sample);
    sum_likehood = 0;
    // First we need to compute the likehood P(x_i | j).
    for (int j = 0; j < L; j++) {
      likehood[j] = computeLikehood(sample, j) * alpha[j];
#ifdef BOUNDCHECK
      if (isnan(likehood[j])) {
        PLWARNING("In GaussMix::computePosteriors - computeLikehood returned nan");
      }
#endif
      sum_likehood += likehood[j];
    }
#ifdef BOUNDCHECK
    if (sum_likehood < epsilon) {
      // x_i is far from each Gaussian, and thus sum_likehood is null
      // because of numerical approximations. We find the closest
      // Gaussian, and say P(j | x_i) = delta_{j is the closest Gaussian}
      PLWARNING("In GaussMix::computePosteriors - A point has near zero density");
    }
#endif
    for (int j = 0; j < L; j++) {
      // Compute the posterior P(j | x_i) = P(x_i | j) * alpha_i / (sum_i ")
      posteriors(i, j) = likehood[j] / sum_likehood;
    }
  }
  // Now update the sample weights.
  updateSampleWeights();
}

////////////////////
// computeWeights //
////////////////////
bool GaussMix::computeWeights() {
  bool replaced_gaussian = false;
  alpha.fill(0);
  for (int i = 0; i < nsamples; i++) {
    for (int j = 0; j < L; j++) {
      alpha[j] += posteriors(i,j);
    }
  }
  alpha /= real(nsamples);
  for (int j = 0; j < L && !replaced_gaussian; j++) {
    if (alpha[j] < alpha_min) {
      // alpha[j] is too small! We need to remove this Gaussian from the
      // mixture, and find a new (better) one.
      replaceGaussian(j);
      replaced_gaussian = true;
    }
  }
  return replaced_gaussian;
}

////////////
// forget //
////////////
void GaussMix::forget()
{
  stage = 0;
  // Free memory.
  mu = Mat();
  posteriors = Mat();
  alpha = Vec();
  sigma = Vec();
  diags = Mat();
  eigenvectors = TVec<Mat>();
  eigenvalues = Mat();
  log_coeff = Vec();
}

//////////////
// generate //
//////////////
void GaussMix::generate(Vec& x) const
{
  generateFromGaussian(x, -1);
}

//////////////////////////
// generateFromGaussian //
//////////////////////////
void GaussMix::generateFromGaussian(Vec& x, int given_gaussian) const {
  int j;    // The index of the Gaussian to use.
  if (given_gaussian < 0)
    j = multinomial_sample(alpha);
  else
    j = given_gaussian % alpha.length();
  x.resize(D);
  if (type == "spherical") {
    for (int k = 0; k < D; k++) {
      x[k] = gaussian_mu_sigma(mu(j, k), sigma[j]);
    }
  } else if (type == "diagonal") {
    for (int k = 0; k < D; k++) {
      x[k] = gaussian_mu_sigma(mu(j, k), diags(k,j));
    }
  } else if (type == "general") {
    static Vec norm;
    static real lambda0;
    norm.resize(n_eigen_computed - 1);
    fill_random_normal(norm);
    lambda0 = eigenvalues(j, n_eigen_computed - 1);
    x.fill(0);
    for (int k = 0; k < n_eigen_computed - 1; k++) {
      x += sqrt(eigenvalues(j,k) - lambda0) * norm[k] * eigenvectors[j](k);
    }
    norm.resize(D);
    fill_random_normal(norm);
    x += norm * sqrt(lambda0);
    x += mu(j);
  } else if(type[0]=='F') {
//    generateFactor(x,given_gaussian);
  } else {
    PLERROR("In GaussMix::generate - Unknown mixture type");
  }
}

///////////////////////
// getNEigenComputed //
///////////////////////
int GaussMix::getNEigenComputed() const {
  return n_eigen_computed;
}

/////////////////////
// getEigenvectors //
/////////////////////
Mat GaussMix::getEigenvectors(int j) const {
  return eigenvectors[j];
}

//////////////////
// getEigenvals //
//////////////////
Vec GaussMix::getEigenvals(int j) const {
  return eigenvalues(j);
}

////////////
// kmeans //
////////////
void GaussMix::kmeans(VMat samples, int nclust, TVec<int> & clust_idx, Mat & clust, int maxit)
// TODO Put it into the PLearner framework.
{
  int nsamples = samples.length();
  Mat newclust(nclust,samples->inputsize());
  clust.resize(nclust,samples->inputsize());
  clust_idx.resize(nsamples);

  Vec input(samples->inputsize());
  Vec target(samples->targetsize());
  real weight;
    
  Vec samples_per_cluster(nclust);
  TVec<int> old_clust_idx(nsamples);
  bool ok=false;

  // build a nclust-long vector of samples indexes to initialize clusters centers
  Vec start_idx(nclust,-1.0);
  int val;
  for(int i=0;i<nclust;i++)
  {
    bool ok=false;
    while(!ok)
    {
      
      ok=true;
      val = rand() % nsamples;
      for(int j=0;j<nclust && start_idx[j]!=-1.0;j++)
        if(start_idx[j]==val)
        {
          ok=false;
          break;
        }
    }
    start_idx[i]=val;
    samples->getExample(val,input,target,weight);    
    clust(i)<<input;
  }

  while(!ok && maxit--)
  {
    newclust.clear();
    samples_per_cluster.clear();
    old_clust_idx<<clust_idx;
    for(int i=0;i<nsamples;i++)
    {
      samples->getExample(i,input,target,weight);
      real dist,bestdist=1E300;
      int bestclust=0;
      for(int j=0;j<nclust;j++)
        if((dist=pownorm(clust(j)-input)) < bestdist)
        {
          bestdist=dist;
          bestclust=j;
        }
      clust_idx[i]=bestclust;
      samples_per_cluster[bestclust] += weight;
      newclust(bestclust) += input * weight;
    }
    for(int i=0; i<nclust; i++)
      if (samples_per_cluster[i]>0)
        newclust(i) /= samples_per_cluster[i];
    clust << newclust;
    ok=true;
    for(int i=0;i<nsamples;i++)
      if(old_clust_idx[i]!=clust_idx[i])
      {
        ok=false;
        break;
      }
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  PLERROR("GaussMix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////////
// precomputeStuff //
/////////////////////
void GaussMix::precomputeStuff() {
  if (type == "spherical") {
    // Nothing to do.
  } else if (type == "diagonal") {
    // Nothing to do.
  } else if (type == "general") {
    // Precompute the log_coeff.
    for (int j = 0; j < L; j++) {
      real log_det = 0;
      for (int k = 0; k < n_eigen_computed; k++) {
#ifdef BOUNDCHECK
        if (eigenvalues(j,k) < epsilon) {
          PLWARNING("In GaussMix::precomputeStuff - An eigenvalue is near zero");
        }
#endif
        log_det += log(eigenvalues(j,k));
      }
      if(D - n_eigen_computed > 0) {
        log_det += log(eigenvalues(j, n_eigen_computed - 1)) * (D - n_eigen_computed);
      }
      log_coeff[j] = - 0.5 * (D * log(2*3.141549) + log_det );
    }
  } else {
    PLERROR("In GaussMix::precomputeStuff - Not implemented for this type");
  }
}

/////////////////////
// replaceGaussian //
/////////////////////
void GaussMix::replaceGaussian(int j) {
  // Find the Gaussian with highest weight.
  int high = 0;
  for (int k = 1; k < L; k++) {
    if (alpha[k] > alpha[high]) {
      high = k;
    }
  }
  // Generate the new center from this Gaussian.
  Vec new_center = mu(j);
  generateFromGaussian(new_center, high);
  // Copy the covariance.
  if (type == "spherical") {
    sigma[j] = sigma[high];
  } else if (type == "diagonal") {
    diags.column(j) << diags.column(high);
  } else if (type == "general") {
    eigenvalues(j) << eigenvalues(high);
    eigenvectors[j] << eigenvectors[high];
    log_coeff[j] = log_coeff[high];
  } else {
    PLERROR("In GaussMix::replaceGaussian - Not implemented for this type");
  }
  // Arbitrarily takes half of the weight of this Gaussian.
  alpha[high] /= 2.0;
  alpha[j] = alpha[high];
}

////////////////////
// resetGenerator //
////////////////////
void GaussMix::resetGenerator(long g_seed) const
{ 
  manual_seed(g_seed);  
}

/////////////////
// resizeStuff //
/////////////////
void GaussMix::resizeStuff() {
  nsamples = train_set->length();
  alpha.resize(L);
  D = train_set->inputsize();
  initial_weights.resize(nsamples);
  mu.resize(L,D);
  posteriors.resize(nsamples, L);
  updated_weights.resize(L, nsamples);
  // Those are not used for every type:
  sigma.resize(0);
  diags.resize(0,0);
  eigenvectors.resize(0);
  eigenvalues.resize(0,0);
  log_coeff.resize(0);
  if (type == "diagonal") {
    diags.resize(D,L);
  } else if (type == "factor") {
  } else if (type == "general") {
    eigenvectors.resize(L);
    log_coeff.resize(L);
    if (n_eigen == -1 || n_eigen == D) {
      // We need to compute all eigenvectors.
      n_eigen_computed = D;
    } else {
      n_eigen_computed = n_eigen + 1;
    }
    for (int i = 0; i < L; i++) {
      eigenvectors[i].resize(n_eigen_computed, D);
    }
    eigenvalues.resize(L, n_eigen_computed);
  } else if (type == "spherical") {
    sigma.resize(L);
  } else {
    PLERROR("In GaussMix::train - Unknown value for the 'type' option");
  }
}

///////////
// train //
///////////
void GaussMix::train()
{
  // TODO Actually, we should compute the posteriors because saving them
  // could take too much room.
  // Check we actually need to train.
  if (stage >= nstages) {
    PLWARNING("In GaussMix::train - The learner is already trained");
    return;
  }

  // Check there is a training set.
  if (!train_set) {
    PLERROR("In GaussMix::train - No training set specified");
  }

  // Make sure everything is the right size.
  resizeStuff();

  if (stage == 0) {
    // Copy the sample weights.
    if (train_set->weightsize() <= 0) {
      initial_weights.fill(1);
    } else {
      Vec tmp1;
      Vec tmp2;
      real w;
      for (int i = 0; i < nsamples; i++) {
        train_set->getExample(i, tmp1, tmp2, w);
        initial_weights[i] = w;
      }
    }
    // Perform K-means to initialize the centers of the mixture.
    TVec<int> clust_idx;  // Store the cluster index for each sample.
    kmeans(train_set, L, clust_idx, mu, kmeans_iterations);
    posteriors.fill(0);
    for (int i = 0; i < nsamples; i++) {
      // Initially, P(j | x_i) = 0 if x_i is not in the j-th cluster,
      // and 1 otherwise.
      posteriors(i, clust_idx[i]) = 1;
    }
    updateSampleWeights();
    computeWeights();
    computeMeansAndCovariances();
    precomputeStuff();
  }

  Vec sample(D);
  bool replaced_gaussian = false;
  ProgressBar* pb = 0;
  int n_steps = nstages - stage;
  if (report_progress) {
    pb = new ProgressBar("Training GaussMix", n_steps);
  }
  while (stage < nstages) {
    do {
      computePosteriors();
      replaced_gaussian = computeWeights();
    } while (replaced_gaussian);
    computeMeansAndCovariances();
    precomputeStuff();
    stage++;
    if (pb) {
      pb->update(n_steps - nstages + stage);
    }
  }
  if (pb)
    delete pb;
}

/////////////////////////
// updateSampleWeights //
/////////////////////////
void GaussMix::updateSampleWeights() {
  for (int j = 0; j < L; j++) {
    updated_weights(j) << initial_weights;
    columnmatrix(updated_weights(j)) *= posteriors.column(j);
  }
}

double GaussMix::log_density(const Vec& x) const
{ 
  PLERROR("In GaussMix::log_density - Not implemented");
  return 0;
}

double GaussMix::survival_fn(const Vec& x) const
{ 
   PLERROR("survival_fn not implemented for GaussMix"); return 0.0; 
}

double GaussMix::cdf(const Vec& x) const
{ 
  PLERROR("cdf not implemented for GaussMix"); return 0.0; 
}

Vec GaussMix::expectation() const
{ 
  PLERROR("expectation not implemented for GaussMix"); return Vec(); 
}

Mat GaussMix::variance() const
{ 
  PLERROR("variance not implemented for GaussMix"); return Mat(); 
}

} // end of namespace PLearn

