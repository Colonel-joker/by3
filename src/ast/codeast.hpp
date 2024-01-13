#pragma once

#include <unordered_map>

#include "ast/base_ast.hpp"
#include "koopa.h"

class GlobalVarDefAST : public BaseAST
{
public:
    std::string name;

    std::unique_ptr<BaseAST> exp;

};
class FuncFParamAST : public BaseAST
{
public:
    enum ParamType
    {
        Int,
        Array
    } type;
    std::string name;
    int index;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;

    FuncFParamAST(ParamType _type, const char *_name, int _index) : type(_type), name(_name), index(_index) {}
    FuncFParamAST(ParamType _type, const char *_name, int _index, std::vector<BaseAST*> &_sz_Exp) : type(_type), name(_name), index(_index)
    {
        for(auto e : _sz_Exp)
            sz_exp.emplace_back(e);
    }
};


class BTypeAST : public BaseAST
{
public:
    std::string type;
    BTypeAST(const char *_name) : type(_name) {}
    IR get_koopa() override{
      IR ret;
      ret.koopaIR=type;
      return ret;
    }
};

//lv7完成
class FuncTypeAST : public BaseAST
{
public:
    std::string type;
    FuncTypeAST(const char *_name) : type(_name) {}
    IR get_koopa() override{
      IR ret;
      ret.koopaIR=type;
      return ret;
    }
};

class DeclAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> constdecl;
  std::unique_ptr<BaseAST> vardecl;
   IR get_koopa() override
  {
    if (type==0)
      return constdecl->get_koopa();
    else 
      return vardecl->get_koopa();
  }
};


class ConstDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> btype;
  std::unique_ptr<BaseAST> constdefs;

  IR get_koopa() override
  { IR ret;
   // ret.koopaIR=btype->get_koopa().koopaIR;
    ret.koopaIR+=constdefs->get_koopa().koopaIR;
    return ret;
  }
};

//constdefs ok
class ConstDefsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> constdefs;
  std::unique_ptr<BaseAST> constdef;
  ConstDefsAST(){}
  IR get_koopa() override
  {
    IR ret;
    if (type==0)
      ret.koopaIR=constdefs->get_koopa().koopaIR+constdef->get_koopa().koopaIR;
    else
      ret.koopaIR=constdef->get_koopa().koopaIR;
    return ret;
  }
};

class ConstDefAST : public BaseAST
{
public:
  enum{
    SIN,//IDENT '=' ConstInitVal
    MUL//IDENT ConstExpBlocks '=' ConstInitVal
  }type;
    std::string ident;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> constexpblocks;
    ConstDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp): ident(_name)
    {
        exp = std::move(_exp);
        type=SIN;
    }
    IR get_koopa()  override {
      IR ret;
      switch (type)
      {
      case SIN:{
        IR::constdefing=1;
        IR ExpIR=exp->get_koopa();
        int index=1;
        if (IR::constmap.find(ident)==IR::constmap.end())
        {
          IR::constmap.emplace(ident,1);
        }
        else
        {
          index=IR::constmap.find(ident)->second+1;
          IR::constmap[ident]=index;
        }
        if (IR::globaldef==1)
        {
          IR::curbmap->umap.emplace(ident,Val(Val::CONST,ExpIR.num,0));
        }
        else
        {
          IR::curbmap->umap.emplace(ident,Val(Val::CONST,ExpIR.num,index));
        }
        IR::constdefing=0;

        break;
      }
      default:
        break;
      }
      return ret;
    }
};
//我在sysy.y中没有写vardef,所以gdb找不到函数
class VarDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> btype;
  std::unique_ptr<BaseAST> vardefs;
 IR get_koopa() override
  {
    return vardefs->get_koopa();
  }
};

class VarDefsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> vardefs;
  std::unique_ptr<BaseAST> vardef;
  IR get_koopa() override
    {
      IR ret;
      if (type==0)
        ret.koopaIR=vardefs->get_koopa().koopaIR+vardef->get_koopa().koopaIR;
      else
        ret.koopaIR=vardef->get_koopa().koopaIR;
      return ret;
    }
};

class VarDefAST : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> exp;
    int type;
    VarDefAST(){}
    VarDefAST(const char *_name,int t)
        : ident(_name),type(t)
    {
      exp = nullptr;
    }
    VarDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp,int t)
        : ident(_name),type(t)
    {
      exp = std::move(_exp);
    }
     IR get_koopa() override
  {
    IR ret;
    switch (type)
    {
    case 0://IDENT
      {
      ret.IRtype=IR::NUM;
      ret.store_rid=0;
      ret.num=0;
      int index=1;
      if (IR::constmap.find(ident)==IR::constmap.end())
      {
        IR::constmap.emplace(ident,1);
      }
      else
      {
        index=IR::constmap.find(ident)->second+1;
        IR::constmap[ident]=index;
      }
      if (IR::globaldef==1)
      {
        IR::curbmap->umap.emplace(ident,Val(0,0,0));
        ret.koopaIR="global @"+ident+"_"+std::to_string(0)+" = alloc i32, ";
        ret.koopaIR+="zeroinit";
        ret.koopaIR+="\n";
      }
      else
      {
        IR::curbmap->umap.emplace(ident,Val(0,0,index));
        ret.koopaIR="  @"+ident+"_"+std::to_string(index)+" = alloc i32\n";
        ret.koopaIR+="  store 0, @"+ident+"_"+std::to_string(index)+"\n";
      }
      }
      break;
    case 1://| IDENT '=' exp {
      {
    
      IR::constdefing=1;
      IR ExpIR=exp->get_koopa();
     
      int index=1;
      if (IR::constmap.find(ident)==IR::constmap.end())
      {
        IR::constmap.emplace(ident,1);
      }
      else
      {
        index=IR::constmap.find(ident)->second+1;
        IR::constmap[ident]=index;
      }
      if (IR::globaldef==1)
      {
        IR::curbmap->umap.emplace(ident,Val(0,ExpIR.num,0));
        ret.koopaIR="global @"+ident+"_"+std::to_string(0)+" = alloc i32, ";
        if (ExpIR.num!=0)
          ret.koopaIR+=std::to_string(ExpIR.num);
        else
          ret.koopaIR+="zeroinit";
        ret.koopaIR+="\n";
      }
      else
      {
        IR::curbmap->umap.emplace(ident,Val(0,ExpIR.num,index));
        ret.koopaIR="  @"+ident+"_"+std::to_string(index)+" = alloc i32\n";
        ret.koopaIR+=ExpIR.koopaIR;
        if (ExpIR.IRtype==IR::EXP)
        ret.koopaIR+="  store %"+std::to_string(ExpIR.store_rid)+", @"+ident+"_"+std::to_string(index)+"\n";
        else
        ret.koopaIR+="  store "+std::to_string(ExpIR.num)+", @"+ident+"_"+std::to_string(index)+"\n";
      }
      IR::constdefing=0;
      }

    default:
      break;
    }
     return ret;
    
    
  }
};

class CompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> comp_units;
    IR get_koopa()  override 
    {
      IR ret;
      ret.koopaIR="decl @getint(): i32\n";
      IR::functype_map.emplace("getint",2);
      ret.koopaIR+="decl @getch(): i32\n";
      IR::functype_map.emplace("getch",2);
      ret.koopaIR+="decl @getarray(*i32): i32\n";
      IR::functype_map.emplace("getarray",2);
      ret.koopaIR+="decl @putint(i32)\n";
      IR::functype_map.emplace("putint",3);
      ret.koopaIR+="decl @putch(i32)\n";
      IR::functype_map.emplace("putch",3);
      ret.koopaIR+="decl @putarray(i32, *i32)\n";
      IR::functype_map.emplace("putarray",3);
      ret.koopaIR+="decl @starttime()\n";
      IR::functype_map.emplace("starttime",3);
      ret.koopaIR+="decl @stoptime()\n\n";
      IR::functype_map.emplace("stoptime",3);
    ret.koopaIR+=comp_units->get_koopa().koopaIR;
    return ret;
    }
};

class CompUnitsAST : public BaseAST
{
public:
 
  int type;
  std::unique_ptr<BaseAST> comp_units;
  std::unique_ptr<BaseAST> global_def;
  
  CompUnitsAST(){}
  CompUnitsAST(std::unique_ptr<BaseAST>&g){
      global_def=std::move(g);
      type=0;
  }
  CompUnitsAST(std::unique_ptr<BaseAST>&c,std::unique_ptr<BaseAST>&g){
      comp_units=std::move(c);
      global_def=std::move(g);
      type=1;
  }
   IR get_koopa() override
  {
    if (type==0)
    {
      IR ret=comp_units->get_koopa();
      IR::registers=0;
      IR::constmap=std::unordered_map<std::string, int>();
      IR func=global_def->get_koopa();
      ret.koopaIR=ret.koopaIR+func.koopaIR;
      return ret;
    }
    else
    return global_def->get_koopa();
  }
};


class GlobalDefAST : public BaseAST
{
public:
  // 用智能指针管理对象
  int type;
  std::unique_ptr<BaseAST> func_def;
  std::unique_ptr<BaseAST> decl;
  GlobalDefAST(){}
  IR get_koopa() override
  {
     if (type==0)
    return func_def->get_koopa();
    else
    {
      IR::globaldef=1;
      IR dec=decl->get_koopa();
      IR::globaldef=0;
      return dec;
    }
    
  }
};



class FuncParamsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> funcparams;
  std::unique_ptr<BaseAST> funcparam;
  IR get_koopa() override
  {
    IR ret;
    if (type==0)
      ret.koopaIR=funcparams->get_koopa().koopaIR+","+funcparam->get_koopa().koopaIR;
    else
      ret.koopaIR=funcparam->get_koopa().koopaIR;
    return ret;
  }
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> func_params;
  blockmap bmap;
  int type;  
  FuncDefAST(){}
  IR get_koopa()  override {
    bmap.umap=std::unordered_map<std::string, Val>();
    bmap.parent=IR::curbmap;
    IR::curbmap=&bmap;//初始化curbmap

    IR ret=func_type->get_koopa();
    std::string ftype =ret.koopaIR;
    ret.koopaIR= "fun @" + ident +"(";
    if (type==1)
    {
      ret.koopaIR=ret.koopaIR+func_params->get_koopa().koopaIR;
    }
    ret.koopaIR += ")";
    if (ftype == "int")
    {
      ret.koopaIR += ": i32";
      IR::functype_map.emplace(ident,2);
    }
    else
      IR::functype_map.emplace(ident,3);
    ret.koopaIR += " {\n";
    ret.koopaIR += "%entry:\n";
     std::unordered_map<std::string, Val>::iterator it;
    for (it=bmap.umap.begin();it!=bmap.umap.end();it++)
    {
      ret.koopaIR+="  %"+it->first+" = alloc ";
      ret.koopaIR+="i32\n";
      ret.koopaIR+="  store @"+it->first+", %"+it->first+"\n";
    }

    IR::blockreturn=0;
    ret.koopaIR += block->get_koopa().koopaIR;
    if (ftype == "void"&&IR::blockreturn==0)//块中没有return 语句
      ret.koopaIR += "  ret\n";
    if (ftype == "int"&&IR::blockreturn==0)
      ret.koopaIR += "  ret 0\n";
    ret.koopaIR += "}\n";
    bmap=*(IR::curbmap);
    IR::curbmap=bmap.parent;
    return ret;

  }
 
};

class FuncParamAST : public BaseAST
{
public:
  int type;
  std::string ident;
  std::unique_ptr<BaseAST> btype;
  std::unique_ptr<BaseAST> exps;
  
  IR get_koopa() override
  {
    IR ret;
    if (type==0)
    {
      ret.koopaIR="@"+ident+": i32";
      IR::curbmap->umap.emplace(ident,Val(0,0,-1));
    }
    else if (type==1){}
    else{}
    return ret;
  }
};
class BlockItemsAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> blockitems;
  std::unique_ptr<BaseAST> blockitem;
  IR get_koopa() override
  {
    IR ret;
    if (type==0)
    {
      IR blksIR=blockitems->get_koopa();
      IR blkIR=blockitem->get_koopa();
      if (blkIR.alreturn==1)
        ret.alreturn=1;
      else
      {
        if (blkIR.alreturn==2)
          ret.alreturn=blksIR.alreturn;
      }
      ret.koopaIR=blksIR.koopaIR+blkIR.koopaIR;
    }
    else
    {
      IR blkIR=blockitem->get_koopa();
      if (blkIR.alreturn==1)
      ret.alreturn=1;
      else
      {
        if (blkIR.alreturn==2)
          ret.alreturn=2;
      }
      ret.koopaIR=blkIR.koopaIR;
    }
    return ret;
  }
};


class BlockItemAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> stmt;
   IR get_koopa() override
  {
    IR ret;
    if (type==0)
    {
      ret = decl->get_koopa();
    }
    else
    {
      ret = stmt->get_koopa();
    }
    return ret;
  }
};

class BlockAST : public BaseAST {
    public:
    enum{
        NONE,
        ITEM
    }type;
   std::unique_ptr<BaseAST> blockitems;
    blockmap bmap;
   BlockAST (std::unique_ptr<BaseAST> & e){
    blockitems=std::move(e);
    type=ITEM;
  }
  BlockAST(){
    type=NONE;
  }
  IR get_koopa() override {
    IR ret;
    switch (type)
    {
    case NONE:{
      ret.alreturn=2;
        return ret;
        break;
    }
        
    case ITEM:{
       bmap.umap=std::unordered_map<std::string, Val>();
        bmap.parent=IR::curbmap;
        IR::curbmap=&bmap;
        IR blkIR=blockitems->get_koopa();
        ret.koopaIR=blkIR.koopaIR;
        bmap=*(IR::curbmap);
        IR::curbmap=bmap.parent;
        ret.alreturn=blkIR.alreturn;
        return ret;
        break;
    }
       
    default:
        break;
    }
   
  }

};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> true_stmt;
  std::unique_ptr<BaseAST> false_stmt;
  while_node wi;
  int type;
  StmtAST(){}
  StmtAST(std::unique_ptr<BaseAST> & e,int t){
    exp=std::move(e);
    type=t;
  }
  IR get_koopa() override {
    IR ret,ExpIR;
    switch (type)
    {
    case 0://LVal '=' Exp ';'
      {
        
        ExpIR=exp->get_koopa();
        ret.koopaIR=ExpIR.koopaIR;
        LValAST* var=(LValAST*)(lval.get());//共享变量
        std::string ident=var->ident;
        Val V=IR::curbmap->find(ident)->second;
        IR lvalIR=lval->get_koopa();
        switch (V.type)
        {
        case Val::VAR:
            IR::curbmap->find(ident)->second.num=ExpIR.num;
  
        case Val::CONST:
          if (ExpIR.IRtype==IR::EXP)
          ret.koopaIR+="  store %"+std::to_string(ExpIR.store_rid)+", @"+ident+"_"+std::to_string(V.index)+"\n";
          else if(ExpIR.IRtype==IR::NUM)
          ret.koopaIR+="  store "+std::to_string(ExpIR.num)+", @"+ident+"_"+std::to_string(V.index)+"\n"; 
          break;
        case Val::ARR:
            ret.IRtype = IR::EXP;
            if (ExpIR.IRtype==IR::EXP)
            ret.koopaIR+=lvalIR.koopaIR+"  store %"+std::to_string(ExpIR.store_rid)+", %"+std::to_string(lvalIR.store_rid)+"\n";
            else if(ExpIR.IRtype==IR::NUM)
            ret.koopaIR+=lvalIR.koopaIR+"  store "+std::to_string(ExpIR.num)+", %"+std::to_string(lvalIR.store_rid)+"\n"; 
        break;
        case Val::CARR:
          if (ExpIR.IRtype==IR::EXP)
          ret.koopaIR+="  store %"+std::to_string(ExpIR.store_rid)+", @"+ident+"_"+std::to_string(V.index)+"\n";
          else if(ExpIR.IRtype==IR::NUM)
          ret.koopaIR+="  store "+std::to_string(ExpIR.num)+", @"+ident+"_"+std::to_string(V.index)+"\n"; 
        default:
        break;
        }
        break;
      }
    case 1://RETURN Exp ';' 
      {  // ret.koopaIR+="stmt 1型";
          if (IR::blockreturn==1)
        {
          return ret;
        }
        ExpIR = exp->get_koopa();
        ret.koopaIR += ExpIR.koopaIR;
        if (ExpIR.IRtype == IR::EXP)
          ret.koopaIR += "  ret %" + std::to_string(ExpIR.store_rid) + '\n';
        else
          ret.koopaIR += "  ret " + std::to_string(ExpIR.num) + '\n';
     
        IR::blockreturn=1;
        break;
      }
     
    case 2://RETURN ';' 
     {
      if (IR::blockreturn==1)
      {
        return ret;
      }
      ret.koopaIR += "  ret \n";
      IR::blockreturn=1;
      break;
     }
    case 3://Block 
    {
      
      IR blkIR=block->get_koopa();
      ret.koopaIR =  blkIR.koopaIR;
      ret.alreturn = blkIR.alreturn;
        break;
    }
    case 4://Exp ';' 
    {
      ret.koopaIR=exp->get_koopa().koopaIR;
      ret.alreturn=0;
      break;
    }
        
    case 5: //';' 
      {
        ret.koopaIR="";
      ret.alreturn=0;
      break;
      }
    case 6://IF '(' Exp ')' Stmt
      {
        ret.koopaIR="";
         
        int label_then=IR::blocks;
        IR::blocks++;
        int label_next=IR::blocks;//IF 语句的下一块
        IR::blocks++;

        ExpIR = exp->get_koopa();
        ret.koopaIR+=ExpIR.koopaIR;
        if (ExpIR.IRtype==IR::EXP)
          ret.koopaIR+="  br %"+std::to_string(ExpIR.store_rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_next)+"\n";
        else 
        {//加0，当成表达式处理
          int rid=IR::registers;
          IR::registers++;
          ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
          ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_next)+"\n";
        }
       
        int blkr=IR::blockreturn;
        IR::blockreturn=0;
        IR tIR=true_stmt->get_koopa();
          ret.koopaIR+="%block"+std::to_string(label_then)+":\n"+tIR.koopaIR;
        if (IR::blockreturn==0)
          ret.koopaIR+="  jump %block"+std::to_string(label_next)+"\n";
        IR::blockreturn=blkr;
        ret.koopaIR+="%block"+std::to_string(label_next)+":\n";
        IR::blockreturn=0;
        break;
      }
     
    case 7://IF '(' Exp ')' Stmt ELSE Stmt
      {
        ret.koopaIR="";
      
      int label_then=IR::blocks;
      IR::blocks++;
      int label_else=IR::blocks;
      IR::blocks++;
      int label_next=IR::blocks;
      IR::blocks++;

      ExpIR = exp->get_koopa();
      ret.koopaIR+=ExpIR.koopaIR;
      if (ExpIR.IRtype==IR::EXP)
      ret.koopaIR+="  br %"+std::to_string(ExpIR.store_rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_else)+"\n";
      else
      {
        int rid=IR::registers;
        IR::registers++;
        ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
        ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_else)+"\n";
      }

      int blkr=IR::blockreturn;
      IR::blockreturn=0;
      IR tIR=true_stmt->get_koopa();
      ret.koopaIR+="%block"+std::to_string(label_then)+":\n"+tIR.koopaIR;
      if (IR::blockreturn==0)
        ret.koopaIR+="  jump %block"+std::to_string(label_next)+"\n";      
      IR::blockreturn=0;
      IR fIR=false_stmt->get_koopa();
      ret.koopaIR+="%block"+std::to_string(label_else)+":\n"+fIR.koopaIR;
      if (IR::blockreturn==0)
        ret.koopaIR+="  jump %block"+std::to_string(label_next)+"\n";
      IR::blockreturn=blkr;
      ret.koopaIR+="%block"+std::to_string(label_next)+":\n";
      IR::blockreturn=0;
        break;
      }
    case 8://WHILE '(' Exp ')' Stmt
    {
       ret.koopaIR="";
      
      
      int tabel_cond=IR::blocks;//条件块号
      IR::blocks++;
      int tabel_then=IR::blocks;//真块号
      
      IR::blocks++;
      int tabel_end=IR::blocks;//假语句块号
      IR::blocks++;
      //
      ret.koopaIR+="  jump %block"+std::to_string(tabel_cond)+"\n";
      ret.koopaIR+="%block"+std::to_string(tabel_cond)+":\n";
      ExpIR = exp->get_koopa();
      ret.koopaIR+=ExpIR.koopaIR;
      if (ExpIR.IRtype==IR::EXP)
        ret.koopaIR+="  br %"+std::to_string(ExpIR.store_rid)+", %block"+std::to_string(tabel_then)+", %block"+std::to_string(tabel_end)+"\n";
      else
      {
        int rid=IR::registers;
        IR::registers++;
        ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
        ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(tabel_then)+", %block"+std::to_string(tabel_end)+"\n";
      }
     
      wi.while_cond=tabel_cond;
      wi.while_end=tabel_end;
      wi.parent=IR::curwlist;
      IR::curwlist=&wi;
      int blkr=IR::blockreturn;
      IR::blockreturn=0;
      IR tIR=true_stmt->get_koopa();
      IR::curwlist=wi.parent;
      ret.koopaIR+="%block"+std::to_string(tabel_then)+":\n"+tIR.koopaIR;
      if (IR::blockreturn==0)
        ret.koopaIR+="  jump %block"+std::to_string(tabel_cond)+"\n";
      IR::blockreturn=blkr;
      
      ret.koopaIR+="%block"+std::to_string(tabel_end)+":\n";
      IR::blockreturn=0;
      break;
    }
    case 9://BREAK ';'
    {
      ret.koopaIR="  jump %block"+std::to_string(IR::curwlist->while_end)+"\n"+"%useless"+std::to_string(IR::uselessblocks)+":\n";
      IR::uselessblocks++;
      break;
    }
    case 10://CONTINUE ';'
    {
      ret.koopaIR="  jump %block"+std::to_string(IR::curwlist->while_cond)+"\n"+"%useless"+std::to_string(IR::uselessblocks)+":\n";
      IR::uselessblocks++;
    }
    default:
      break;
    }
    
    return ret;
  }

};
