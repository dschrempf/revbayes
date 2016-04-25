#include "PhyloCTMCSiteHomogeneousRestriction.h"

RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::PhyloCTMCSiteHomogeneousRestriction(const TypedDagNode<Tree> *t, bool c, size_t nSites, bool amb, RestrictionAscertainmentBias::Coding ty) :
    PhyloCTMCSiteHomogeneousConditional<RestrictionState>(  t, 2, c, nSites, amb, AscertainmentBias::Coding(ty))
{
}

RevBayesCore::PhyloCTMCSiteHomogeneousRestriction* RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::clone( void ) const {

    return new PhyloCTMCSiteHomogeneousRestriction( *this );
}

bool RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::isSitePatternCompatible( std::map<size_t, size_t> charCounts )
{
    std::map<size_t, size_t>::iterator zero = charCounts.find(0);
    std::map<size_t, size_t>::iterator one  = charCounts.find(1);

    bool compatible = true;
    
    if( charCounts.size() == 1 )
    {
        if(zero != charCounts.end() && (coding & RestrictionAscertainmentBias::NOABSENCESITES) )
        {
            compatible = false;
        }
        else if(one != charCounts.end()  && (coding & RestrictionAscertainmentBias::NOPRESENCESITES) )
        {
            compatible = false;
        }
    }
    else
    {
        if(zero != charCounts.end() && zero->second == 1 && (coding & RestrictionAscertainmentBias::NOSINGLETONABSENCE) )
        {
            compatible = false;
        }
        else if(one != charCounts.end() && one->second == 1 && (coding & RestrictionAscertainmentBias::NOSINGLETONPRESENCE) )
        {
            compatible = false;
        }
    }
    
    return compatible;
}

double RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::sumRootLikelihood( void )
{
#ifdef SSE_ENABLED
    double sumPartialProbs = sumUncorrectedRootLikelihood();
#else
    double sumPartialProbs = PhyloCTMCSiteHomogeneous<RestrictionState>::sumRootLikelihood();
#endif
    
    if(coding == RestrictionAscertainmentBias::ALL)
        return sumPartialProbs;
    
    
    // get the root node
    const TopologyNode &root = this->tau->getValue().getRoot();

    // get the index of the root node
    size_t nodeIndex = root.getIndex();
    
    const std::vector<double> &f = this->getRootFrequencies();
    
    std::vector<double>::const_iterator p_node = correctionLikelihoods.begin() + this->activeLikelihood[nodeIndex] * activeCorrectionOffset  + nodeIndex*correctionNodeOffset;
    
    std::vector<double> perMaskCorrections = std::vector<double>(numCorrectionMasks, 0.0);
    
    // iterate over each correction mask
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
        {    
            size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset;

            std::vector<double>::const_iterator         u_i = p_node   + offset;
            
            double prob = 0.0;
            
            for(size_t ci = 0; ci < this->numChars; ci++)
            {
                // constant site pattern likelihoods
                std::vector<double>::const_iterator         uC_i = u_i  + ci*this->numChars;
                // invert singleton likelihoods
                std::vector<double>::const_iterator         uI_i = uC_i + correctionOffset;
                
                if(coding & RestrictionAscertainmentBias::NOABSENCESITES)
                    prob += uC_i[0];
                        
                if(coding & RestrictionAscertainmentBias::NOSINGLETONPRESENCE)
                    prob += uI_i[0];

                if((coding & RestrictionAscertainmentBias::NOPRESENCESITES) && maskObservationCounts[mask] > 1)
                    prob += uC_i[1];

                // if there are only two observed tips, then don't double-count singleton gains
                // if there is only one observed tip, then don't double-count absence sites
                if((coding & RestrictionAscertainmentBias::NOSINGLETONABSENCE) && maskObservationCounts[mask] > 2)
                    prob += uI_i[1];
            }
            
            perMaskCorrections[mask] += prob;


            // invert the mixture probability
            double mixprob = prob;

            // add corrections for invariant sites
            double p_inv = this->pInv->getValue();
            if(p_inv > 0.0)
            {
                mixprob *= (1.0 - p_inv);
                
                if(coding & RestrictionAscertainmentBias::NOABSENCESITES)
                    mixprob += f[0]*p_inv;
                
                if(coding & RestrictionAscertainmentBias::NOPRESENCESITES)
                    mixprob += f[1]*p_inv;
            }
            
            // correct rounding errors
            if(mixprob <= 0.0)
                mixprob = 0.0;
        
            perMaskMixtureCorrections[mask*numSiteRates + mixture] = mixprob;
        }

        // add corrections for invariant sites
        double p_inv = this->pInv->getValue();
        if(p_inv > 0.0)
        {
            perMaskCorrections[mask] *= (1.0 - p_inv);

            if(coding & RestrictionAscertainmentBias::NOABSENCESITES)
                perMaskCorrections[mask] += f[0] * p_inv * this->numSiteRates;

            if(coding & RestrictionAscertainmentBias::NOPRESENCESITES)
                perMaskCorrections[mask] += f[1] * p_inv * this->numSiteRates;
        }

        // normalize and invert the log-probability
        perMaskCorrections[mask] = 1.0 - perMaskCorrections[mask]/this->numSiteRates;

        if(perMaskCorrections[mask] <= 0.0)
            perMaskCorrections[mask] = RbConstants::Double::inf;

        perMaskCorrections[mask] = log(perMaskCorrections[mask]);
        
        // apply the correction for this correction mask
        sumPartialProbs -= perMaskCorrections[mask]*correctionMaskCounts[mask];
    }

    return sumPartialProbs;
}

#ifdef SSE_ENABLED

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>

RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::PhyloCTMCSiteHomogeneousRestriction(const PhyloCTMCSiteHomogeneousRestriction &n) : PhyloCTMCSiteHomogeneousConditional<RestrictionState>(n),
        numSIMDBlocks(n.numSIMDBlocks), per_mixture_Likelihoods(NULL)
{
    size_t REALS_PER_SIMD_REGISTER = 2;

    if ( inMcmcMode == true )
    {
        per_mixture_Likelihoods = (double*)_mm_malloc(pattern_block_size * sizeof(double), REALS_PER_SIMD_REGISTER*sizeof(double));
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::resizeLikelihoodVectors( void )
{
    PhyloCTMCSiteHomogeneousConditional<RestrictionState>::resizeLikelihoodVectors();

    size_t REALS_PER_SIMD_REGISTER = 2;

    numSIMDBlocks               = size_t((pattern_block_size - 1)/REALS_PER_SIMD_REGISTER) + 1;
    siteOffset                  = 2*REALS_PER_SIMD_REGISTER;
    mixtureOffset               = numSIMDBlocks*siteOffset;
    nodeOffset                  = numSiteRates*mixtureOffset;
    activeLikelihoodOffset      = num_nodes*nodeOffset;

    // only do this if we are in MCMC mode. This will safe memory
    if ( inMcmcMode == true )
    {

        // we resize the partial likelihood vectors to the new dimensions
        _mm_free(partialLikelihoods);
        _mm_free(per_mixture_Likelihoods);

        partialLikelihoods = (double*)_mm_malloc(2*activeLikelihoodOffset * sizeof(double), REALS_PER_SIMD_REGISTER*sizeof(double));
        per_mixture_Likelihoods = (double*)_mm_malloc(pattern_block_size * sizeof(double), REALS_PER_SIMD_REGISTER*sizeof(double));

        // reinitialize likelihood vectors
        for (size_t i = 0; i < 2*activeLikelihoodOffset; i++)
        {
            partialLikelihoods[i] = 0.0;
        }

    }

    if ( useMarginalLikelihoods == true )
    {
        // we resize the partial likelihood vectors to the new dimensions
        delete [] marginalLikelihoods;

        marginalLikelihoods = new double[activeLikelihoodOffset];

        // reinitialize likelihood vectors
        for (size_t i = 0; i < activeLikelihoodOffset; i++)
        {
            marginalLikelihoods[i] = 0.0;
        }

    }

    perNodeSiteLogScalingFactors = std::vector<std::vector< std::vector<double> > >(2, std::vector<std::vector<double> >(num_nodes, std::vector<double>(numSIMDBlocks*REALS_PER_SIMD_REGISTER, 0.0) ) );

    transitionProbMatrices = std::vector<TransitionProbabilityMatrix>(numSiteRates, TransitionProbabilityMatrix(numChars) );

}

void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::computeRootLikelihood( size_t root, size_t left, size_t right)
{

    // get the root frequencies
    const std::vector<double> &f                    = this->getRootFrequencies();

    __m128d mf0 = _mm_load1_pd (&f[0]);
    __m128d mf1 = _mm_load1_pd (&f[1]);

    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partialLikelihoods + this->activeLikelihood[root]  * this->activeLikelihoodOffset + root  * this->nodeOffset;
    const double* p_left   = this->partialLikelihoods + this->activeLikelihood[left]  * this->activeLikelihoodOffset + left  * this->nodeOffset;
    const double* p_right  = this->partialLikelihoods + this->activeLikelihood[right] * this->activeLikelihoodOffset + right * this->nodeOffset;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {

        // iterate over all sites
        for (size_t site = 0; site < numSIMDBlocks; ++site)
        {
            size_t offset = mixture*mixtureOffset + site*siteOffset;

            // get the pointers to the likelihoods for this site and mixture category

            __m128d *          p_site_mixture    = (__m128d *)&*(p  + offset);
            __m128d *     p_site_mixture_left    = (__m128d *)&*(p_left  + offset);
            __m128d *    p_site_mixture_right    = (__m128d *)&*(p_right + offset);

            __m128d m1 = _mm_mul_pd (p_site_mixture_left[0], p_site_mixture_right[0]);

            __m128d m2 = _mm_mul_pd (p_site_mixture_left[1], p_site_mixture_right[1]);

            p_site_mixture[0] = _mm_mul_pd (m1, mf0);

            p_site_mixture[1] = _mm_mul_pd (m2, mf1);

        } // end-for over all sites (=patterns)

    } // end-for over all mixtures (=rate categories)

    if(coding != AscertainmentBias::ALL)
    {
        computeRootCorrection(root, left, right);
    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::computeRootLikelihood( size_t root, size_t left, size_t right, size_t middle)
{

    // get the root frequencies
    const std::vector<double> &f                    = this->getRootFrequencies();

    __m128d mf0 = _mm_load1_pd (&f[0]);
    __m128d mf1 = _mm_load1_pd (&f[1]);

    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partialLikelihoods + this->activeLikelihood[root]   * this->activeLikelihoodOffset + root   * this->nodeOffset;
    const double* p_left   = this->partialLikelihoods + this->activeLikelihood[left]   * this->activeLikelihoodOffset + left   * this->nodeOffset;
    const double* p_right  = this->partialLikelihoods + this->activeLikelihood[right]  * this->activeLikelihoodOffset + right  * this->nodeOffset;
    const double* p_middle = this->partialLikelihoods + this->activeLikelihood[middle] * this->activeLikelihoodOffset + middle * this->nodeOffset;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // iterate over all sites
        for (size_t site = 0; site < this->numSIMDBlocks; ++site)
        {
            size_t offset = mixture*mixtureOffset + site*siteOffset;

            // get the pointers to the likelihoods for this site and mixture category

            __m128d *          p_site_mixture    = (__m128d *)&*(p  + offset);
            __m128d *     p_site_mixture_left    = (__m128d *)&*(p_left  + offset);
            __m128d *    p_site_mixture_right    = (__m128d *)&*(p_right + offset);
            __m128d *    p_site_mixture_middle   = (__m128d *)&*(p_middle + offset);

            __m128d m1 = _mm_mul_pd (p_site_mixture_left[0], p_site_mixture_right[0]);
            m1 = _mm_mul_pd (m1, p_site_mixture_middle[0]);

            __m128d m2 = _mm_mul_pd (p_site_mixture_left[1], p_site_mixture_right[1]);
            m2 = _mm_mul_pd (m2, p_site_mixture_middle[1]);

            p_site_mixture[0] = _mm_mul_pd (m1, mf0);

            p_site_mixture[1] = _mm_mul_pd (m2, mf1);

        } // end-for over all sites (=patterns)

    } // end-for over all mixtures (=rate categories)

    if(coding != AscertainmentBias::ALL)
    {
        computeRootCorrection(root, left, right, middle);
    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::computeInternalNodeLikelihood(const TopologyNode &node, size_t nodeIndex, size_t left, size_t right)
{

    // compute the transition probability matrix
    this->updateTransitionProbabilities( nodeIndex, node.getBranchLength() );

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left  = this->partialLikelihoods + this->activeLikelihood[left]*this->activeLikelihoodOffset + left*this->nodeOffset;
    const double*   p_right = this->partialLikelihoods + this->activeLikelihood[right]*this->activeLikelihoodOffset + right*this->nodeOffset;
    double*         p_node  = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        TransitionProbabilityMatrix pij = this->transitionProbMatrices[mixture];

        __m128d mp00 = _mm_load1_pd (&pij[0][0]);
        __m128d mp01 = _mm_load1_pd (&pij[0][1]);
        __m128d mp10 = _mm_load1_pd (&pij[1][0]);
        __m128d mp11 = _mm_load1_pd (&pij[1][1]);

        // compute the per site probabilities
        for (size_t site = 0; site < this->numSIMDBlocks ; ++site)
        {
            size_t offset = mixture*mixtureOffset + site*siteOffset;

            // get the pointers to the likelihoods for this site and mixture category

            __m128d *          p_site_mixture    = (__m128d *)&*(p_node  + offset);
            __m128d *     p_site_mixture_left    = (__m128d *)&*(p_left  + offset);
            __m128d *    p_site_mixture_right    = (__m128d *)&*(p_right + offset);

            __m128d m1 = _mm_mul_pd (p_site_mixture_left[0], p_site_mixture_right[0]);

            __m128d m2 = _mm_mul_pd (p_site_mixture_left[1], p_site_mixture_right[1]);

            // iterate over all possible starting states

            __m128d m3 = _mm_mul_pd (m1, mp00);
            __m128d m4 = _mm_mul_pd (m2, mp01);

            p_site_mixture[0] = _mm_add_pd(m3,m4);

            m3 = _mm_mul_pd (m1, mp10);
            m4 = _mm_mul_pd (m2, mp11);

            p_site_mixture[1] = _mm_add_pd(m3,m4);

        } // end-for over all sites (=patterns)

    } // end-for over all mixtures (=rate-categories)

    if(coding != AscertainmentBias::ALL)
    {
        computeInternalNodeCorrection(node, nodeIndex, left, right);
    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::computeInternalNodeLikelihood(const TopologyNode &node, size_t nodeIndex, size_t left, size_t right, size_t middle)
{

    // compute the transition probability matrix
    this->updateTransitionProbabilities( nodeIndex, node.getBranchLength() );

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left      = this->partialLikelihoods + this->activeLikelihood[left]*this->activeLikelihoodOffset + left*this->nodeOffset;
    const double*   p_middle    = this->partialLikelihoods + this->activeLikelihood[middle]*this->activeLikelihoodOffset + middle*this->nodeOffset;
    const double*   p_right     = this->partialLikelihoods + this->activeLikelihood[right]*this->activeLikelihoodOffset + right*this->nodeOffset;
    double*         p_node      = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        TransitionProbabilityMatrix pij = this->transitionProbMatrices[mixture];

        __m128d mp00 = _mm_load1_pd (&pij[0][0]);
        __m128d mp01 = _mm_load1_pd (&pij[0][1]);
        __m128d mp10 = _mm_load1_pd (&pij[1][0]);
        __m128d mp11 = _mm_load1_pd (&pij[1][1]);

        // compute the per site probabilities
        for (size_t site = 0; site < this->numSIMDBlocks ; ++site)
        {
            size_t offset = mixture*mixtureOffset + site*siteOffset;

            // get the pointers to the likelihoods for this site and mixture category

            __m128d *          p_site_mixture    = (__m128d *)&*(p_node  + offset);
            __m128d *     p_site_mixture_left    = (__m128d *)&*(p_left  + offset);
            __m128d *    p_site_mixture_right    = (__m128d *)&*(p_right + offset);
            __m128d *    p_site_mixture_middle   = (__m128d *)&*(p_middle + offset);

            __m128d m1 = _mm_mul_pd (p_site_mixture_left[0], p_site_mixture_right[0]);
            m1 = _mm_mul_pd (m1, p_site_mixture_middle[0]);

            __m128d m2 = _mm_mul_pd (p_site_mixture_left[1], p_site_mixture_right[1]);
            m2 = _mm_mul_pd (m2, p_site_mixture_middle[1]);

            // iterate over all possible starting states

            __m128d m3 = _mm_mul_pd (m1, mp00);
            __m128d m4 = _mm_mul_pd (m2, mp01);

            p_site_mixture[0] = _mm_add_pd(m3,m4);

            m3 = _mm_mul_pd (m1, mp10);
            m4 = _mm_mul_pd (m2, mp11);

            p_site_mixture[1] = _mm_add_pd(m3,m4);

        } // end-for over all sites (=patterns)

    } // end-for over all mixtures (=rate-categories)

    if(coding != AscertainmentBias::ALL)
    {
        computeInternalNodeCorrection(node, nodeIndex, left, right, middle);
    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::computeTipLikelihood(const TopologyNode &node, size_t nodeIndex)
{

    size_t REALS_PER_SIMD_REGISTER = 2;

    double* p_node = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    const std::vector<bool> &gap_node = this->gapMatrix[nodeIndex];
    const std::vector<unsigned long> &char_node = this->charMatrix[nodeIndex];

    // compute the transition probabilities
    this->updateTransitionProbabilities( nodeIndex, node.getBranchLength() );

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        TransitionProbabilityMatrix pij = this->transitionProbMatrices[mixture];

        // iterate over all sites
        for (size_t site = 0; site != this->numSIMDBlocks; ++site)
        {
            double *          p_site_mixture    = p_node + mixture*mixtureOffset + site*siteOffset;

            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t pattern = site*REALS_PER_SIMD_REGISTER + ss;
                size_t index   = ss;

                if(pattern >= pattern_block_size)
                    break;

                // is this site a gap?
                if ( gap_node[pattern] )
                {
                    // since this is a gap we need to assume that the actual state could have been any state
                    p_site_mixture[index] = 1.0;
                    p_site_mixture[index + REALS_PER_SIMD_REGISTER] = 1.0;
                }
                else // we have observed a character
                {
                    // get the original character
                    unsigned long org_val = char_node[pattern];

                    p_site_mixture[index] = pij[0][org_val];
                    p_site_mixture[index + REALS_PER_SIMD_REGISTER] = pij[1][org_val];

                }
            }

        } // end-for over all sites/patterns in the sequence

    } // end-for over all mixture categories

    if(coding != AscertainmentBias::ALL)
    {
        computeTipCorrection(node, nodeIndex);
    }
}

double RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::sumUncorrectedRootLikelihood( void )
{
    // get the root node
    const TopologyNode &root = tau->getValue().getRoot();

    // get the index of the root node
    size_t node_index = root.getIndex();

    // get the pointers to the partial likelihoods of the left and right subtree
    double*   p_node  = this->partialLikelihoods + this->activeLikelihood[node_index] * this->activeLikelihoodOffset  + node_index*this->nodeOffset;

    size_t REALS_PER_SIMD_REGISTER = 2;

    for(size_t pattern = 0; pattern < this->pattern_block_size; pattern++)
    {
        per_mixture_Likelihoods[pattern] = 0.0;
    }

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {

        __m128d * mTotals = (__m128d *)per_mixture_Likelihoods;

        for (size_t block = 0; block < numSIMDBlocks; block++)
        {
            // get the pointers to the likelihoods for this site and mixture category
            __m128d *   p_site_mixture     = (__m128d *)(p_node + block*siteOffset + mixture*mixtureOffset);

            *mTotals = _mm_add_pd(*mTotals, _mm_add_pd(p_site_mixture[0], p_site_mixture[1]));

            mTotals++;
        }

    } // end-for over all mixtures (=rate categories)

    // sum the log-likelihoods for all sites together
    double sumPartialProbs = 0.0;
    // get the root frequencies
    const std::vector<double> &f = this->getRootFrequencies();
    double p_inv = pInv == NULL ? pInv->getValue() : 0.0;
    double oneMinusPInv = 1.0 - p_inv;
    if ( p_inv > 0.0 )
    {
        for (size_t block = 0; block < numSIMDBlocks; block++)
        {
            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t site = block*REALS_PER_SIMD_REGISTER + ss;

                if(site >= pattern_block_size)
                    continue;

                if ( RbSettings::userSettings().getUseScaling() == true )
                {
                    if ( this->siteInvariant[site] == true )
                    {
                        sumPartialProbs += log( p_inv * f[ this->invariantSiteIndex[site] ] * exp(this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site]) + oneMinusPInv * per_mixture_Likelihoods[site] / this->numSiteRates ) * patternCounts[site];
                    }
                    else
                    {
                        sumPartialProbs += log( oneMinusPInv * per_mixture_Likelihoods[site] / this->numSiteRates ) * patternCounts[site];
                    }
                }
                else // no scaling
                {

                    if ( this->siteInvariant[site] == true )
                    {
                        sumPartialProbs += log( p_inv * f[ this->invariantSiteIndex[site] ]  + oneMinusPInv * per_mixture_Likelihoods[site] / this->numSiteRates ) * patternCounts[site];
                    }
                    else
                    {
                        sumPartialProbs += log( oneMinusPInv * per_mixture_Likelihoods[site] / this->numSiteRates ) * patternCounts[site];
                    }

                }
            }
        }
    }
    else
    {

        for (size_t block = 0; block < numSIMDBlocks; block++)
        {
            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t site = block*REALS_PER_SIMD_REGISTER + ss;

                if(site >= pattern_block_size)
                    continue;

                sumPartialProbs += log( per_mixture_Likelihoods[site] / this->numSiteRates ) * patternCounts[site];

                if ( RbSettings::userSettings().getUseScaling() == true )
                {

                    sumPartialProbs -= this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] * patternCounts[site];
                }
            }

        }


    }


#ifdef RB_MPI

    // we only need to send message if there is more than one process
    if ( num_processes > 1 )
    {

        // send the likelihood from the helpers to the master
        if ( process_active == false )
        {
            // send from the workers the log-likelihood to the master
            MPI::COMM_WORLD.Send(&sumPartialProbs, 1, MPI::DOUBLE, active_PID, 0);
        }

        // receive the likelihoods from the helpers
        if ( process_active == true )
        {
            for (size_t i=active_PID+1; i<active_PID+num_processes; ++i)
            {
                double tmp = 0;
                MPI::COMM_WORLD.Recv(&tmp, 1, MPI::DOUBLE, int(i), 0);
                sumPartialProbs += tmp;
            }
        }

        // now send back the combined likelihood to the helpers
        if ( process_active == true )
        {
            for (size_t i=active_PID+1; i<active_PID+num_processes; ++i)
            {
                MPI::COMM_WORLD.Send(&sumPartialProbs, 1, MPI::DOUBLE, int(i), 0);
            }
        }
        else
        {
            MPI::COMM_WORLD.Recv(&sumPartialProbs, 1, MPI::DOUBLE, active_PID, 0);
        }

    }

#endif

    return sumPartialProbs;
}

void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::scale( size_t nodeIndex)
{
    size_t REALS_PER_SIMD_REGISTER = 2;

    double* p_node = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && nodeIndex % RbSettings::userSettings().getScalingDensity() == 0 )
    {
        // iterate over all mixture categories
        for (size_t block = 0; block < numSIMDBlocks ; ++block)
        {
            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t site = block*REALS_PER_SIMD_REGISTER + ss;

                if(site >= pattern_block_size)
                    continue;

                // the max probability
                double max = 0.0;

                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    max = std::max(max, *p_site_mixture);
                    max = std::max(max, *(p_site_mixture + REALS_PER_SIMD_REGISTER));
                }

                this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = -log(max);


                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    *p_site_mixture /= max;
                    *(p_site_mixture + REALS_PER_SIMD_REGISTER) /= max;

                }
            }

        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = 0;
        }

    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::scale( size_t nodeIndex, size_t left, size_t right )
{
    size_t REALS_PER_SIMD_REGISTER = 2;

    double* p_node = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && nodeIndex % RbSettings::userSettings().getScalingDensity() == 0 )
    {
        // iterate over all mixture categories
        for (size_t block = 0; block < numSIMDBlocks ; ++block)
        {
            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t site = block*REALS_PER_SIMD_REGISTER + ss;

                if(site >= pattern_block_size)
                    continue;

                // the max probability
                double max = 0.0;

                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    max = std::max(max, *p_site_mixture);
                    max = std::max(max, *(p_site_mixture + REALS_PER_SIMD_REGISTER));
                }

                this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] - log(max);


                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    *p_site_mixture /= max;
                    *(p_site_mixture + REALS_PER_SIMD_REGISTER) /= max;

                }
            }
        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site];
        }

    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousRestriction::scale( size_t nodeIndex, size_t left, size_t right, size_t middle )
{
    size_t REALS_PER_SIMD_REGISTER = 2;

    double* p_node   = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && nodeIndex % RbSettings::userSettings().getScalingDensity() == 0 )
    {
        // iterate over all mixture categories
        for (size_t block = 0; block < numSIMDBlocks ; ++block)
        {
            for (size_t ss = 0; ss < REALS_PER_SIMD_REGISTER; ss++)
            {
                size_t site = block*REALS_PER_SIMD_REGISTER + ss;

                if(site >= pattern_block_size)
                    continue;

                // the max probability
                double max = 0.0;

                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    max = std::max(max, *p_site_mixture);
                    max = std::max(max, *(p_site_mixture + REALS_PER_SIMD_REGISTER));
                }

                this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[middle]][middle][site] - log(max);


                // compute the per site probabilities
                for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
                {
                    // get the pointers to the likelihood for this mixture category
                    size_t offset = mixture*this->mixtureOffset + block*this->siteOffset;

                    double*          p_site_mixture          = p_node + offset + ss;

                    *p_site_mixture /= max;
                    *(p_site_mixture + REALS_PER_SIMD_REGISTER) /= max;

                }
            }
        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[nodeIndex]][nodeIndex][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[middle]][middle][site];
        }

    }
}

#endif
