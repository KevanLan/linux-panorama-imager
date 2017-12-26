#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>
#include <istream>
#include <sstream>
#include <stdint.h>

struct Util {

    /**
     * How to perform the split() operation
     */
    enum SplitMode {
        /** Normal split operation */
        SplitModeNormal,
        /** Allow for spaces and multiple consecutive occurences of the delimiter */
        SplitModeFuzzy,
        /** Take into account bash-like quoting and escaping rules */
        SplitModeQuoted
    };

    /**
     * split() - Splits a string into elements using a provided delimiter
     *
     * @s:          the string to split
     * @delim:      the delimiter to use
     * @elems:      the string vector to populate
     * @mode:       the SplitMode to use
     *
     * Using @delim to determine field boundaries, splits @s into separate
     * string elements.  These elements are returned in the string vector
     * @elems. As long as @s is non-empty, there will be at least one
     * element in @elems.
     */
    static void split(const std::string& src, char delim,
                      std::vector<std::string>& elems,
                      Util::SplitMode mode);

    /**
     * toString() - Converts a string to a plain-old-data type.
     *
     * @asString:   a string representation of plain-old-data.
     */
    template<typename T> static T fromString(const std::string& asString)
    {
        std::stringstream ss(asString);
        T retVal = T();
        ss >> retVal;
        return retVal;
    }
};

#endif /* UTIL_H */
