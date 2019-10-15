#ifndef Dist_PhyloMultiSampleOrnsteinUhlenbeck_H
#define Dist_PhyloMultiSampleOrnsteinUhlenbeck_H

#include <math.h>
#include <iosfwd>
#include <string>
#include <vector>

#include "ContinuousCharacterData.h"
#include "RlContinuousCharacterData.h"
#include "RlTypedDistribution.h"
#include "DagMemberFunction.h"
#include "DeterministicNode.h"
#include "DynamicNode.h"
#include "RevObject.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "RlDagMemberFunction.h"
#include "RlDeterministicNode.h"
#include "RlStochasticNode.h"
#include "RlTypedFunction.h"
#include "StochasticNode.h"
#include "TypedDagNode.h"
#include "TypedDistribution.h"
#include "TypedFunction.h"

namespace RevLanguage {
class TypeSpec;
    
    class Dist_PhyloMultiSampleOrnsteinUhlenbeck :  public TypedDistribution< ContinuousCharacterData > {
        
    public:
        Dist_PhyloMultiSampleOrnsteinUhlenbeck( void );
        virtual ~Dist_PhyloMultiSampleOrnsteinUhlenbeck();
        
        // Basic utility functions
        Dist_PhyloMultiSampleOrnsteinUhlenbeck*                 clone(void) const;                                                                      //!< Clone the object
        static const std::string&                               getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                                  getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::string                                             getDistributionFunctionName(void) const;                                                //!< Get the Rev-name for this distribution.
        const TypeSpec&                                         getTypeSpec(void) const;                                                                //!< Get the type spec of the instance
        const MemberRules&                                      getParameterRules(void) const;                                                          //!< Get member rules (const)
        void                                                    printValue(std::ostream& o) const;                                                      //!< Print the general information on the function ('usage')
        
        
        // Distribution functions you have to override
        RevBayesCore::TypedDistribution< RevBayesCore::ContinuousCharacterData >*      createDistribution(void) const;
        
    protected:
        
        void                                                    setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
        
        
    private:
        
        RevPtr<const RevVariable>                               tree;
        RevPtr<const RevVariable>                               alpha;
        RevPtr<const RevVariable>                               theta;
        RevPtr<const RevVariable>                               sigma;
        RevPtr<const RevVariable>                               within_species_variances;
        RevPtr<const RevVariable>                               root_states;
        RevPtr<const RevVariable>                               n_sites;
        RevPtr<const RevVariable>                               taxa;                                                                                   //!< The taxons

        
    };
    
}


#endif

