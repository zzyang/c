// #define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include "parser.hpp"
#include "ast.hpp"
#include <boost/fusion/adapted.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

BOOST_FUSION_ADAPT_STRUCT(ast::charset,
        (bool, negated)
        (std::set<ast::charset::element>, elements))

BOOST_FUSION_ADAPT_STRUCT(ast::atom,
        (ast::simple, expr)
        (ast::multiplicity, mult))

#if defined(BOOST_SPIRIT_DEBUG)
namespace ast { // DEBUG facilities
    template <typename T> std::ostream& operator<<(std::ostream& os, T const& v) {
        using std::operator<<;
        return os << std::string(typeid(v).name());
    }
}
#endif

namespace qi  = boost::spirit::qi;
namespace phx = boost::phoenix;

template <typename It>
    struct parser : qi::grammar<It, ast::alternative()>
{
    parser() : parser::base_type(alternative)
    {
        using namespace qi;
        using phx::construct;
        using ast::multiplicity;

        alternative = sequence % '|';
        sequence    = *atom;

        simple      =
                      (group)
                    | (charset)
                    | ('.' >> qi::attr(ast::any_char()))
                    | ('^' >> qi::attr(ast::start_of_match()))
                    | ('$' >> qi::attr(ast::end_of_match()))
                    // optimize literal tree nodes by grouping unquantified literal chars
                    | (as_string [ +(literal >> !char_("{?+*")) ])
                    | (as_string [ literal ]) // lone char/escape + explicit_quantifier
                    ;

        atom        = (simple >> quantifier); // quantifier may be implicit

        explicit_quantifier  =
                    // bounded ranges:
                      lit('?')                                   [ _val = construct<multiplicity>( 0, 1)   ]
                    | ('{'  >> uint_ >> '}' )                    [ _val = construct<multiplicity>(_1, _1)  ]
                    // repeating ranges can be marked non-greedy:
                    | (
                          lit('+')                               [ _val = construct<multiplicity>( 1, boost::none) ]
                        | lit('*')                               [ _val = construct<multiplicity>( 0, boost::none) ]
                        | ('{'  >> uint_ >> ",}")                [ _val = construct<multiplicity>(_1, boost::none) ]
                        | ('{'  >> uint_ >> "," >> uint_ >> '}') [ _val = construct<multiplicity>(_1, _2)  ]
                        | ("{," >> uint_ >> '}' )                [ _val = construct<multiplicity>( 0, _1)  ]
                      ) >> -lit('?')       [ phx::bind(&multiplicity::greedy, _val) = false ]
                    ;

        quantifier = explicit_quantifier | attr(ast::multiplicity());

        charset     = '['
                   >> (lit('^') >> attr(true) | attr(false)) // negated
                   >> *(range | charset_el)
                    > ']'
                    ;

        range       = charset_el >> '-' >> charset_el;

        group       = '(' >> alternative >> ')';

        literal     = unescape | ~char_("\\+*?.^$|{()") ;

        unescape    = ('\\' > char_);

        // helper to optionally unescape waiting for raw ']'
        charset_el  = !lit(']') >> (unescape|char_);

        BOOST_SPIRIT_DEBUG_NODES(
                (alternative) (sequence)
                (simple) (atom)
                (explicit_quantifier) (quantifier)
                (charset) (charset_el) (range) (group) (literal) (unescape)
                )
    }

  private:
    qi::rule<It, ast::alternative()>    alternative;
    qi::rule<It, ast::sequence()>       sequence;
    qi::rule<It, ast::atom()>           atom;
    qi::rule<It, ast::simple()>         simple;
    qi::rule<It, ast::multiplicity()>   explicit_quantifier, quantifier;
    qi::rule<It, ast::charset()>        charset;
    qi::rule<It, ast::charset::range()> range;
    qi::rule<It, ast::group()>          group;
    qi::rule<It, char()>                literal, unescape, charset_el;
};

bool doParse(const std::string& input, ast::regex& data)
{
    typedef std::string::const_iterator It;

    static const parser<It> p;

    try
    {
        auto f(begin(input)), l(end(input));
        bool ok = qi::parse(f,l,p,data);
        if (!ok)
            std::cerr << "parse failed: '" << std::string(f,l) << "'\n";

        if (f!=l) std::cerr << "trailing unparsed: '" << std::string(f,l) << "'\n";
        return ok;
    } catch(const qi::expectation_failure<It>& e)
    {
        std::string frag(e.first, e.last);
        std::cerr << e.what() << " at '" << frag << "'\n";
        return false;
    }
}
