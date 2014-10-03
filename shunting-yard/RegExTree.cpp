#include <map>
#include <string>
#include <set>
#include <stack>
#include <iostream>
#include <sstream>
#include <fstream>

#define null 0

using namespace std;

class RegExTree
{
  private:

    typedef struct tree
    {
        char *ch;  // characters for leaf node, operator for inner node.
        //int pos;
        bool is_leaf;

        struct tree* lc;
        struct tree* rc;

        char name[10];

    } node;

    node* root;
    stack<tree*> Stack;

    string r;

    ostringstream os;

  public:

    RegExTree(string re)
    {
        r = re;
    }

    void dot(node* root)
    {
        if (root->lc)
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << *(root->ch) << "\",shape=\"Mrecord\",]" << "\n";
            os << root->name << " -> " << root->lc->name << "\n";
            dot(root->lc);
        }

        if (root->rc)
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << *(root->ch) << "\",shape=\"Mrecord\",]" << "\n";
            os << root->name << " -> " << root->rc->name << "\n";
            dot(root->rc);
        }
        else
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << *(root->ch) << "\",shape=\"Mrecord\",]" << "\n";
            //os << *(root->ch) << "\n";
        }
    }

    void  Display(node* root)
    {
        //std::ostringstream os;
        os << "digraph common {\n";
        os << "label=\"" << r << "\"\n";

        std::ofstream f("graph.dot");

        dot(root);


        os << "}" << endl;

        f << os.str() << endl;
    }

    void ReParseTree()
    {
        string re = r;

        int len = re.length();
        for (int i = 0; i < len; i++)
        {
            char* c = &re[i];

            node* n;
            n = (node *)malloc(sizeof(node));

            n->ch = c;
            n->is_leaf = false;

            stringstream s;
            s << "node";
            s << i+1;

            //cout << s.str() << endl;
            //string name = s.str();
            s >> n->name;

            switch (*c) {
			case '[':
				break;
			case ']':
				break;

			case '?':
			case '+':
			case '*':
                n->rc = null;
                n->lc = Stack.top();
                Stack.pop();
                Stack.push(n);
                break;

            case '|':
			case '&':
                n->rc = Stack.top();
                Stack.pop();
                n->lc = Stack.top();
                Stack.pop();
                Stack.push(n);
                break;

            default:
                n->is_leaf = true;
                n->lc = null;
                n->rc = null;
                Stack.push(n);
                break;
            };

        }

        root = Stack.top();
        Stack.pop();

        Display(root);

        //return root;
    };

};

/*

int main()
{

}
*/
