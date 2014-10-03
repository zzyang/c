//#include "RegExConverter.cpp"
#include "RegExTree.cpp"

int main()
{
    //RegExConverter rc;

    //string re = "(aa|bb)*[ab]+b.b";
    string r1;
    //string re = "[abc]a";
    //string re = "3+4*2?(1-5)^2^3";
    string re = "(asc)[\x0c\x0a\x0d\x09\x0b]*\x28.*\x29";
    RegExTree t(re);

    string r;

    r1 = t.formatRegEx(re);
    cout << r1 << "\n";

    r = t.infixToPostfix(re);
    cout << r << "\n";

    //RegExTree t(r);

    t.ReParseTree(r);

    //cout << r << "\n";
}
