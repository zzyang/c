#include <map>
#include <string>
#include <set>
#include <stack>
#include <iostream>

#define null 0
#define OP_LEN 4
#define BP_LEN 1

using namespace std;
typedef pair <int, char> op_pair;


class RegExConverter {

	/** Operators precedence map. The more the higher **/
	private:
	  map<char, int> precedenceMap;
	  map<char, int>::iterator it;

	public:
	 RegExConverter()
	 {
        //precedenceMap.insert(op_pair('(', 1));
		precedenceMap.insert(op_pair('|', 2));
		precedenceMap.insert(op_pair('&', 3)); // explicit concatenation operator
		precedenceMap.insert(op_pair('?', 4));
        precedenceMap.insert(op_pair('*', 4));
		precedenceMap.insert(op_pair('+', 4));
		//precedenceMap.insert(op_pair('-', 3));
		//precedenceMap.insert(op_pair('^', 5));
	 };

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

	/**
	 * Transform regular expression by inserting a '@' as explicit concatenation
	 * operator.
	 */

      string formatRegEx(string Regex)
      {
		string res;
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
};


