#pragma once
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "node.hpp"

using namespace std;

class Expression:public Node{
public:
    virtual void genCode(FILE *file) = 0;
};

class DecimalIntegerLiteralExpression:public Expression{
private:
    int value;
public:
    DecimalIntegerLiteralExpression(int value){
        this->value = value;
    };

    int getValue() {
        return value;
    }

    void dump(int indent){
        label(indent, "IntegerLiteralExpression: %d\n", value);
    }
    bool resolveNames(LexicalScope* scope) {
        return true;
    }

    void genCode(FILE *file) {
        int regInd = this->registerIndex++;
        emit(file, "ESValue* r%d = new Number(%d);", regInd, this->value);
    }
};

class DecimalLiteralExpression:public Expression{
private:
    double value;
public:
    DecimalLiteralExpression(double value){
        this->value = value;
    };

    double getValue() {
        return value;
    }

    void dump(int indent){
        label(indent, "IntegerLiteralExpression: %d\n", value);
    }
    bool resolveNames(LexicalScope* scope) {
        return true;
    }

    void genCode(FILE *file) {

    }
};

class IdentifierExpression:public Expression{
private:
    std::string name;
    Reference* reference;
public:
    IdentifierExpression(std::string name){
        this->name = name;
        this->reference = NULL;
    };

    std::string getReferencedName() {
        return name;
    }

    void dump(int indent){
        label(indent, "IdentifierExpression: %s\n", name.c_str());
    }
    bool resolveNames(LexicalScope* scope) {
        if (scope != NULL) {
            reference = scope->resolve(name);
        }
        if (reference == NULL) {
            fprintf(stderr, "Error: Undeclared identifier: %s\n", name.c_str());
        }
        return reference != NULL;
    }

    void genCode(FILE *file) {
        int regInd = this->registerIndex++;
        emit(file, "ESValue* r%d = new ReferenceType(\"%s\")", regInd, name.c_str());
    }
};

class StringLiteralExpression: public Expression {
private:
	std::string val;
public:
	StringLiteralExpression(const char* val) {
		this->val = std::string(val);
	};

    std::string getValue() {
        return val;
    }

	void dump(int indent) {
		label(indent, "StringLiteralExpression: %s\n", val.c_str());
	}
    bool resolveNames(LexicalScope* scope) {
        return true;
    }

    void genCode(FILE *file) {

    }
};

class AssignmentExpression:public Expression, Reference {
private:
    Expression *lhs, *rhs;
public:
    AssignmentExpression(Expression *lhs, Expression *rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    };

    AssignmentExpression(Expression* expression){
        this->lhs = expression;
    }

    void dump(int indent) {
        label(indent, "AssignmentExpression\n");
        lhs->dump(indent + 1, "lhs");
        if(rhs != NULL){
            rhs->dump(indent + 1, "rhs");
        }
    }

    /**
     * Returns the base value component of the reference
     * TODO: This method will need to be expanded for all possible types... maybe?
     */
    ESValue* getBase() {

        // attempt to cast to a string
        StringLiteralExpression* stringLiteralExpression = dynamic_cast<StringLiteralExpression*>(rhs);
        if (stringLiteralExpression) {
            return new String(stringLiteralExpression->getValue());
        }

        // attempt to cast to an int
        DecimalIntegerLiteralExpression* decimalIntegerLiteralExpression = dynamic_cast<DecimalIntegerLiteralExpression*>(rhs);
        if (decimalIntegerLiteralExpression) {
            return new Number(decimalIntegerLiteralExpression->getValue());
        }

        // attempt to cast to a double
        DecimalLiteralExpression* decimalLiteralExpression = dynamic_cast<DecimalLiteralExpression*>(rhs);
        if (decimalLiteralExpression) {
            return new Number(decimalLiteralExpression->getValue());
        }

        // ??? fail!
        return new Undefined();
    }

    /**
     * Returns the referenced name component of the reference.
     */
    std::string getReferencedName() {
        IdentifierExpression *identifier = dynamic_cast<IdentifierExpression *> (lhs);
        if (identifier != NULL) {
            return identifier->getReferencedName();
        }
        return NULL;
    }

    /**
     * Returns the strict reference flag component of the reference.
     */
    bool isStrictReference() {
        return false;
    }

    /**
     * Returns true if Type(base) is Boolean, String, Symbol, or Number.
     */
    bool hasPrimitiveBase() {
        return getBase()->isPrimitive();
    }

    /**
     * Returns true if either the base value is an object or hasPrimitiveBase() is true; otherwise returns false.
     */
    bool isPropertyReference() {
        if (getBase()->getType() == object || getBase()->isPrimitive()) {
            return true;
        }
        return false;
    }

    /**
     * Returns true if the base value is undefined and false otherwise.
     */
    bool isUnresolvableReference() {
        return getBase()->getType() == undefined;
    }

    /**
     * Returns true if this reference has a thisValue component
     */
    bool isSuperReference() {
        return false;
    }

    bool resolveNames(LexicalScope *scope) {
        if (lhs && rhs) {
//        return true;
            IdentifierExpression *identifier = dynamic_cast<IdentifierExpression *> (lhs);
            if (identifier != NULL) {
                // not sure `this` in this context is what we are after, but whatever
                scope->addToSymbolTable(identifier->getReferencedName(), this);
            }
            return lhs->resolveNames(scope) && rhs->resolveNames(scope);
        }
        return false;
    }

    void genCode(FILE *file) {
        int regInd = this->registerIndex++;

        std::string idenName = getReferencedName();
        emit(file, "ESValue* r%d = new ReferenceType(\"%s\")", regInd, idenName.c_str());
        this->rhs->genCode(file);

        int regIndAfer = this->registerIndex - 1;
        emit(file, "ESValue* r%d = Core::Asign(r%d, r%d)", this->registerIndex++, regInd, regIndAfer);
    }
};

class ObjectLiteralExpression : public Expression {
private:
    vector<Expression*> *propertyDefinitionList;
public:
    //No parameter constructor
    ObjectLiteralExpression(){};
    ObjectLiteralExpression(vector<Expression*> *propertyDefinitionList) {
        this->propertyDefinitionList = propertyDefinitionList;
    };

    void dump(int indent) {
        label(indent, "ObjectLiteralExpression\n");

        if(propertyDefinitionList != NULL) {
            for (vector<Expression*>::iterator iter = propertyDefinitionList->begin(); iter != propertyDefinitionList->end(); ++iter)
                (*iter)->dump(indent+1);
        }
    }

    bool resolveNames(LexicalScope* scope) {
        bool scoped = true;
        if (propertyDefinitionList) {
            for (vector<Expression *>::iterator it = propertyDefinitionList->begin();
                 it != propertyDefinitionList->end(); ++it) {
                if (*it) {
                    if (!(*it)->resolveNames(scope)) {
                        scoped = false;
                    }
                }
            }
            return scoped;
        }
        return false;
    }

    void genCode(FILE *file) {

    }
};

class PropertyDefinitionExpression : public Expression {
private:
    Expression *key;
    Expression *value;

public:
    PropertyDefinitionExpression(Expression *key, Expression *value) {
        this->key = key;
        this->value = value;
    };

    void dump(int indent) {
        label(indent, "PropertyDefinitionExpression\n");
        indent++;
        label(indent, "Key\n");
        key->dump(indent + 1);
        label(indent, "Value\n");
        if (value != NULL) {
            value->dump(indent + 1);
        } else {
            label(indent + 1, "[UNDEFINED]\n");
        }
    }

    bool resolveNames(LexicalScope *scope) {
        if (key && value) {
            return key->resolveNames(scope) && value->resolveNames(scope);
        }
        return false;
    };

    void genCode(FILE *file) {

    }
};

class LiteralPropertyNameExpression : public Expression {
private:
    Expression *literalExpression;
public:
    LiteralPropertyNameExpression(Expression *literalExpression) {
        this->literalExpression = literalExpression;
    };

    void dump (int indent) {
        label(indent, "LiteralPropertyNameExpression\n");
        indent++;
        literalExpression->dump(indent);
    }

    bool resolveNames(LexicalScope* scope) {
        if (literalExpression) {
            return literalExpression->resolveNames(scope);
        }
        return false;
    }

    void genCode(FILE *file) {

    }
};

class ComputedPropertyNameExpression : public Expression {
private:
    Expression *computedExpression;
public:
    ComputedPropertyNameExpression(Expression *computedExpression) {
        this->computedExpression = computedExpression;
    };

    void dump(int indent) {
        label(indent, "ComputedPropertyNameExpression\n");
        indent++;
        computedExpression->dump(indent);
    }

    bool resolveNames(LexicalScope *scope) {
        if (computedExpression) {
            return computedExpression->resolveNames(scope);
        }
        return false;
    }

    void genCode(FILE *file) {

    }
};

class Arguments : public Expression {
private:
    std::vector<AssignmentExpression*>* argumentList;
public:
    Arguments(vector<AssignmentExpression*>* argumentList){
        this->argumentList = argumentList;
    };

    void dump(int indent) {
        label(indent, "Arguments\n");
        for (vector<AssignmentExpression*>::iterator iter = argumentList->begin(); iter != argumentList->end(); ++iter)
          (*iter)->dump(indent+1);
    }
    
  bool resolveNames(LexicalScope* scope) {
    return true;
  }

  void genCode(FILE *file) {

    }
};


class CallExpression : public Expression {
private:
    Expression* expression;
    Arguments* arguments;
public:
    CallExpression(Expression *expression) {
        this->expression = expression;
    };

    void dump(int indent) {
        label(indent, "CallExpression\n");
        indent++;
        expression->dump(indent);
        if(arguments != NULL) {
            arguments->dump(indent+1);
        }
    }

    bool resolveNames(LexicalScope *scope) {
        if (expression) {
            return expression->resolveNames(scope);
        }
        return false;
    }

    void genCode(FILE *file) {

    }
};

class BinaryExpression : public Expression {
private:
    Expression* lhs;
    Expression* rhs;
    char op;
public:
    BinaryExpression(Expression* lhs, Expression* rhs, char op) {
        this->lhs = lhs;
        this->rhs = rhs;

        this->op = op;
    };

    bool resolveNames(LexicalScope *scope) {
        if (lhs && rhs) {
            return lhs->resolveNames(scope) && rhs->resolveNames(scope);
        }
        return false;
    }

    void genCode(FILE *file) {

        //rhs must genCode() first, otherwise, the output will be wrong
        this->rhs->genCode(file);
        this->lhs->genCode(file);

        int regInd = this->registerIndex++;

        //Becareful with Substract, Divide and Modulo.
        int rRegInd = regInd - 2;
        int lRegInd = regInd - 1;

        switch(op) {
            case '+':
                emit(file, "ESValue* r%d = Core::Plus(r%d, r%d);", regInd, lRegInd, rRegInd);
                break;
            case '-':
                emit(file, "ESValue* r%d = Core::Substract(r%d, r%d);", regInd, lRegInd, rRegInd);
                break;
            case '*':
                emit(file, "ESValue* r%d = Core::Multiply(r%d, r%d);", regInd, lRegInd, rRegInd);
                break;
            case '/':
                emit(file, "ESValue* r%d = Core::Divide(r%d, r%d);", regInd, lRegInd, rRegInd);
                break;
            case '%':
                emit(file, "ESValue* r%d = Core::Modulo(r%d, r%d);", regInd, lRegInd, rRegInd);
                break;
        }
    }

    void dump(int indent) {
        label(indent, "BinaryExpression\n");
        label(indent + 1, "op: %c\n", op);
        lhs->dump(indent + 1, "lhs");
        if(rhs != NULL){
            rhs->dump(indent + 1, "rhs");
        }
    }
};