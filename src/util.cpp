#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <dirent.h>

#include "log.h"
#include "util.h"

using std::string;
using std::vector;

static void fill_escape_vector(const string &str, vector<bool> &esc_vec)
{
    enum State {
        StateNormal,
        StateEscaped,
        StateDoubleQuoted,
        StateDoubleQuotedEscaped,
        StateSingleQuoted
    };

    State state = StateNormal;

    for (string::const_iterator iter = str.begin();
         iter != str.end();
         iter++) {
        const char c(*iter);
        bool esc = false;

        switch (state) {
        case StateNormal:
            if (c == '"')
                state = StateDoubleQuoted;
            else if (c == '\\')
                state = StateEscaped;
            else if (c == '\'')
                state = StateSingleQuoted;
            break;
        case StateEscaped:
            esc = true;
            state = StateNormal;
            break;
        case StateDoubleQuoted:
            if (c == '"')
                state = StateNormal;
            else if (c == '\\')
                state = StateDoubleQuotedEscaped;
            else
                esc = true;
            break;
        case StateDoubleQuotedEscaped:
            esc = true;
            state = StateDoubleQuoted;
            break;
        case StateSingleQuoted:
            if (c == '\'')
                state = StateNormal;
            else
                esc = true;
        default:
            break;
        }

        esc_vec.push_back(esc);
    }
}

static void split_normal(const string& src, char delim, vector<string>& elementVec)
{
    std::stringstream ss(src);
    string item;
    while(std::getline(ss, item, delim))
        elementVec.push_back(item);
}

static void split_fuzzy(const string& src, char delim, vector<string>& elementVec)
{
    // Fuzzy case: Initialize our delimiter string based upon the caller's plus
    // a space to allow for more flexibility.
    string delimiter(" ");
    delimiter += delim;
    // Starting index into the string of the first token (by definition, if
    // we're parsing a string, there is at least one token).
    string::size_type startPos(0);
    // string::find_first_of() looks for any character in the string provided,
    // it is not treated as a sub-string, so regardless of where the space or
    // comma is or how many there are, the result is the same.
    string str(src);
    string::size_type endPos = str.find_first_of(delimiter);
    while (endPos != string::npos) {
        // Push back the current element starting at startPos for
        // (endPos - startPos) characters.  std::string takes care of
        // terminators, etc.
        elementVec.push_back(string(str, startPos, endPos - startPos));
        // Index of the next element after any delimiter characters.  Same
        // caveat applies to find_first_not_of() that applies to
        // find_first_of(); endPos tells it where to start the search.
        string::size_type nextPos = str.find_first_not_of(delimiter, endPos);
        // Erase the part of the string we've already parsed.
        str = str.erase(startPos, nextPos - startPos);
        // Look for the next delimiter.  If there isn't one, we bail out.
        endPos = str.find_first_of(delimiter);
    }
    // Regardless of whether we initially had one element or many, 'str' now
    // only contains one.
    elementVec.push_back(str);
}

static void split_quoted(const string& src, char delim, vector<string>& elementVec)
{
    std::stringstream ss;
    vector<bool> escVec;

    /* Mark characters in the string as escaped or not */
    fill_escape_vector(src, escVec);

    /* Sanity check... */
    if (src.length() != escVec.size())
        return;

    for (vector<bool>::const_iterator iter = escVec.begin();
         iter != escVec.end();
         iter++) {
        bool escaped = static_cast<bool>(*iter);
        char c = src[iter - escVec.begin()];

        /* Output all characters, except unescaped ",\,' */
        if ((c != '"' && c != '\\' && c != '\'') || escaped) {
            /* If we reach an unescaped delimiter character, do a split */
            if (c == delim && !escaped) {
                elementVec.push_back(ss.str());
                ss.str("");
                ss.clear();
            } else {
                ss << c;
            }
        }

    }

    /* Handle final element, delimited by end of string */
    const string &finalElement(ss.str());
    if (!finalElement.empty())
        elementVec.push_back(finalElement);
}

void Util::split(const string& src, char delim, vector<string>& elementVec,
            Util::SplitMode mode)
{
    // Trivial rejection
    if (src.empty()) {
        return;
    }

    switch (mode) {
    case Util::SplitModeNormal:
        return split_normal(src, delim, elementVec);
    case Util::SplitModeFuzzy:
        return split_fuzzy(src, delim, elementVec);
    case Util::SplitModeQuoted:
        return split_quoted(src, delim, elementVec);
    default:
        break;
    }
}
