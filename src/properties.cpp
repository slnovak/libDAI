/*  Copyright (C) 2006-2008  Joris Mooij  [joris dot mooij at tuebingen dot mpg dot de]
    Radboud University Nijmegen, The Netherlands /
    Max Planck Institute for Biological Cybernetics, Germany

    This file is part of libDAI.

    libDAI is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libDAI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libDAI; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <iostream>
#include <dai/properties.h>
#include <dai/exceptions.h>


namespace dai {


/// Sends a single Property object to an output stream
std::ostream& operator<< (std::ostream & os, const Property & p) {
    os << p.first << "=";
    if( p.second.type() == typeid(size_t) )
        os << boost::any_cast<size_t>(p.second);
    else if( p.second.type() == typeid(std::string) )
        os << boost::any_cast<std::string>(p.second);
    else if( p.second.type() == typeid(double) )
        os << boost::any_cast<double>(p.second);
    else if( p.second.type() == typeid(bool) )
        os << boost::any_cast<bool>(p.second);
    else if( p.second.type() == typeid(PropertySet) )
        os << boost::any_cast<PropertySet>(p.second);
    else
        DAI_THROW(UNKNOWN_PROPERTY_TYPE);
    return( os );
}


/// Sends a PropertySet object to an output stream
std::ostream& operator<< (std::ostream & os, const PropertySet & ps) {
    os << "[";
    for( PropertySet::const_iterator p = ps.begin(); p != ps.end(); p++ ) {
        if( p != ps.begin() )
            os << ",";
        os << (Property)*p;
    }
    os << "]";
    return os;
}


/// Reads a PropertySet object from an input stream, storing values as strings
std::istream& operator >> (std::istream& is, PropertySet & ps) {
    ps = PropertySet();

    std::string s;
    is >> s;

    // Check whether s is of the form "[.*]"
    if( (s.length() < 2) || (s.at(0) != '[') || (s.at(s.length()-1)) != ']' )
        DAI_THROW(MALFORMED_PROPERTY);

    size_t N = s.length() - 1;
    for( size_t token_start = 1; token_start < N; ) {
        size_t token_end;

        // scan until '=' is found
        for( token_end = token_start + 1; token_end < N; token_end++ )
            if( s[token_end] == '=' )
                break;
        if( token_end == N )
            DAI_THROW(MALFORMED_PROPERTY);
        // we found a key
        std::string key = s.substr(token_start, token_end - token_start);

        token_start = token_end + 1;
        // scan until matching ',' is found
        int level = 0;
        for( token_end = token_start; token_end < N; token_end++ ) {
            if( s[token_end] == '[' )
                level++;
            else if( s[token_end] == ']' )
                level--;
            else if( (s[token_end] == ',') && (level == 0) )
                break;
        }
        if( !(level == 0) )
            DAI_THROW(MALFORMED_PROPERTY);
        // we found a vlue
        std::string value = s.substr(token_start, token_end - token_start);

        // store the key,value pair
        ps.Set(key,value);

        // go on with the next one
        token_start = token_end + 1;
    }

    return is;
}


} // end of namespace dai
