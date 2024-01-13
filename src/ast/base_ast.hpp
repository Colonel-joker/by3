/*介绍
IRtype： 枚举类型，表示中间代码的类型，包括 EXP（表达式）、NUM（数字）和 ARR（数组）。

registers： 记录中间代码中使用的寄存器数量。
constmap： std::map，用于管理常量表，记录常量的标识符及其索引。
globalname： std::map，用于管理全局符号表，记录全局符号的类型（常量、变量、int函数、void函数）。
asize： std::vector，用于记录数组的大小信息。
curbmap： 指向 blockmap 类型的指针，用于表示当前块（作用域）的符号表。
store： 存储位置，用于记录中间代码中的存储位置。
num： 立即数，用于记录中间代码中的数字值。
alreturn： 标记当前块中是否存在返回语句。
koopaIR： 字符串，用于存储生成的目标代码。
funcrparams： 字符串，用于存储函数调用时的参数信息。
IR::constdefing 为 1，表示当前正在处理常量定义

*/
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <unordered_map>
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
  int num;  
  int index;//变量索引，0是全局变量，正数是局部变量
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

struct blockmap{//符号表
  std::unordered_map<std::string, Val> umap;
  blockmap *parent;
  int alreturn;
  blockmap(){
    parent=0;
  }
  blockmap(blockmap* p){
    parent=p;
  }
  std::unordered_map<std::string, Val>::iterator find(std::string ident){
    std::unordered_map<std::string, Val>::iterator it=umap.find(ident);
    if (it!=umap.end())
      return it;
    else  //去父节点找
    {
      if (parent!=0)
         return parent->find(ident);
      else 
         return std::unordered_map<std::string, Val>::iterator();//空指针
    }
  }
};

struct while_node{//每一层创建一个wile_node用于记录while的条件块号和跳出块号
  int while_cond;
  int while_end;
  while_node* parent;
  while_node()
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
    //  VAR
   }IRtype; 
  static int registers;//寄存器数目
  static int constdefing;//在定义常量
  static int globaldef;//在定义全局变量
  static int blocks;//每当需要创建一个新的基本块时，IR::blocks 的值会被递增，并且用于为新的基本块生成一个唯一的标识符
  static int uselessblocks;//废块，给contunue和break后的语句标记上
  
  static int blockreturn;//block中是否有返回语句，用于决定谁来输出ret
  static std::unordered_map<std::string, int> constmap;//常量表建立映射：  ident->index 避免同名
  static std::unordered_map<std::string, int> functype_map; //函数类型表建立映射：ident->type 规定2 int函数; 3 void函数   
  static blockmap* curbmap;//当前块的符号表指针
  int store_rid; //当前表达式存储的寄存器号
  int num;   //立即数
  static while_node* curwlist;//lv7 当前while链表的指针,实现一层层跳出去
  int alreturn;//alreturn处理多个语句时快速检查代码块是否包含返回语句
  std::string koopaIR;//中间代码字符串
  std::string funcrparams;//参数串 lv8
  void get_IRtype_fromVal(Val::VT v){
      switch (v)
      {
      case Val::VAR:
        this->IRtype=EXP;
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
    store_rid = 0;
    num = 0;
    IRtype = EXP;
    koopaIR = "";
    alreturn = 0;
  }
};


class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual IR get_koopa() = 0;
 
};

