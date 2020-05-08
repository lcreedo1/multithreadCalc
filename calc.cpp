/**
 * Liam Creedon
 * lcreedo1@jhu.edu
 * hw6 part 1
 */

#include "calc.h"
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

struct Calc{
};

class CalcImpl : public Calc {

    map<string, int> vars;
    pthread_mutex_t mutX;

public:
    CalcImpl() {
        pthread_mutex_init(&mutX, NULL);
    }

    ~CalcImpl () {
        pthread_mutex_destroy(&mutX);
    }

    int evalExpr(string expr, int &result) {
        vector<string> newExpr = tokenize(expr);
        int exprLength = 0;
        exprLength = newExpr.size();
        if ((exprLength != 1) && (exprLength != 3) && (exprLength != 5)) {
            //cout << "failing size check" << exprLength;
            return 0;
        }
        int goodExpr = validateExpr(newExpr);
        if (goodExpr == 1) {
            //is a good expression so determine result of expression
            //pointers needed?
            if (exprLength == 1) {
                pthread_mutex_lock(&mutX);
                result = eval1(newExpr);
                pthread_mutex_unlock(&mutX);
            } else if (exprLength == 3) {
                pthread_mutex_lock(&mutX);
                result = eval3(newExpr);
                pthread_mutex_unlock(&mutX);
            } else if (exprLength == 5) {
                pthread_mutex_lock(&mutX);
                result = eval5(newExpr);
                pthread_mutex_unlock(&mutX);
            }
            return goodExpr;
        }
        //should be 0 indicating bad expression
        return goodExpr;
    }

    vector<string> tokenize(const string &expr) {
        vector<string> vec;
        stringstream s(expr);

        string tok;
        while (s >> tok) {
            vec.push_back(tok);
        }

        return vec;
    }

    // returns 0 if invalid expression, 1 if valid
    int validateExpr(vector<string> ex) {
        int size = ex.size(); // one, three, or five

        if (size == 1) { // operand
            return validate1(ex);
        } else if (size == 3) { // operand op operand || var = operand
            return validate3(ex);
        } else { // var = operand op operand
            return validate5(ex);
        }
        return 0;
    }

    int validate1(vector<string> ex) {
        // if digits and uniform, return 1
        // if chars, uniform, and char is in map, return 1
        bool uni = isUniform(ex[0]);
        if (!uni) { // invalid input if not uniform
            //cout << "is not uniform";
            return 0;
        }
        if (isDigit(ex[0][0]) || (isSigned(ex[0][0]) && isDigit(ex[0][1]))) {
            // valid input if string is digits or signed digits
            return 1;
        }
        // otherwise string will be chars, so see if it exists in vars
        return findVar(ex[0]);
    }

    int validate3(vector<string> ex) {
        if (ex[1].length() >  1) {
            return 0;
        }
        if (isOp(ex[1][0])) { // operand op operand
            bool op1 = isUniform(ex[0]);
            bool op2 = isUniform(ex[2]);
            if (!op1 || !op2) {
                return 0;
            }

            bool tok1Dig = (isDigit(ex[0][0]) || (isSigned(ex[0][0]) && isDigit(ex[0][1])));
            bool tok2Dig = (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1])));

            if (tok1Dig && tok2Dig) {
                if (ex[1][0] == '/' && stoi(ex[2]) == 0) {
                    return 0; // div by 0
                } else {
                    return 1; //both operands are digits
                }
            } else if (tok1Dig && !tok2Dig) {
                int found = findVar(ex[2]); //checks if operand 2 exists in map
                if (found == 1) {
                    if (ex[1][0] == '/' && this->vars[ex[2]] == 0) {
                        return 0; // div by 0
                    }
                    return 1;
                }
                return 0;
            } else if (!tok1Dig && tok2Dig) {
                if (ex[1][0] == '/' && stoi(ex[2]) == 0) {
                    return 0; // div by 0
                }
                return findVar(ex[0]);

            } else { //checks if both operands exist in map
                if ((findVar(ex[0]) == 1) && (findVar(ex[2]) == 1)) {
                    if (ex[1][0] == '/' && this->vars[ex[2]] == 0) {
                        return 0; // div by 0
                    }
                    return 1;
                }
                return 0;
            }

        } else if (isEq(ex[1][0])) { // var = operand
            bool op1 = isUniform(ex[0]);
            bool op2 = isUniform(ex[2]);
            if (!op1 || !op2) {
                return 0;
            }
            if (!isAlpha(ex[0][0])) { // first token is not a variable name
                return 0;
            }
            if (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1]))) {
                // operand is a number or signed number
                return 1;
            } else if (isAlpha(ex[2][0])) { // checks if operand exists in map
                return findVar(ex[2]);
            }

        } else { // middle token is not = or an operator
            return 0;
        }
        return 0;
    }

    int validate5(vector<string> ex) { // var = operand op operand
        bool op1 = isUniform(ex[0]);
        bool op2 = isUniform(ex[2]);
        bool op3 = isUniform(ex[4]);
        if (!op1 || !op2 || !op3) { // var or operands are not uniform
            return 0;
        }

        if (ex[1].length() > 1 || !isEq(ex[1][0])) {
            return 0; // token 2 is not an equals sign
        }

        if (ex[3].length() > 1 || !isOp(ex[3][0])) {
            return 0; // token 4 is not an operator
        }

        if (!isAlpha(ex[0][0])) {
            return 0; // variable name is not alphabet characters
        }

        bool tok2Dig = (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1])));
        bool tok3Dig = (isDigit(ex[4][0]) || (isSigned(ex[4][0]) && isDigit(ex[4][1])));
        if (tok2Dig && tok3Dig) {
            if (ex[3][0] == '/' && stoi(ex[4]) == 0) {
                return 0; // div by 0
            }
            return 1; // tok2 and tok3 are numbers
        } else if (tok2Dig && !tok3Dig) {
            int found = findVar(ex[4]); // tok3 is a var and needs to be checked
            if (found == 1) {
                if (ex[3][0] == '/' && this->vars[ex[4]] == 0) {
                    return 0; // div by 0
                }
                return 1;
            }
            return 0;
        } else if (!tok2Dig && tok3Dig) {
            if (ex[3][0] == '/' && this->vars[ex[4]] == 0) {
                return 0; // div by 0
            }
            return findVar(ex[2]); // tok2 is a var and needs to be checked
        } else {
            if ((findVar(ex[2]) == 1) && (findVar(ex[4]) == 1)) {
                if (ex[3][0] == '/' && this->vars[ex[4]] == 0) {
                    return 0; // div by 0
                }
                return 1; // tok2 and tok3 are vars that exist in map
            }
            return 0; // either tok2 or tok3 do not exist in map
        }
        return 0;
    }

    //checks if given string consists of entirely digits or alphabet chars
    //returns true if it does, false if it does not
    bool isUniform(string tok) {
        bool good = true;
        int numOrVar = 0;
        int offset = 0;
        if (isDigit(tok[0])) { //string begins with a digit
            numOrVar = 1;
        } else if (isAlpha(tok[0])) { //string begins with a char
            numOrVar = 2;
        } else if (isSigned(tok[0])){ //string is signed
            offset++;
            //cout << "is signed\n";
            if (isDigit(tok[1])) { //string begins with a digit
                numOrVar = 1;
            } else if (isAlpha(tok[1])) { //string begins with a char
                numOrVar = 2;
                //return false;
            }
        } else {
            return false;
        }

        int len = tok.length();
        for (int i = offset; i < len; i++) {
            char c = tok[i];
            if (numOrVar == 1) {
                good = isDigit(c);
            } else if (numOrVar == 2) {
                good = isAlpha(c);
            }
            if (!good) {
                return good;
            }
        }
        return good;
    }

    // returns 1 if operand exists in map, 0 if not
    int findVar(string s) {
        return this->vars.count(s);
    }

    bool isDigit(char c) {
        return '0' <= c && c <= '9';
    }

    bool isAlpha(char c) {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    }

    bool isOp(char c) {
        return (c == '+') || (c == '-') || (c == '*') || (c == '/');
    }

    bool isEq(char c) {
        return c == '=';
    }

    bool isSigned(char c) {
        return c == '+' || c == '-';
    }

    int eval1(vector<string> ex) { //operand
        if (isDigit(ex[0][0]) || (isSigned(ex[0][0]) && isDigit(ex[0][1]))) {
            stringstream ss(ex[0]);
            int x = 0;
            ss >> x;
            return x;
        } else {
            return this->vars[ex[0]];
        }
        return 0;
    }

    int eval3(vector<string> ex) { //operand op operand || var = operand
        if (isOp(ex[1][0])) { //operand op operand
            char op = ex[1][0];
            int op1 = 0;
            int op2 = 0;
            int res;
            if (isDigit(ex[0][0]) || (isSigned(ex[0][0]) && isDigit(ex[0][1]))) {
                op1 = stoi(ex[0]);
            } else {
                op1 = this->vars[ex[0]];
            }
            if (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1]))) {
                op2 = stoi(ex[2]);
            } else {
                op2 = this->vars[ex[2]];
            }

            switch(op) {
                case '+':
                    res = op1 + op2;
                    break;
                case '-':
                    res = op1 - op2;
                    break;
                case '*':
                    res = op1 * op2;
                    break;
                case '/':
                    res = op1 / op2;
                    break;
                default:
                    res = 0;
            }
            return res;

        } else { //var = operand
            if (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1]))) {

                this->vars[ex[0]] = stoi(ex[2]);

            } else {

                this->vars[ex[0]] = this->vars[ex[2]];

            }
            return this->vars[ex[0]];

        }
    }

    int eval5(vector<string> ex) { // var = operand op operand
        char op = ex[3][0];
        int op2 = 0;
        int op3 = 0;
        int res;

        if (isDigit(ex[2][0]) || (isSigned(ex[2][0]) && isDigit(ex[2][1]))) {
            op2 = stoi(ex[2]);
        } else {
            op2 = this->vars[ex[2]];
        }
        if (isDigit(ex[4][0]) || (isSigned(ex[4][0]) && isDigit(ex[4][1]))) {
            op3 = stoi(ex[4]);
        } else {
            op3 = this->vars[ex[4]];
        }

        switch(op) {
            case '+':
                res = op2 + op3;
                break;
            case '-':
                res = op2 - op3;
                break;
            case '*':
                res = op2 * op3;
                break;
            case '/':
                res = op2 / op3;
                break;
            default:
                res = 0;
        }
        this->vars[ex[0]] = res;
        return res;
    }
};

extern "C" struct Calc *calc_create(void) {
    return new CalcImpl();
}

extern "C" void calc_destroy(struct Calc *calc) {
    CalcImpl *obj = static_cast<CalcImpl *>(calc);
    delete obj;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
    CalcImpl *obj = static_cast<CalcImpl *>(calc);
    return obj->evalExpr(expr, *result);
}
