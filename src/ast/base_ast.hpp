
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <map>
// 所有 AST 的基类

static int nowww=1;


class IR
{
public:
   enum{
      EXP,
      NUM,
      ARR
   }IRtype;//类型 0 式子 1 立即数 2数组 
  static int registers;
  static int arraydefing;
  static int constdefing;
  static int vardefing;
  static int blocks;
  static int globaldef;
  static int uselessblocks;
  static int shortcircuit;
  static int t_tag;
  static int f_tag;
  static int while_end;
  static int while_cond;
  static int array_ptr;
  static int array_block;
  static int func_array;
  static int blockreturn;
  static std::string var_name;
  //static whileinfo* curwi;
  static std::map<std::string, int> constmap;
  static std::map<std::string, int> globalname; //全局符号 0 常量; 1 变量; 2 int函数; 3 void函数
  static std::vector<int> arraydef;
  static std::vector<int> asize;
  //static blockmap* curbmap;
  int store; //存储位置
  int num;   //立即数
  
  int alreturn;
  std::string koopaIR;
  std::string funcrparams;
  std::vector<int> arrayparam;
  IR()
  {
    store = 0;
    num = 0;
    IRtype = EXP;
    koopaIR = "";
    funcrparams="";
    alreturn = 0;
    arrayparam=std::vector<int>();
  }
};

class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual IR get_koopa() const = 0;
 // virtual void DumpAST() const = 0;
  virtual int Cal() const {
     assert(false);
        return 0;
  }
};

