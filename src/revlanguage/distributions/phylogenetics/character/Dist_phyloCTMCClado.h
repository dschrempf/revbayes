#ifndef Dist_phyloCTMCClado_H
#define Dist_phyloCTMCClado_H

#include "AbstractHomologousDiscreteCharacterData.h"
#include "RlAbstractHomologousDiscreteCharacterData.h"
#include "RlTypedDistribution.h"

namespace RevLanguage {
    
    class Dist_phyloCTMCClado :  public TypedDistribution< AbstractHomologousDiscreteCharacterData > {
        
    public:
        Dist_phyloCTMCClado( void );
        virtual ~Dist_phyloCTMCClado();
        
        // Basic utility functions
        Dist_phyloCTMCClado*                            clone(void) const;                                                                      //!< Clone the object
        static const std::string&                       getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::string                                     getDistributionFunctionName(void) const;                                                //!< Get the Rev-name for this distribution.
        const TypeSpec&                                 getTypeSpec(void) const;                                                                //!< Get the type spec of the instance
        const MemberRules&                              getParameterRules(void) const;                                                          //!< Get member rules (const)
        void                                            printValue(std::ostream& o) const;                                                      //!< Print the general information on the function ('usage')
        
        
        // Distribution functions you have to override
        RevBayesCore::TypedDistribution< RevBayesCore::AbstractHomologousDiscreteCharacterData >*      createDistribution(void) const;
        
    protected:
        
        void                                            setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
        
        
    private:
        
        RevPtr<const RevVariable>                       tree;
        RevPtr<const RevVariable>                       q;
        RevPtr<const RevVariable>                       cladoProbs;
        RevPtr<const RevVariable>                       cladoTimes;
        RevPtr<const RevVariable>                       rate;
        RevPtr<const RevVariable>                       siteRates;
        RevPtr<const RevVariable>                       rootFrequencies;
        RevPtr<const RevVariable>                       pInv;
        RevPtr<const RevVariable>                       nSites;
        RevPtr<const RevVariable>                       nStates;
        RevPtr<const RevVariable>                       type;
        RevPtr<const RevVariable>                       treatAmbiguousAsGap;
        
    };
    
}


#endif
