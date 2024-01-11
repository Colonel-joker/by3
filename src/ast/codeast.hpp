#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa.h"

// class GlobalVarDefAST : public BaseAST
// {
// public:
//     std::string name;
//     std::unique_ptr<BaseAST> exp;

//     GlobalVarDefAST(std::unique_ptr<BaseAST> &vardef_ast)
//     {
//         VarDefAST *var = dynamic_cast<VarDefAST*>(vardef_ast.release());
//         name = var->name;
//         exp = std::move(var->exp);
//         delete var;
//     }
// };
enum InstType
{
    ConstDecl,
    Decl,
    ArrayDecl,
    Stmt,
    Branch,
    While,
    Break,
    Continue
};
typedef std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> InstSet;
//lv4
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
      ret.store=0;
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
        ret.koopaIR+="  store %"+std::to_string(ExpIR.store)+", @"+ident+"_"+std::to_string(index)+"\n";
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

// class GlobalVarDefAST : public BaseAST
// {
// public:
//     std::string name;
//     std::unique_ptr<BaseAST> exp;

//     GlobalVarDefAST(std::unique_ptr<BaseAST> &vardef_ast)
//     {
//         VarDefAST *var = dynamic_cast<VarDefAST*>(vardef_ast.release());
//         name = var->ident;
//         exp = std::move(var->exp);
//         delete var;
//     }
// };
//lv3


class CompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_def;
    IR get_koopa()  override 
    {
        //std::cout << "CompUnitAST { ";
        IR::registers=0;
        return func_def->get_koopa();
        //std::cout << " }";
    }
};


class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> func_para;
  blockmap bmap;
  FuncDefAST(){}
  IR get_koopa()  override {
    bmap.umap=std::map<std::string, Val>();
    bmap.parent=IR::curbmap;
    IR::curbmap=&bmap;//初始化curbmap

    IR ret=func_type->get_koopa();
    std::string ftype =ret.koopaIR;
    ret.koopaIR= "fun @" + ident +"(): ";
    if(ftype=="int")
      ret.koopaIR += "i32";
    ret.koopaIR += " {\n";
    ret.koopaIR += "%entry:\n";
     std::map<std::string, Val>::iterator it;
    for (it=bmap.umap.begin();it!=bmap.umap.end();it++)
    {
      ret.koopaIR+="  %"+it->first+" = alloc ";
      if (it->second.type==Val::ARR)
      {
        int len=it->second.asize.size();
        if (len==0)
        {
          ret.koopaIR+="*i32";
        }
        else
        {
          ret.koopaIR+="*";
          for (int i=1;i<len;i++)
          {
            ret.koopaIR+="[";
          }
          ret.koopaIR+="i32, ";
          for (int i=len-1;i>1;i--)
          {
            ret.koopaIR+=std::to_string(it->second.asize[i])+"], ";
          }
          ret.koopaIR+=std::to_string(it->second.asize[1])+"]";
        }
        ret.koopaIR+="\n";
      }
      else 
        ret.koopaIR+="i32\n";
      ret.koopaIR+="  store @"+it->first+", %"+it->first+"\n";
    }

    IR::blockreturn=0;
    ret.koopaIR += block->get_koopa().koopaIR;
    if (ftype == "void"&&IR::blockreturn==0)
      ret.koopaIR += "  ret\n";
    if (ftype == "int"&&IR::blockreturn==0)
      ret.koopaIR += "  ret 0\n";
    ret.koopaIR += "}\n";
    bmap=*(IR::curbmap);
    IR::curbmap=bmap.parent;
    return ret;

  }
  // void DumpAST() const override {
  //   std::cout << "FuncDefAST { ";
  //   func_type->DumpAST();
  //   std::cout << ", " << ident << ", ";
  //   block->DumpAST();
  //   std::cout << " }";
  // }
};


  // void DumpAST() const override {
  //   std::cout << "BTypeAST { ";
  //   std::cout << type;
  //   std::cout << " }";
  // }
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
       bmap.umap=std::map<std::string, Val>();
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
  // void DumpAST() const override {
  //   std::cout << "BlockAST { ";
  //   stmt->DumpAST();
  //   std::cout << " }";
  // }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> true_stmt;
  std::unique_ptr<BaseAST> false_stmt;
  whileinfo wi;
  int type;
  StmtAST(){}
  StmtAST(std::unique_ptr<BaseAST> & e,int t){
    exp=std::move(e);
    type=t;
  }
  IR get_koopa() override {
  //  if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
    IR ret,ExpIR;
   // ret.koopaIR+="stmt ";
   // ret.koopaIR+=to_string(type);
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
          ret.koopaIR+="  store %"+std::to_string(ExpIR.store)+", @"+ident+"_"+std::to_string(V.index)+"\n";
          else if(ExpIR.IRtype==IR::NUM)
          ret.koopaIR+="  store "+std::to_string(ExpIR.num)+", @"+ident+"_"+std::to_string(V.index)+"\n"; 
          break;
        case Val::ARR:
            ret.IRtype = IR::EXP;
            if (ExpIR.IRtype==IR::EXP)
            ret.koopaIR+=lvalIR.koopaIR+"  store %"+std::to_string(ExpIR.store)+", %"+std::to_string(lvalIR.store)+"\n";
            else if(ExpIR.IRtype==IR::NUM)
            ret.koopaIR+=lvalIR.koopaIR+"  store "+std::to_string(ExpIR.num)+", %"+std::to_string(lvalIR.store)+"\n"; 
        break;
        case Val::CARR:
          if (ExpIR.IRtype==IR::EXP)
          ret.koopaIR+="  store %"+std::to_string(ExpIR.store)+", @"+ident+"_"+std::to_string(V.index)+"\n";
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
          //ret.alreturn=1;
          return ret;
        }
        ExpIR = exp->get_koopa();
        ret.koopaIR += ExpIR.koopaIR;
        if (ExpIR.IRtype == IR::EXP)
          ret.koopaIR += "  ret %" + std::to_string(ExpIR.store) + '\n';
        else
          ret.koopaIR += "  ret " + std::to_string(ExpIR.num) + '\n';
        //ret.alreturn=1;
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
        IR::shortcircuit=0;//重置短路信号
        int label_then=IR::blocks;
        IR::blocks++;
        int label_next=IR::blocks;//IF 语句的下一块
        IR::blocks++;

        ExpIR = exp->get_koopa();
        ret.koopaIR+=ExpIR.koopaIR;
        if (ExpIR.IRtype==IR::EXP||ExpIR.IRtype==IR::VAR)
          ret.koopaIR+="  br %"+std::to_string(ExpIR.store)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_next)+"\n";
        else 
        {//加0，当成表达式处理
          int rid=IR::registers;
          IR::registers++;
          ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
          ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_next)+"\n";
        }
        IR::shortcircuit=0;
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
      IR::shortcircuit=0;
      int label_then=IR::blocks;
      IR::blocks++;
      int label_else=IR::blocks;
      IR::blocks++;
      int label_next=IR::blocks;
      IR::blocks++;

      ExpIR = exp->get_koopa();
      ret.koopaIR+=ExpIR.koopaIR;
      if (ExpIR.IRtype==IR::EXP||ExpIR.IRtype==IR::VAR)
      ret.koopaIR+="  br %"+std::to_string(ExpIR.store)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_else)+"\n";
      else
      {
        int rid=IR::registers;
        IR::registers++;
        ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
        ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(label_then)+", %block"+std::to_string(label_else)+"\n";
      }
      IR::shortcircuit=0;
      
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
      IR::shortcircuit=0;
      
      int tabel_cond=IR::blocks;
      IR::blocks++;
      int tabel_then=IR::blocks;
      IR::t_tag=tabel_then;
      IR::blocks++;
      int tabel_end=IR::blocks;
      IR::blocks++;
      //
      ret.koopaIR+="  jump %block"+std::to_string(tabel_cond)+"\n";
      ret.koopaIR+="%block"+std::to_string(tabel_cond)+":\n";
      ExpIR = exp->get_koopa();
      ret.koopaIR+=ExpIR.koopaIR;
      if (ExpIR.IRtype==IR::EXP||ExpIR.IRtype==IR::VAR)
        ret.koopaIR+="  br %"+std::to_string(ExpIR.store)+", %block"+std::to_string(tabel_then)+", %block"+std::to_string(tabel_end)+"\n";
      else
      {
        int rid=IR::registers;
        IR::registers++;
        ret.koopaIR+="  %"+std::to_string(rid)+" = add 0, "+std::to_string(ExpIR.num)+"\n";
        ret.koopaIR+="  br %"+std::to_string(rid)+", %block"+std::to_string(tabel_then)+", %block"+std::to_string(tabel_end)+"\n";
      }
      IR::shortcircuit=0;
      wi.while_cond=tabel_cond;
      wi.while_end=tabel_end;
      wi.parent=IR::curwi;
      IR::curwi=&wi;
      int blkr=IR::blockreturn;
      IR::blockreturn=0;
      IR tIR=true_stmt->get_koopa();
      IR::curwi=wi.parent;
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
      ret.koopaIR="  jump %block"+std::to_string(IR::curwi->while_end)+"\n"+"%useless"+std::to_string(IR::uselessblocks)+":\n";
      IR::uselessblocks++;
      break;
    }
    case 10://CONTINUE ';'
    {
      ret.koopaIR="  jump %block"+std::to_string(IR::curwi->while_cond)+"\n"+"%useless"+std::to_string(IR::uselessblocks)+":\n";
      IR::uselessblocks++;
    }
    default:
      break;
    }
    
    return ret;
  }
  // void DumpAST() const override {
  //   std::cout << "StmtAST { ";
  //   exp->DumpAST();
  //   std::cout << " }";
  // }
};

class ReturnAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> ret_num;
    ReturnAST()
    {
        ret_num = nullptr;
    }
    ReturnAST(std::unique_ptr<BaseAST> &_ret_num)
    {
        ret_num = std::move(_ret_num);
    }
};