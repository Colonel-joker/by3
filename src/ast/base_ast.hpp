/*介绍
IRtype： 枚举类型，表示中间代码的类型，包括 EXP（表达式）、NUM（数字）和 ARR（数组）。

registers： 记录中间代码中使用的寄存器数量。
constmap： std::map，用于管理常量表，记录常量的标识符及其索引。
globalname： std::map，用于管理全局符号表，记录全局符号的类型（常量、变量、int函数、void函数）。
arraydef： std::vector，用于记录数组的维度信息。
asize： std::vector，用于记录数组的大小信息。
curbmap： 指向 blockmap 类型的指针，用于表示当前块（作用域）的符号表。
store： 存储位置，用于记录中间代码中的存储位置。
num： 立即数，用于记录中间代码中的数字值。
alreturn： 标记当前块中是否存在返回语句。
koopaIR： 字符串，用于存储生成的目标代码。
funcrparams： 字符串，用于存储函数调用时的参数信息。
arrayparam： std::vector，用于存储数组参数的信息。
IR::constdefing 为 1，表示当前正在处理常量定义
*/
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <map>
// 所有 AST 的基类

static int nowww=1;

//0 var,1 const,2 array 3 const array
struct Val{
   enum VT{
      VAR,
      CONST,
      ARR,
      CARR
   }type;
  int num;  // isconst=0或1时为值,isconst=2或3时为维度
  int index;
  std::vector<int> asize;
  Val(){
    type=VAR;
    num=0;
    index=0;
  }
  Val(int t,int Num,int Index){
    switch (t)
    {
    case 0:
      type=VAR;
      break;
    case 1:
      type=CONST;
      break;
    case 2:
      type=ARR;
      break;
    case 3:
      type=CARR;
      break;
    default:
      break;
    }
    num=Num;
    index=Index;
  }
};

struct blockmap{
  std::map<std::string, Val> umap;
  blockmap *parent;
  int alreturn;
  blockmap(){
    parent=0;
  }
  blockmap(blockmap* p){
    parent=p;
  }
  std::map<std::string, Val>::iterator find(std::string ident){
    if (umap.find(ident)!=umap.end())
      return umap.find(ident);
    else
    {
      if (parent!=0)
         return parent->find(ident);
      else 
         return std::map<std::string, Val>::iterator();//空指针
    }
  }
};

struct whileinfo{
  int while_cond;
  int while_end;
  whileinfo* parent;
  whileinfo()
  {
    while_cond=0;
    while_end=0;
  }
};




class IR
{
public:
   enum{
      EXP,
      NUM,
      ARR,
      VAR
   }IRtype;//类型 0 式子 1 立即数 2数组 
  static int registers;
  static int arraydefing;
  static int constdefing;

  static int blocks;//每当需要创建一个新的基本块时，IR::blocks 的值会被递增，并且用于为新的基本块生成一个唯一的标识符
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
  static std::map<std::string, int> constmap;//常量表
  static std::map<std::string, int> globalname; //全局符号 0 常量; 1 变量; 2 int函数; 3 void函数
  static std::vector<int> arraydef;
  static std::vector<int> asize;
  static blockmap* curbmap;
  int store; //存储位置
  int num;   //立即数
  static whileinfo* curwi;
  int alreturn;//alreturn处理多个语句时快速检查代码块是否包含返回语句
  std::string koopaIR;//中间代码字符串
  std::string funcrparams;
  std::vector<int> arrayparam;
  void get_IRtype_fromVal(Val::VT v){
      switch (v)
      {
      case Val::VAR:
        this->IRtype=VAR;
        break;
      case Val::CONST:
        this->IRtype=NUM;
        break;
      default:
        this->IRtype=ARR;
        break;
      }
  }
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

  virtual IR get_koopa() = 0;
 // virtual void DumpAST() const = 0;
  virtual int Cal() const {
     assert(false);
        return 0;
  }
};

