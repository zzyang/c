#include "RegExConverter.cpp"
#include "RegExTree.cpp"

int main()
{
    RegExConverter rc;

    string re = "(aa|bb)*a+bb#";
    //string re = "(abc)#";
    //string re = "3+4*2?(1-5)^2^3";

    string r;

    r = rc.formatRegEx(re);
    cout << r << "\n";

    r = rc.infixToPostfix(re);
    cout << r << "\n";

    RegExTree t(r);

    t.ReParseTree();

    //cout << r << "\n";
}
