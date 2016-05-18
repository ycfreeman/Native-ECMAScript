#pragma once
#include <cstdarg>
#include <cstdio>
#include "node.hpp"

using namespace std;

class Expression:public Node{
	 
	
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
  
    void GenCode(FILE* file)
	{
		
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
  
    void GenCode(FILE* file)
	{
		
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
   
    
    void GenCode(FILE* file)
	{
		
	}
	
	void GenStoreCode(FILE* file)
	{
		emit(file, "stloc %d", reference->getReferencedName());
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
   
    
    void GenCode(FILE* file)
	{
		
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

    void dump(int indent) {
        label(indent, "AssignmentExpression\n");
        lhs->dump(indent + 1, "lhs");
        rhs->dump(indent + 1, "rhs");
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

   
    
     void GenStoreCode(FILE* file) 	{
		emit(file, "stloc %d", getBase());
	}
   
    
    void GenCode(FILE* file) {
    	rhs->GenCode(file);
	//	lhs->GenStoreCode(file);
		lhs->GenCode(file);
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

   
    
    void GenCode(FILE* file)
	{
		
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

    
    
    void GenCode(FILE* file)
	{
		
	}
	
	
};

class LiteralPropertyNameExpression : public Expression {
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

   
    
    void GenCode(FILE* file)
	{
		
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

  
    
    void GenCode(FILE* file)
	{
		
	}

};