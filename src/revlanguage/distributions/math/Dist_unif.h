#ifndef Dist_unif_H
#define Dist_unif_H

#include "UniformDistribution.h"
#include "RlContinuousDistribution.h"

namespace RevLanguage {
    
    
    /**
     * The RevLanguage wrapper of the uniform distribution.
     *
     * The RevLanguage wrapper of the uniform distribution simply
     * manages the interactions through the Rev with our core.
     * That is, the internal distribution object can be constructed and hooked up
     * in a model graph.
     * See the UniformDistribution for more details.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2012-08-08, version 1.0
     *
     */
    class Dist_unif :  public ContinuousDistribution {
    
    public:
        Dist_unif( void );
        virtual ~Dist_unif();
    
        // Basic utility functions
        Dist_unif*                                      clone(void) const;                                                                      //!< Clone the object
        static const std::string&                       getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::vector<std::string>                        getDistributionFunctionAliases(void) const;                                             //!< Get the alternative names used for the constructor function in Rev.
        std::string                                     getDistributionFunctionName(void) const;                                                //!< Get the Rev-name for this distribution.
        const TypeSpec&                                 getTypeSpec(void) const;                                                                //!< Get the type spec of the instance
        const MemberRules&                              getParameterRules(void) const;                                                          //!< Get member rules (const)
        void                                            printValue(std::ostream& o) const;                                                      //!< Print the general information on the function ('usage')
    
    
        // Distribution functions you have to override
        RevBayesCore::UniformDistribution*              createDistribution(void) const;
    
    protected:
    
        void                                            setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
    
    
    private:
        RevPtr<const RevVariable>                       lower;
        RevPtr<const RevVariable>                       upper;
        
    };
    
}

#endif
