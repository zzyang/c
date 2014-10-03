#ifndef __AST__
#define __AST__

#include <set>
#include <vector>
#include <string>
#include <boost/tuple/tuple.hpp> // for charset
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/variant.hpp>     // for tree nodes
#include <boost/optional.hpp>    // for multiplicity upperbound

namespace ast
{
    struct multiplicity
    {
        unsigned minoccurs;
        boost::optional<unsigned> maxoccurs;
        bool greedy;

        multiplicity(unsigned minoccurs = 1, boost::optional<unsigned> maxoccurs = 1)
            : minoccurs(minoccurs), maxoccurs(maxoccurs), greedy(true)
        { }

        bool unbounded() const { return !maxoccurs; }
        bool repeating() const { return !maxoccurs || *maxoccurs > 1; }
    };

    struct charset
    {
        bool negated;

        using range   = boost::tuple<char, char>; // from, till
        using element = boost::variant<char, range>;

        std::set<element> elements;
        // TODO: single set for loose elements, simplify() method
    };

    struct start_of_match {};
    struct end_of_match {};
    struct any_char {};
    struct group;

    typedef boost::variant<   // unquantified expression
        start_of_match,
        end_of_match,
        any_char,
        charset,
        std::string,          // literal
        boost::recursive_wrapper<group> // sub expression
    > simple;

    struct atom               // quantified simple expression
    {
        simple       expr;
        multiplicity mult;
    };

    using sequence    = std::vector<atom>;
    using alternative = std::vector<sequence>;
    using regex       = boost::variant<atom, sequence, alternative>;

    struct group {
        alternative root;

        group() = default;
        group(alternative root) : root(std::move(root)) { }
    };
}

#endif // __AST__
