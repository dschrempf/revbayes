//
//  Mntr_CharacterHistoryTimeInStates.h
//  RevBayes_development_branch
//
//  Created by will freyman on 4/25/18.
//  Copyright (c) 2018 will freyman. All rights reserved.
//

#ifndef __RevBayes_development_branch__Mntr_CharacterHistoryTimeInStates__
#define __RevBayes_development_branch__Mntr_CharacterHistoryTimeInStates__

#include "CharacterHistoryTimeInStatesMonitor.h"
#include "RlMonitor.h"

#include <ostream>
#include <string>


namespace RevLanguage {
    
    class Mntr_CharacterHistoryTimeInStates : public Monitor {
        
    public:
        
        Mntr_CharacterHistoryTimeInStates(void);                                                                                            //!< Default constructor
        
        // Basic utility functions
        virtual Mntr_CharacterHistoryTimeInStates*      clone(void) const;                                                                      //!< Clone object
        void                                            constructInternalObject(void);                                                          //!< We construct the a new internal monitor.
        static const std::string&                       getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::string                                     getMonitorName(void) const;                                                             //!< Get the name used for the constructor function in Rev.
        const MemberRules&                              getParameterRules(void) const;                                                          //!< Get member rules (const)
        virtual const TypeSpec&                         getTypeSpec(void) const;                                                                //!< Get language type of the object
        virtual void                                    printValue(std::ostream& o) const;                                                      //!< Print value (for user)
        
    protected:
        
        void                                            setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
        
        std::vector<RevPtr<const RevVariable> >         vars;
        RevPtr<const RevVariable>                       filename;
        RevPtr<const RevVariable>                       printgen;
        RevPtr<const RevVariable>                       version;
        RevPtr<const RevVariable>                       cdbdp;
        RevPtr<const RevVariable>                       separator;
        RevPtr<const RevVariable>                       append;

    };
    
}


#endif
