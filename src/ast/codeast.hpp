#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa.h"

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
class CompUnitAST : public BaseAST
{
  

public:
     std::unique_ptr<BaseAST> func_def;
    IR get_koopa() const override 
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
  IR get_koopa() const override {
    
    IR ret=func_type->get_koopa();

    std::string ftype =ret.koopaIR;
    ret.koopaIR= "fun @" + ident +"(): ";
    if(ftype=="int")
      ret.koopaIR += "i32";
    ret.koopaIR += " {\n";
    ret.koopaIR += "%entry:\n";
    IR::blockreturn=0;
    ret.koopaIR += block->get_koopa().koopaIR;
    if (ftype == "void"&&IR::blockreturn==0)
      ret.koopaIR += "  ret\n";
    if (ftype == "int"&&IR::blockreturn==0)
      ret.koopaIR += "  ret 0\n";
    ret.koopaIR += "}\n";
   // bmap=*(IR::curbmap);
    //std::cout << IR::curbmap->umap.find("a")->second.index << std::endl;
   // IR::curbmap=bmap.parent;
    return ret;

  }
  int Cal() const override{ return 12; }
  // void DumpAST() const override {
  //   std::cout << "FuncDefAST { ";
  //   func_type->DumpAST();
  //   std::cout << ", " << ident << ", ";
  //   block->DumpAST();
  //   std::cout << " }";
  // }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  FuncTypeAST(const char *_type) : type(_type) {}
  IR get_koopa() const override {//可能改void
    IR ret;
    ret.koopaIR=type;
    return ret;
   
  }
  // void DumpAST() const override {
  //   std::cout << "FuncTypeAST { ";
  //   std::cout << type;
  //   std::cout << " }";
  // }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
   BlockAST (std::unique_ptr<BaseAST> & e){
    stmt=std::move(e);
  }
  IR get_koopa() const override {
    return stmt->get_koopa();
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
  int type;
  StmtAST(std::unique_ptr<BaseAST> & e,int t){
    exp=std::move(e);
    type=t;
  }
  IR get_koopa() const override {
  //  if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
    IR ret,ExpIR;
    switch (type)
    {
    case 0:
     // be_end_dep[nowdep]=be_end_bl[nowbl]=1;
     if (IR::blockreturn==1)
      {
        //ret.alreturn=1;
        return ret;
      }
      ExpIR = exp->get_koopa();
      ret.koopaIR = ExpIR.koopaIR;
      if (ExpIR.IRtype == IR::EXP)
        ret.koopaIR += "  ret %" + std::to_string(ExpIR.store) + '\n';
      else
        ret.koopaIR += "  ret " + std::to_string(ExpIR.num) + '\n';
      //ret.alreturn=1;
      IR::blockreturn=1;
      break;
    
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

// class NumberAST : public BaseAST {
//  public:
//   int number;
//   NumberAST(int i):number(i){}
//   IR get_koopa() const override {
//     IR ret;
//     ret.koopaIR+=to_string(number);
//     ret.num=number;
//     return ret;
//   }
//   int Cal()const override
//   {
//       return number;
//   }
//   // void DumpAST() const override {
//   //   std::cout << "NumberAST { " << number << " }";
//   // }
// };

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