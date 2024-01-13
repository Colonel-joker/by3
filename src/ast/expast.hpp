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
    int type;
    std::string ident;
    std::unique_ptr<BaseAST> exps;
    LValAST(const char *_name,int t) : ident(_name){type=0;} 
   
    LValAST(const char *_name, std::unique_ptr<BaseAST> &_exps,int t) : ident(_name)
    {
        exps=std::move(_exps);
        type=1;   
    } 
    IR get_koopa()  override{
        IR ret;
        switch (type)
        {
        case 0:
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


class ExpsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> exps;
  std::unique_ptr<BaseAST> exp;
    IR get_koopa() override
  {
    IR ret;
    //wait
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
                    ret.store_rid = IR::registers;
                    IR::registers++;
                    int index=IR::curbmap->find(ident)->second.index;
                    if (index!=-1)
                        ret.koopaIR="  %"+std::to_string(ret.store_rid)+" = load @"+ident+"_"+std::to_string(index)+"\n";
                    else
                        ret.koopaIR="  %"+std::to_string(ret.store_rid)+" = load %"+ident+"\n";
                    break;
                }
                case Val::CONST:{
                    ret.IRtype = IR::NUM;
                    ret.num = V.num;
                    break;
                }
                default:
                break;
                }
                    
                
            }
                
            default:
                break;
                
            }
            return ret;
           
        }
        
       
};
class FuncRParamsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> funcrparams;

  std::unique_ptr<BaseAST> exp;
   IR get_koopa() override
  {
    IR ret,ExpIR;
    ExpIR=exp->get_koopa();
    if (type==0)
    {
      IR paramsIR=funcrparams->get_koopa();
      ret.koopaIR=paramsIR.koopaIR+ExpIR.koopaIR;
      ret.funcrparams=paramsIR.funcrparams+",";
    }
    else
    {
      ret.koopaIR=ExpIR.koopaIR;
      ret.funcrparams="";
    }
    if (ExpIR.IRtype==IR::EXP||ExpIR.IRtype==IR::ARR)
    {
    ret.funcrparams+="%"+std::to_string(ExpIR.store_rid);
    }
    else if (ExpIR.IRtype==IR::NUM)
    {
    ret.funcrparams+=std::to_string(ExpIR.num);
    }
   
    
    return ret;
  }
};
class UnaryExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op,
        F1,
        F2
    } type;
    std::string op;
    std::string ident;
    std::unique_ptr<BaseAST> newexp; // PrimaryExp or UnaryExp
    std::unique_ptr<BaseAST> funcRParams;

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
    UnaryExpAST(const char *_ident) : ident(_ident)
    {
        type = F1;
    }
    UnaryExpAST(const char *_ident, std::unique_ptr<BaseAST> &rparams,int t) : ident(_ident)
    {
        funcRParams=std::move(rparams);
        type = F2;
    }
    
     IR get_koopa()override
    {   
        IR ret,pre;
        switch (type)
        {
        //ret.koopaIR+="253行type:  "+to_string(type);
        case Primary://return 0
            return newexp->get_koopa();
        case Op://return + exp
            {
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
                        ret.koopaIR += '%' + std::to_string(pre.store_rid);
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
                        ret.koopaIR += '%' + std::to_string(pre.store_rid);
                    }
                    else
                        ret.koopaIR +=  std::to_string(pre.num);
                }
                ret.koopaIR += '\n';
                ret.IRtype = IR::EXP;
                ret.store_rid = storeid;
                return ret;
            }
           
          case F1://F()
            {
                IR ret;
                if (IR::functype_map.find(ident)!=IR::functype_map.end())
                {
                    int glbtype=IR::functype_map.find(ident)->second;
                    if (glbtype==2)
                    {
                        int storeid = 0;
                        storeid = IR::registers;
                        IR::registers++;
                        ret.IRtype=IR::EXP;
                        ret.store_rid=storeid;
                        ret.koopaIR="  %"+std::to_string(storeid)+" = call @"+ident+"()\n";
                    }
                    else
                    {
                        ret.koopaIR="  call @"+ident+"()\n";
                    }
                }
                return ret;
            }
          case F2://F(x)
            {
                IR ret;
                IR paramsIR=funcRParams->get_koopa();
                ret.koopaIR=paramsIR.koopaIR;
                if (IR::functype_map.find(ident)!=IR::functype_map.end())
                {
                    int glbtype=IR::functype_map.find(ident)->second;
                    if (glbtype==2)
                    {
                        int storeid = 0;
                        storeid = IR::registers;
                        IR::registers++;
                        ret.IRtype=IR::EXP;
                        ret.store_rid=storeid;
                        ret.koopaIR+="  %"+std::to_string(storeid)+" = call @"+ident+"("+paramsIR.funcrparams+")\n";
                    }
                    else
                    {
                    ret.koopaIR+="  call @"+ident+"("+paramsIR.funcrparams+")\n";
                    }
                } 
                return ret;
            } 
       }
    }
    
    
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store_rid = storeid;
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
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
        }
        else
            ret.koopaIR = ret.koopaIR + std::to_string(l.num);
        ret.koopaIR += ", ";
        if (r.IRtype == IR::EXP)
        {
            ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
        }
        else
            ret.koopaIR = ret.koopaIR + std::to_string(r.num);
        ret.koopaIR += '\n';
        ret.IRtype = IR::EXP;
        ret.store_rid = storeid;
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store_rid = storeid;
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(l.num);
            ret.koopaIR += ", ";
            if (r.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
            }
            else
                ret.koopaIR = ret.koopaIR + std::to_string(r.num);
            ret.koopaIR += '\n';
            ret.IRtype = IR::EXP;
            ret.store_rid = storeid;
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
            IR::registers++;
            ret.koopaIR = ret.koopaIR + "  %" + std::to_string(lid) + " = ne ";
            ret.koopaIR = ret.koopaIR + std::to_string(0) + ", ";
            if (l.IRtype == IR::EXP)
            {
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
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
            ret.store_rid = storeid;
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(l.store_rid);
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
                ret.koopaIR = ret.koopaIR + '%' + std::to_string(r.store_rid);
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
            ret.store_rid = storeid;
            ret.num=l.num||r.num;
            return ret;
        }

    }
};