#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include "base_ast.hpp"
using namespace std;
//lv4

class LValAST : public BaseAST
{
  
public:
    enum 
    {
        NUM,
        ARR
    }type;
    std::string ident;
    std::vector<std::unique_ptr<BaseAST>> idx;
    LValAST(const char *_name) : ident(_name) 
    {
        type = NUM;
    }
    LValAST(const char *_name, std::vector<BaseAST*> &_idx) : ident(_name)
    {
        type = ARR;
        for(auto &i : _idx)
            idx.emplace_back(i);
            
    } 
    IR get_koopa()  override{
        IR ret;
        switch (type)
        {
        case NUM:
            ret.get_IRtype_fromVal(IR::curbmap->find(ident)->second.type);
            if(ret.IRtype==IR::ARR)
                ret.num=0;
            break;
        
        default:
            break;
        }
        return ret;
    }
};











//lv3
class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> unaryExp;

    ExpAST(std::unique_ptr<BaseAST> &_unaryExp)
    {
        unaryExp = std::move(_unaryExp);
    }
    IR get_koopa() override
    {
        return unaryExp->get_koopa();
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
            NUM,
            LVAL
        } type;
        unique_ptr<BaseAST> newexp;
        unique_ptr<BaseAST> lval;
        int number;
        PrimaryExpAST(std::unique_ptr<BaseAST> &_exp)
        {
        newexp = std::move(_exp);
        type=EXP;
        }
        PrimaryExpAST(std::unique_ptr<BaseAST> &_exp,int l)
        {
        lval = std::move(_exp);
        type=LVAL;
        }
        PrimaryExpAST(int n):number(n){
            type =NUM;
        }
        IR get_koopa()override
        {   IR ret;
            switch (type)
            {
            case EXP:{
                ret =newexp->get_koopa();
                ret.IRtype=IR::EXP;
                return ret;
                break;
            }
            case NUM:{
                ret.IRtype=IR::NUM;
                ret.num=number;
                return ret;
                break;
            }
            case LVAL:{
                LValAST* var=(LValAST*)(lval.get());
                std::string ident=var->ident;
                Val V=IR::curbmap->find(ident)->second;
                switch (V.type)
                {
                case Val::VAR:{
                    ret.IRtype = IR::EXP;
                    ret.num = V.num;
                    ret.store = IR::registers;
                    IR::registers++;
                    int index=IR::curbmap->find(ident)->second.index;
                    if (index!=-1)
                        ret.koopaIR="  %"+std::to_string(ret.store)+" = load @"+ident+"_"+std::to_string(index)+"\n";
                    else
                        ret.koopaIR="  %"+std::to_string(ret.store)+" = load %"+ident+"\n";
                    break;
                }
                    
                case Val::CONST:{
                    ret.IRtype = IR::NUM;
                    ret.num = V.num;
                    break;
                }
                default://数组
                    IR::arraydefing=0;
                    break;
                }
                break;
            }
                
            default:
                break;
                
            }
            return ret;
           
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
     IR get_koopa()override
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
        Primary,//UnaryExp | 
        Op //MulExp ("*" | "/" | "%") UnaryExp;
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
    IR get_koopa()override{
        IR ret;
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            
            IR l = leftExp->get_koopa();
            IR r = rightExp->get_koopa();
            ret.koopaIR = l.koopaIR;
            ret.koopaIR += r.koopaIR;
            int storeid = 0;
            storeid = IR::registers;
            // if (l.IRtype&&r.IRtype)
            // {
            //     ret.IRtype = IR::NUM;
            //     ret.store = storeid;
            //     if (op=="*")
            //     {
            //     ret.num=l.num*r.num;
            //     }
            //     else if(op=="/"){
            //     ret.num=l.num/r.num;
            //     }
            //     else
            //     {
            //     ret.num=l.num%r.num;
            //     }
            //     return ret;
            // }
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid);
            if (op == "*")
            {
                ret.koopaIR += " = mul ";
            }
            else if (op == "/")
                ret.koopaIR += " = div ";
            else
                ret.koopaIR += " = mod ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store = storeid;
            if (op=="*")
            {
            ret.num=l.num*r.num;
            }
            else if(op=="/"){
            ret.num=l.num/r.num;
            }
            else
            {
            ret.num=l.num%r.num;
            }
           
        }
        return ret;
    }
   
   
};


class AddExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;  //MulExp | AddExp ("+" | "-") MulExp;
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
    IR get_koopa()override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//
            IR ret,l,r;
            l=leftExp->get_koopa();
            r=rightExp->get_koopa();
            ret.koopaIR=l.koopaIR;
            ret.koopaIR+=r.koopaIR;
            int storeid =IR::registers;
            // if (l.IRtype&&r.IRtype)//都不是表达式
            //  {
            //     ret.IRtype = 1;
            //     ret.store = storeid;
            //     if (op=="+")
            //     {
            //          ret.num=l.num+r.num;
            //     }
            //     else{
            //         ret.num=l.num-r.num;
            //     }
            //     return ret;
            // }
        IR::registers++;
        ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid);
        if (op == "+")
        {
            ret.koopaIR += " = add ";
        }
        else
            ret.koopaIR += " = sub ";
        if (l.IRtype == IR::EXP)
        {
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
        }
        else
            ret.koopaIR = ret.koopaIR + std::to_string(l.num);
        ret.koopaIR += ", ";
        if (r.IRtype == IR::EXP)
        {
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
        }
        else
            ret.koopaIR = ret.koopaIR + std::to_string(r.num);
        ret.koopaIR += '\n';
        ret.IRtype = IR::EXP;
        ret.store = storeid;
        if (op=="+")
        {
            ret.num=l.num+r.num;
        }
        else{
            ret.num=l.num-r.num;
        }
        return ret;
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
    IR get_koopa() override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            IR ret;
            IR l = leftExp->get_koopa();
            IR r = rightExp->get_koopa();
            ret.koopaIR = l.koopaIR;
            ret.koopaIR += r.koopaIR;
            int storeid = 0;
            storeid = IR::registers;
            // if (l.IRtype&&r.IRtype)
            // {
            //     ret.IRtype = 1;
            //     ret.store = storeid;
            //     if (op == "<=")
            //     {
            //     ret.num = l.num<=r.num;
            //     }
            //     else if (op == ">=")
            //     ret.num = l.num>=r.num;
            //     else if (op == "<")
            //     ret.num=l.num<r.num;
            //     else
            //     ret.num=l.num>r.num;
            //     return ret;
            // }
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid);
            if (op == "<=")
            {
                ret.koopaIR += " = le ";
            }
            else if (op == ">=")
                ret.koopaIR += " = ge ";
            else if (op == "<")
                ret.koopaIR += " = lt ";
            else
                ret.koopaIR += " = gt ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store = storeid;
            if (op == "<=")
            {
                ret.num = l.num<=r.num;
            }
            else if (op == ">=")
                ret.num = l.num>=r.num;
            else if (op == "<")
                ret.num=l.num<r.num;
            else
                ret.num=l.num>r.num;

            return ret;
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
    IR get_koopa()override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            IR ret;
            IR l = leftExp->get_koopa();
            IR r = rightExp->get_koopa();
            ret.koopaIR = l.koopaIR;
            ret.koopaIR += r.koopaIR;
            int storeid = 0;
            storeid = IR::registers;
            // if (l.IRtype&&r.IRtype)
            // {
            //     ret.IRtype = 1;
            //     ret.store = storeid;
            //     if (op=="==")
            //     {
            //     ret.num=l.num==r.num;
            //     }
            //     else
            //     {
            //     ret.num=l.num!=r.num;
            //     }
            //     return ret;
            // }
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid);
            if (op == "==")
            {
                ret.koopaIR += " = eq ";
            }
            else
                ret.koopaIR += " = ne ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store = storeid;
            if (op=="==")
            {
                ret.num=l.num==r.num;
            }
            else
            {
                ret.num=l.num!=r.num;
            }
        return ret;
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
    IR get_koopa()override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            IR ret,l,r;
            l = leftExp->get_koopa();
            r = rightExp->get_koopa();
            ret.koopaIR = l.koopaIR;
            ret.koopaIR += r.koopaIR;
            int storeid = 0;
            int lid, rid;
            lid = IR::registers;
            // if (l.IRtype&&r.IRtype)
            // {
            //     ret.IRtype = 1;
            //     ret.store = storeid;
            //     ret.num=l.num&&r.num;
            //     return ret;
            // }
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(lid) + " = ne ";
            ret.koopaIR = ret.koopaIR + std::to_string(0) + ", ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += '\n';
            rid = IR::registers;
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(rid) + " = ne ";
            ret.koopaIR = ret.koopaIR + std::to_string(0) + ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            storeid = IR::registers;
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid) + " = and ";
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(lid);
            ret.koopaIR += ", ";
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(rid);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store = storeid;
            ret.num=l.num&&r.num;
            return ret;
        }

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
    IR get_koopa()override{
        if(type==Primary){
            return leftExp->get_koopa();
        }
        else if(type==Op){//待定
            IR ret,l,r;
            l = leftExp->get_koopa();
            r = rightExp->get_koopa();
            ret.koopaIR = l.koopaIR;
            ret.koopaIR += r.koopaIR;
            int storeid = 0;
            int lid, rid;
            lid = IR::registers;
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(lid) + " = ne ";
            ret.koopaIR = ret.koopaIR + std::to_string(0) + ", ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += '\n';
            rid = IR::registers;
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(rid) + " = ne ";
            ret.koopaIR = ret.koopaIR + std::to_string(0) + ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            storeid = IR::registers;
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(storeid) + " = or ";
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(lid);
            ret.koopaIR += ", ";
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(rid);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store = storeid;
            ret.num=l.num||r.num;
            return ret;
        }

    }
};