#include "ast.hpp"
#include "parser.hpp"
#include <set>
#include <map>
#include <sstream>
#include <functional>
#include <iostream>

static std::string multiplicity_text(ast::multiplicity const& m) {
    std::ostringstream os;
    //
         if (m.minoccurs == 0 && m.unbounded())           os << '*'                                                ;
    else if (m.minoccurs == 1 && m.unbounded())           os << '+'                                                ;
    else if (m.unbounded())                               os << '{'  << m.minoccurs << ",}"                        ;
    // cannot be unbounded beyond this point
    else if (m.minoccurs == 1 && *m.maxoccurs == 1)       { ; }
    else if (m.minoccurs == *m.maxoccurs)                 os << '{'  << m.minoccurs << '}'                         ;
    else if (m.minoccurs == 0 && *m.maxoccurs == 1)       os << '?'                                                ;
    else if (m.minoccurs == 0)                            os << "{," << *m.maxoccurs << '}'                        ;
    else                                                  os << '{'  << m.minoccurs << ","  << *m.maxoccurs << "}" ;
    // non-greedyness
    if (m.repeating() && !m.greedy)
        os << "?";

    return os.str();
}

static std::ostream& escape_into(std::ostream& os, char v, bool escape_dquote = false) {
    switch(v)
    {
        case '\\': return os << "\\\\";
        case '\n': return os << "\\n";
        case '\t': return os << "\\t";
        case '\r': return os << "\\r";
        case '\b': return os << "\\b";
        case '\0': return os << "\\0";
        case '"':  return os << (escape_dquote? "\\\"" : "\"");
        default:
                   return os << v; // TODO more escapes
    }
}

static std::ostream& escape_into(std::ostream& os, std::string const& v, bool escape_dquote = false) {
    for(auto ch : v)
        escape_into(os, ch, escape_dquote);
    return os;
}

struct regex_tostring : boost::static_visitor<>
{
    std::ostream& os;
    int id = 0;

    regex_tostring(std::ostream& os) : os(os) {}
    ~regex_tostring() { os << "\n"; }

    void operator()(ast::alternative const & a) const {
        for (size_t i = 0; i<a.size(); ++i)
        {
            if (i) os << '|';
            (*this)(a[i]);
        }
    }

    void operator()(ast::atom const & v) const {
        boost::apply_visitor(*this, v.expr);
        (*this)(v.mult);
    }

    void operator()(ast::start_of_match    const & v) const { os << '^'; }
    void operator()(ast::end_of_match      const & v) const { os << '$'; }
    void operator()(ast::any_char          const & v) const { os << '.'; }
    void operator()(ast::group             const & v) const { os << '('; (*this)(v.root); os << ')'; }
    void operator()(char                   const v)   const { escape_into(os, v); }
    void operator()(std::string            const & v) const { escape_into(os, v); }
    void operator()(std::vector<ast::atom> const & v) const { for(auto& atom : v) (*this)(atom); }
    void operator()(ast::multiplicity      const & m) const { os << multiplicity_text(m); }

    void operator()(ast::charset           const & v) const {
        os << '[';
        if (v.negated) os << '^';
        for(auto& el : v.elements) boost::apply_visitor(*this, el);
        os << ']';
    }
    void operator()(ast::charset::range    const & v) const {
        using std::get;
        escape_into(os, get<0>(v)) << "-";
        escape_into(os, get<1>(v));
    }
};

struct regex_todigraph : boost::static_visitor<std::string>
{
    std::ostream& os;
    regex_todigraph(std::ostream& os, std::string const& label)  : os(os)
    {
        static int graph_id = 0;
        os << "subgraph cluster_regex" << std::to_string(++graph_id) << " {\n";
        os << "style=\"dashed\";\n";
        os << "node[fontname=\"times,italic\"];\n";
        os << "label=\"";
        escape_into(os, label, true) << "\";\n\n";
    }

    ~regex_todigraph()
    {
        os << "}\n";
    }

  private:
    using attributes = std::map<std::string, std::string>;

    static attributes merge(attributes const& a, attributes const& b)
    {
        attributes c = a;
        for (auto& e : b)
            c.insert(e);
        return c;
    }

    static attributes box()      { return { { "shape", "box" } }; }
    static attributes diamond()  { return { { "shape", "diamond" }, { "style", "rounded," } }; }
    static attributes sequence() { return { { "shape", "Mrecord" } }; }
    static attributes special()  { return { { "shape", "box" }, { "style", "filled," }, { "fillcolor", "gray" }, { "fontsize", "10" } }; }
    static attributes literal()  { return { { "fontname", "Courier" } }; }

    std::string emit_node(std::string const& caption, ast::multiplicity mult=ast::multiplicity(), attributes attrs = attributes()) const
    {
        static int id = 0;

        auto const thisnode = "node" + std::to_string(id++);

        auto const quantifier_text = multiplicity_text(mult);
        attrs.emplace("fontname", "Times,italic");
        attrs.emplace("label",    caption + quantifier_text);

        if (!mult.minoccurs)
            attrs["style"] += "dotted,";

        os << thisnode << "[";

        for(auto& attr : attrs)
        {
            escape_into(os, attr.first, true);
            os << "=\"";
            escape_into(os, attr.second, true) << "\",";
        }

        os << "];\n";

        if (mult.repeating())
        {
            // add self-referential edge for repeat (I know, it isn't a state chart...)
            os << thisnode << " -> " << thisnode
                << " [label=\"" << quantifier_text << "\""
                << (mult.greedy? ",color=red,arrowhead=dot" : ",color=blue,arrowhead=odot")
                << "];\n";
        }

        return thisnode;
    }

    std::string emit_node(std::string const& caption, attributes const& style) const
    {
        return emit_node(caption, ast::multiplicity(), style);
    }

    void emit_vertices(std::string const& parent, std::set<std::string> const& children) const {
        os << parent << " -> {";
        std::copy(begin(children), end(children), std::ostream_iterator<std::string>(os, "; "));
        os << "}\n";
    }

  public:
    std::string operator()(ast::alternative const& a) const {
        if (a.size()>1)
        {
            std::set<std::string> children;
            for (auto& branch: a)
                children.insert((*this)(branch));

            std::string const thisnode = emit_node("alternative", diamond());
            emit_vertices(thisnode, children);
            return thisnode;
        } else
        {
            return (*this)(a[0]);
        }
    }

    std::string operator()(ast::start_of_match const& v, ast::multiplicity mult = {}) const {
        return emit_node("start-of-match", mult, special());
    }
    std::string operator()(ast::end_of_match const& v, ast::multiplicity mult = {}) const {
        return emit_node("end-of-match", mult, special());
    }
    std::string operator()(ast::any_char const& v, ast::multiplicity mult = {}) const {
        return emit_node("any", mult, special());
    }
    std::string operator()(ast::group const& v, ast::multiplicity mult = {}) const {
        {
            auto child = (*this)(v.root);

            std::string const thisnode = emit_node("group", mult);
            emit_vertices(thisnode, { child });
            return thisnode;
        }
    }

    std::string operator()(ast::charset const& v, ast::multiplicity mult = {}) const {
        std::set<std::string> children;
        for (auto& el: v.elements)
            children.insert(boost::apply_visitor(*this, el));

        std::string const thisnode = emit_node(
                v.negated?"negated-charset":"charset",
                mult,
                { { "fontcolor", v.negated?"red":"blue" } });

        emit_vertices(thisnode, children);
        return thisnode;
    }
    std::string operator()(char v) const {
        std::ostringstream os;
        os             << "'";
        escape_into(os, v) << "'";
        return emit_node(os.str(), merge(box(), literal()));
    }
    std::string operator()(ast::charset::range const& v) const {
        std::ostringstream os;
        using std::get;
        os                     << "'";
        escape_into(os, get<0>(v)) << "…";
        escape_into(os, get<1>(v)) << "'";
        return emit_node(os.str(), merge(box(), literal()));
    }

    std::string operator()(std::string const& v, ast::multiplicity mult = {}) const {
        std::ostringstream os;
        os << (v.length()==1?"'":"\"");
        escape_into(os, v);
        os << (v.length()==1?"'":"\"");

        return emit_node(os.str(), mult, merge(box(), literal()));
    }

    std::string operator()(std::vector<ast::atom> const& v, ast::multiplicity mult = {}) const {
        if (v.size()>1)
        {
            std::set<std::string> children;
            for (auto& atom: v)
                children.insert((*this)(atom));

            std::string const thisnode = emit_node("sequence", mult, sequence());
            emit_vertices(thisnode, children);
            return thisnode;
        } else
        {
            for (auto& atom: v)
                return (*this)(atom);
        }

        return "";
    }

    std::string operator()(ast::atom const& atom) const {
#if 1
        // simplification that hides the 'atom' intermediate node and applies
        // `multiplicity` decoration directly on the simple node or sub group
        return boost::apply_visitor(std::bind(std::ref(*this), std::placeholders::_1, atom.mult), atom.expr);
#else
        // the "classical approach" emits the child and links it to an "atom" node
        auto child = boost::apply_visitor(*this, atom.expr);
        if (atom.mult == ast::multiplicity())
            return child;

        auto anode = emit_node("atom", atom.mult);
        emit_vertices(anode, { child });
        return anode;
#endif
    }
};

void check_roundtrip(ast::regex tree, std::string input)
{
    std::ostringstream os;
    regex_tostring str(os);

    boost::apply_visitor(str, tree);

    if (os.str() != input)
        std::cerr << "WARNING: '" << input << "' -> '" << os.str() << "'\n";
}

int main()
{
    std::cout << "digraph common {\n";

    for (std::string pattern: {
            "abc?",
            "ab+c",
            "(ab)+c",
            "[^-a\\-f-z\"\\]aaaa-]?",
            "abc|d",
            "a?",
            ".*?(a|b){,9}?",
            "(XYZ)|(123)",
            //"(and)[\x0c\x0a\x0d\x09\x0b]+[\x2b-]?[0123456789]+[\x0c\x0a\x0d\x09\x0b]*[><=]+[\x0c\x0a\x0d\x09\x0b]*[\x2b-]?[0123456789]+",
        })
    {
        std::cout << "// ================= " << pattern << " ========\n";
        ast::regex tree;
        if (doParse(pattern, tree))
        {
            check_roundtrip(tree, pattern);

            regex_todigraph printer(std::cout, pattern);
            boost::apply_visitor(printer, tree);
        }
    }

    std::cout << "}\n";
}
