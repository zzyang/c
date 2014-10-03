#include <map>
#include <string>
#include <set>
#include <stack>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>

#define null 0
#define OP_LEN 4
#define BP_LEN 1

using namespace std;
typedef pair <int, char> op_pair;

class RegExTree
{
  private:

    map<char, int> precedenceMap;
    map<char, int>::iterator it;

    typedef struct tree
    {
        char *ch;  // characters for leaf node, operator for inner node.
        //int pos;
        bool is_leaf;

        struct tree* lc;
        struct tree* rc;

        string name;

    } node;

    node* root;
    stack<tree*> Stack;

    queue<string> bracket;

    string r;

    ostringstream os;

    /**
    * Get char precedence.
    *
    * @param c char
    * @return corresponding precedence
    */

    int getPrecedence(char c)
    {
	it = precedenceMap.find(c);
	return (it ==  precedenceMap.end())? 6 : precedenceMap.at(c);
    }

  public:

    RegExTree(string re)
    {
        r = re;

        //precedenceMap.insert(op_pair('(', 1));
        precedenceMap.insert(op_pair('|', 2));
        precedenceMap.insert(op_pair('&', 3)); // explicit concatenation operator
        precedenceMap.insert(op_pair('?', 4));
        precedenceMap.insert(op_pair('*', 4));
        precedenceMap.insert(op_pair('+', 4));
        //precedenceMap.insert(op_pair('-', 3));
        //precedenceMap.insert(op_pair('^', 5));
    }


    /**
	 * Transform regular expression by inserting a '&' as explicit concatenation
	 * operator.
	 */

      string formatRegEx(string Regex)
      {
		string res;

		// add '#' in the end of string
		Regex += '#';

		// replace expression '[ ]' with '.', add the contents into stack

		int b, l; //begin and length of contents in [];
		string tmp;
		string c = ".";
        for (int i = 0; i < Regex.length(); i++)
        {
            if (Regex[i] == '[')
            {
                b = i;
                l = 1;
                while (Regex[b + l] != ']')
                {
                    l++;
                }
                tmp = Regex.substr(b, l+1);
                bracket.push(tmp);

                Regex.replace(b, l+1, c);
            }
            else if (Regex[i] == '.')
            {
                bracket.push(Regex.substr(i,1));
            }
        }

		char allOperators[OP_LEN] = {'|', '?', '+', '*'};
		char binaryOperators[BP_LEN] = {'|'};


		set<char> a1;
		set<char>::iterator ia1;
		set<char> b1;
		set<char>::iterator ib1;

		for (int i = 0; i < OP_LEN; i++)
            a1.insert(allOperators[i]);

        for (int i = 0; i < BP_LEN; i++)
            b1.insert(binaryOperators[i]);

		for (int i = 0; i < Regex.length(); i++) {
			char c1 = Regex[i];

			if (i + 1 < Regex.length()) {
				char c2 = Regex[i + 1];

				res += c1;

                ia1 = a1.find(c2);
                ib1 = b1.find(c1);

				if ((c1 != '(') && (c2 != ')') && (ia1 == a1.end()) && (ib1 == b1.end())) {
						res += '&';
				}
			}
		}
		res += Regex[Regex.length() - 1];

		return res;
	  }

    void dot(node* root)
    {
        string tmp = root->ch;

        if (root->lc)
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << tmp << "\",shape=\"Mrecord\",]" << "\n";
            os << root->name << " -> " << root->lc->name << "\n";
            dot(root->lc);
        }

        if (root->rc)
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << tmp << "\",shape=\"Mrecord\",]" << "\n";
            os << root->name << " -> " << root->rc->name << "\n";
            dot(root->rc);
        }
        else
        {
            os << root->name << "[fontname=\"Courier\",label=\"" << tmp << "\",shape=\"Mrecord\",]" << "\n";
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

/**
	 * Convert regular expression from infix to postfix notation using
	 * Shunting-yard algorithm.
	 *
	 * @param regex infix notation
	 * @return postfix notation
	 */

	  string infixToPostfix(string Regex)
	  {
		string postfix;

		stack<char> Stack;

		string formattedRegEx = formatRegEx(Regex);

		for (int i = 0; i <  formattedRegEx.length(); i++ ) {
            char c = formattedRegEx[i];
			switch (c) {
				case '(':
					Stack.push(c);
					break;

				case ')':
					while (Stack.top() != '(') {
						postfix += Stack.top();
						Stack.pop();
					}
					Stack.pop();
					break;

				case '|':
				case '?':
				case '+':
				case '*':
				case '&':
					while (Stack.size() > 0) {
						char peekedChar = Stack.top();

						int peekedCharPrecedence = getPrecedence(peekedChar);
						int currentCharPrecedence  = getPrecedence(c);

                        if (peekedChar == '(')
                        {
                            break;
                        }

						if (peekedCharPrecedence >= currentCharPrecedence) {
							postfix += Stack.top();
							Stack.pop();
						} else {
							break;
						}
					}
					Stack.push(c);
					break;
                default:
                    postfix += c;
                    break;
			}

		}


        // pop remaining elements in the stack
		while (Stack.size() > 0)
		{
			postfix += Stack.top();
			Stack.pop();
		}

		return postfix;
	}

    void ReParseTree(string re)
    {
        //string re = r;

        int len = re.length();
        for (int i = 0; i < len; i++)
        {
            node* n = new node;
            //n = (node *)malloc(sizeof(node));

            char* c = &re[i];
            string tmp;

            if (*c == '.')
            {

                tmp = bracket.front();
                bracket.pop();

                n->ch = (char *)malloc(tmp.length()+1);
                std::copy(tmp.begin(), tmp.end(), n->ch);
                n->ch[tmp.size()] = '\0';
            }
            else
            {
                n->ch = (char *)malloc(2);
                *(n->ch) = *c;
                n->ch[1] = '\0';
            }

            n->is_leaf = false;

            stringstream s;
            s << "node";
            s << i+1;
            //s << '\0';

            s >> n->name;

            switch (*c) {

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
