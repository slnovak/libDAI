/*  This file is part of libDAI - http://www.libdai.org/
 *
 *  libDAI is licensed under the terms of the GNU General Public License version
 *  2, or (at your option) any later version. libDAI is distributed without any
 *  warranty. See the file COPYING for more details.
 *
 *  Copyright (C) 2002       Martijn Leisink  [martijn@mbfys.kun.nl]
 *  Copyright (C) 2006-2009  Joris Mooij      [joris dot mooij at libdai dot org]
 *  Copyright (C) 2002-2007  Radboud University Nijmegen, The Netherlands
 */


/// \file
/// \brief Defines the VarSet class, which represents a set of random variables.


#ifndef __defined_libdai_varset_h
#define __defined_libdai_varset_h


#include <vector>
#include <map>
#include <ostream>
#include <dai/var.h>
#include <dai/util.h>
#include <dai/smallset.h>


namespace dai {


/// Represents a set of variables.
/** \note A VarSet is implemented using a SmallSet<Var> instead
 *  of the more natural std::set<Var> because of efficiency reasons.
 *  That is, internally, the variables in the set are sorted according
 *  to their labels: the set of variables \f$\{x_l\}_{l\in L}\f$ is
 *  represented as a vector \f$(x_{l(0)},x_{l(1)},\dots,x_{l(|L|-1)})\f$
 *  where \f$l(0) < l(1) < \dots < l(|L|-1)\f$
 *  and \f$L = \{l(0),l(1),\dots,l(|L|-1)\}\f$.
 */
class VarSet : public SmallSet<Var> {
    public:
    /// @name Constructors and destructors
    //@{
        /// Default constructor (constructs an empty set)
        VarSet() : SmallSet<Var>() {}

        /// Construct from \link SmallSet \endlink<\link Var \endlink> \a x
        VarSet( const SmallSet<Var> &x ) : SmallSet<Var>(x) {}

        /// Construct a VarSet with one element, \a v
        VarSet( const Var &v ) : SmallSet<Var>(v) {}

        /// Construct a VarSet with two elements, \a v1 and \a v2
        VarSet( const Var &v1, const Var &v2 ) : SmallSet<Var>(v1,v2) {}

        /// Construct a VarSet from the range between \a begin and \a end.
        /** \tparam VarIterator Iterates over instances of type Var.
         *  \param begin Points to first Var to be added.
         *  \param end Points just beyond last Var to be added.
         *  \param sizeHint For efficiency, the number of elements can be speficied by \a sizeHint.
         */
        template <typename VarIterator>
        VarSet( VarIterator begin, VarIterator end, size_t sizeHint=0 ) : SmallSet<Var>(begin,end,sizeHint) {}
    //@}

    /// @name Queries
    //@{
        /// Calculates the number of states of this VarSet, which is simply the number of possible joint states of the variables in \c *this.
        /** The number of states of the Cartesian product of the variables in this VarSet
         *  is simply the product of the number of states of each variable in this VarSet.
         *  If \c *this corresponds with the set \f$\{x_l\}_{l\in L}\f$,
         *  where variable \f$x_l\f$ has label \f$l\f$, and denoting by \f$S_l\f$ the
         *  number of possible values ("states") of variable \f$x_l\f$, the number of
         *  joint configurations of the variables in \f$\{x_l\}_{l\in L}\f$ is given by \f$\prod_{l\in L} S_l\f$.
         */
        size_t nrStates() {
            size_t states = 1;
            for( VarSet::const_iterator n = begin(); n != end(); n++ )
                states *= n->states();
            return states;
        }

        /// Calculates the linear index in the Cartesian product of the variables in \c *this that corresponds to a particular joint assignment of the variables, specified by \a states.
        /** \param states Specifies the states of some variables.
         *  \return The linear index in the Cartesian product of the variables in \c *this
         *  corresponding with the joint assignment specified by \a states, where variables
         *  for which no state is specified are assumed to be in state 0.
         *
         *  The linear index is calculated as follows. The variables in \c *this are
         *  ordered according to their label (in ascending order); say \c *this corresponds with
         *  the set \f$\{x_{l(0)},x_{l(1)},\dots,x_{l(n-1)}\}\f$ with \f$l(0) < l(1) < \dots < l(n-1)\f$,
         *  where variable \f$x_l\f$ has label \a l. Denote by \f$S_l\f$ the number of possible values
         *  ("states") of variable \f$x_l\f$. The argument \a states corresponds
         *  with a mapping \a s that assigns to each variable \f$x_l\f$ a state \f$s(x_l) \in \{0,1,\dots,S_l-1\}\f$,
         *  where \f$s(x_l)=0\f$ if \f$x_l\f$ is not specified in \a states. The linear index \f$S\f$ corresponding
         *  with \a states is now calculated by:
         *  \f{eqnarray*}
         *    S &:=& \sum_{i=0}^{n-1} s(x_{l(i)}) \prod_{j=0}^{i-1} S_{l(j)} \\
         *      &= & s(x_{l(0)}) + s(x_{l(1)}) S_{l(0)} + s(x_{l(2)}) S_{l(0)} S_{l(1)} + \dots + s(x_{l(n-1)}) S_{l(0)} \cdots S_{l(n-2)}.
         *  \f}
         *
         *  \note If \c *this corresponds with \f$\{x_l\}_{l\in L}\f$, and \a states specifies a state
         *  for each variable \f$x_l\f$ for \f$l\in L\f$, calcState() induces a mapping
         *  \f$\sigma : \prod_{l\in L} X_l \to \{0,1,\dots,\prod_{l\in L} S_l-1\}\f$ that
         *  maps a joint state to a linear index; this is the inverse of the mapping
         *  \f$\sigma^{-1}\f$ induced by calcStates().
         */
        size_t calcState( const std::map<Var, size_t> &states ) {
            size_t prod = 1;
            size_t state = 0;
            for( VarSet::const_iterator n = begin(); n != end(); n++ ) {
                std::map<Var, size_t>::const_iterator m = states.find( *n );
                if( m != states.end() )
                    state += prod * m->second;
                prod *= n->states();
            }
            return state;
        }

        /// Calculates the joint assignment of the variables in \c *this corresponding to the linear index \a linearState.
        /** \param linearState should be smaller than nrStates().
         *  \return A mapping \f$s\f$ that maps each Var \f$x_l\f$ in \c *this to its state \f$s(x_l)\f$, as specified by \a linearState.
         *
         *  The variables in \c *this are ordered according to their label (in ascending order); say \c *this corresponds with
         *  the set \f$\{x_{l(0)},x_{l(1)},\dots,x_{l(n-1)}\}\f$ with \f$l(0) < l(1) < \dots < l(n-1)\f$,
         *  where variable \f$x_l\f$ has label \a l. Denote by \f$S_l\f$ the number of possible values
         *  ("states") of variable \f$x_l\f$ with label \a l.
         *  The mapping \f$s\f$ returned by this function is defined as:
         *  \f{eqnarray*}
         *    s(x_{l(i)}) = \left\lfloor\frac{S \mbox { mod } \prod_{j=0}^{i} S_{l(j)}}{\prod_{j=0}^{i-1} S_{l(j)}}\right\rfloor \qquad \mbox{for all $i=0,\dots,n-1$}.
         *  \f}
         *  where \f$S\f$ denotes the value of \a linearState.
         *
         *  \note If \c *this corresponds with \f$\{x_l\}_{l\in L}\f$, calcStates() induces a mapping
         *  \f$\sigma^{-1} : \{0,1,\dots,\prod_{l\in L} S_l-1\} \to \prod_{l\in L} X_l\f$ that
         *  maps a linear index to a joint state; this is the inverse of the mapping \f$\sigma\f$
         *  induced by calcState().
         */
        std::map<Var, size_t> calcStates( size_t linearState ) {
            std::map<Var, size_t> states;
            for( VarSet::const_iterator n = begin(); n != end(); n++ ) {
                states[*n] = linearState % n->states();
                linearState /= n->states();
            }
            DAI_ASSERT( linearState == 0 );
            return states;
        }
    //@}

    /// @name Input and output
    //@{
        /// Writes a VarSet to an output stream
        friend std::ostream& operator<<( std::ostream &os, const VarSet &vs )  {
            os << "{";
            for( VarSet::const_iterator v = vs.begin(); v != vs.end(); v++ )
                os << (v != vs.begin() ? "," : "") << *v;
            os << "}";
            return( os );
        }
    //@}
};


} // end of namespace dai


/** \example example_varset.cpp
 *  This example shows how to use the Var and VarSet classes. It also explains the concept of "states" for VarSets.
 *
 *  \section Output
 *  \verbinclude examples/example_varset.out
 *
 *  \section Source
 */


#endif
