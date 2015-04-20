#ifndef Dist_chisq_H
#define Dist_chisq_H

#include "ChisqDistribution.h"
#include "RealPos.h"
#include "RlPositiveContinuousDistribution.h"

namespace RevLanguage {
    
    /**
     * The RevLanguage wrapper of the chi-square distribution.
     *
     * The RevLanguage wrapper of the chi-square distribution simply
     * manages the interactions through the Rev with our core.
     * That is, the internal distribution object can be constructed and hooked up
     * in a model graph.
     * See the ChisqDistribution for more details.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2015-04-19, version 1.0
     *
     */
    class Dist_chisq : public PositiveContinuousDistribution {
        
    public:
        Dist_chisq(void);
        virtual                                        ~Dist_chisq(void);
        
        // Basic utility functions
        Dist_chisq*                                     clone(void) const;                                                              //!< Clone the object
        static const std::string&                       getClassType(void);                                                             //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                         //!< Get class type spec
        const TypeSpec&                                 getTypeSpec(void) const;                                                        //!< Get the type spec of the instance
        const MemberRules&                              getParameterRules(void) const;                                                     //!< Get member rules (const)
        void                                            printValue(std::ostream& o) const;                                              //!< Print the general information on the function ('usage')
        
        
        // Distribution functions you have to override
        RevBayesCore::ChisqDistribution*                createDistribution(void) const;
        
    protected:
        
        void                                            setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);     //!< Set member variable
        
        
    private:
        
        RevPtr<const RevVariable>                       degrees;

    };
    
}

#endif
