#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include "base_ast.hpp"
using namespace std;
class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> unaryExp;

    ExpAST(std::unique_ptr<BaseAST> &_unaryExp)
    {
        unaryExp = std::move(_unaryExp);
    }
    IR get_koopa()const override
    {
        return unaryExp->get_koopa();
    }
    int Cal() const override
    {
        return unaryExp->Cal();
    }
    // void DumpAST()const override{
    //     unaryExp->DumpAST();
    // }
};
class PrimaryExpAST:public BaseAST
{
        public:
            enum
        {
            EXP,
            NUM
        } type;
        unique_ptr<BaseAST> newexp;
        int number;
        PrimaryExpAST(std::unique_ptr<BaseAST> &_exp)
        {
        newexp = std::move(_exp);
        type=EXP;
        }
        PrimaryExpAST(int n):number(n){
            type =NUM;
            }
        IR get_koopa()const override
        {   IR ret;
            switch (type)
            {
            case EXP:
                 ret =newexp->get_koopa();
                 ret.IRtype=IR::EXP;
                return ret;
                break;
            case NUM:
                ret.IRtype=IR::NUM;
                ret.num=number;
                return ret;
              break;
            default:
                break;
            
            }
           
        }
          int Cal() const override
        {
            return newexp->Cal();
        }
        // void DumpAST()const override{
        //      newexp->get_koopa();
        // }
};

class UnaryExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op,
      //  Function
    } type;
    std::string op;
    std::unique_ptr<BaseAST> newexp; // PrimaryExp or UnaryExp
    std::vector<BaseAST*> funcRParams;

    UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        newexp = std::move(_primary_exp);
    }
    UnaryExpAST(const char *_op, std::unique_ptr<BaseAST> &_unary_exp)
    {
        type = Op;
        op = std::string(_op);
        newexp = std::move(_unary_exp);
    }
   // UnaryExpAST(const char *_ident, std::vector<BaseAST*> &rparams) : op(_ident), funcRParams(rparams)
   // {
   //     type = Function;
    //}
     IR get_koopa()const override
    {   
        IR ret,pre;
        switch (type)
        {
        case Primary://return 0
            return newexp->get_koopa();
            break;
        case Op://return + exp
            
            pre=newexp->get_koopa();
            ret.koopaIR+=pre.koopaIR;
            int storeid=IR::registers;
            switch (op[0])
            {
            case '+':
                ret.num=pre.num;
                break;
            case '-':
                ret.num=-pre.num;
                break;
            case '!':
                ret.num=!pre.num;
                break;    
            default:
                break;
            }
            
           //if(pre.IRtype){//return +2  ->ret 2        
         //      ret.IRtype=IR::NUM;
          //     ret.store=storeid;
               
          //     return ret;
        //   }
        //   else{//return +(-2) ->  ret 
                IR::registers++;
                ret.koopaIR += "  %" + std::to_string(storeid);
                switch (op[0])
                {
                case '+':
                    return pre;  //忽略该%
                    break;
                case '-':
                    ret.koopaIR += " = sub ";
                    break;
                case '!':
                    ret.koopaIR += " = eq ";
                    break;    
                default:
                    break;
                }
                //+ 已经return
                if (op[0] == '!')
                {
                    if (pre.IRtype == IR::EXP)
                    {
                        ret.koopaIR += '%' + std::to_string(pre.store);
                    }
                    else
                        ret.koopaIR += std::to_string(pre.num);
                    ret.koopaIR += ", ";
                    ret.koopaIR +=  std::to_string(0);
                }
                else if(op[0] == '-'){
                    ret.koopaIR += std::to_string(0);
                    ret.koopaIR += ", ";
                     if (pre.IRtype == IR::EXP)
                    {
                        ret.koopaIR += '%' + std::to_string(pre.store);
                    }
                    else
                        ret.koopaIR +=  std::to_string(pre.num);
                }
                ret.koopaIR += '\n';
                ret.IRtype = IR::EXP;
                ret.store = storeid;
                return ret;
      //      }
    
       }
    }
    
    
     int Cal() const override
    {
        if (type == Primary)
            return newexp->Cal();
        int res = 0;
        if (op == "+")
            res = newexp->Cal();
        else if (op == "-")
            res = -newexp->Cal();
        else if (op == "!")
            res = !newexp->Cal();
        return res;
    }
    // void DumpAST()const override
    // {
    //     newexp->get_koopa();
    // }
};

class MulExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    MulExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    MulExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            IR ret;
            ret.koopaIR+=op;
            return ret=rightExp->get_koopa();
        }
    }
     int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "*")
            res = leftExp->Cal() * rightExp->Cal();
        else if (op == "/")
            res = leftExp->Cal() / rightExp->Cal();
        else if (op == "%")
            res = leftExp->Cal() % rightExp->Cal();
        return res;
    }
   
};


class AddExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    AddExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    AddExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            leftExp->get_koopa();
        }
        else if(type==Op){//待定
            cout<<op;
            rightExp->get_koopa();
        }
    }
   int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "+")
            res = leftExp->Cal() + rightExp->Cal();
        else if (op == "-")
            res = leftExp->Cal() - rightExp->Cal();
        return res;
    }
};

class RelExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    RelExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    RelExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            leftExp->get_koopa();
        }
        else if(type==Op){//待定
            cout<<op;
            rightExp->get_koopa();
        }
    }
     int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "<")
            res = leftExp->Cal() < rightExp->Cal();
        else if (op == "<=")
            res = leftExp->Cal() <= rightExp->Cal();
        else if (op == ">")
            res = leftExp->Cal() > rightExp->Cal();
        else if (op == ">=")
            res = leftExp->Cal() >= rightExp->Cal();
        return res;
    }
};


class EqExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    EqExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    EqExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            leftExp->get_koopa();
        }
        else if(type==Op){//待定
            cout<<op;
            rightExp->get_koopa();
        }
    }
    int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "==")
            res = leftExp->Cal() == rightExp->Cal();
        else if (op == "!=")
            res = leftExp->Cal() != rightExp->Cal();
        return res;
    }
};

class LAndExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    LAndExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    LAndExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            leftExp->get_koopa();
        }
        else if(type==Op){//待定
            cout<<op;
            rightExp->get_koopa();
        }
    }
   int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "&&")
            res = leftExp->Cal() && rightExp->Cal();
        return res;
    }
};

class LOrExpAST : public BaseAST
{
    public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    LOrExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    LOrExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }
    IR get_koopa()const override{
        if(type==Primary){
            leftExp->get_koopa();
        }
        else if(type==Op){//待定
            leftExp->get_koopa();
            cout<<op;
            rightExp->get_koopa();
        }
    }
     int Cal() const override
    {
        if (type == Primary)
            return leftExp->Cal();
        int res = 0;
        if (op == "||")
            res = leftExp->Cal() || rightExp->Cal();
        return res;
    }
};