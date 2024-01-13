#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "koopa.h"
#include <unordered_map>
#include <queue>

using namespace std;


string  riscv_code = "";//riscv代码
string registers[16] = {"a0","x0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
string funclist[8]={"@getint","@getch","@getarray","@putint","@putch","@putarray","@starttime","@stoptime"};//跳过库函数
long num_ins=0;//指令数
bool detect_depth=0;//先根据指令数目探测栈深度
long stackptr=0;//栈指针
long registerptr = 2;//一般从t0开始用寄存器，本部分实际上只使用了a0,x0,t0,t1
struct stack_value{//栈值
  long depth;
  long val; 
  long type; //0 i32 1 array 2 pointer
  stack_value(long P,long V):depth(P),val(V){}
  stack_value():depth(0),val(0){
  }
};

unordered_map<koopa_raw_value_t, stack_value> stackmap;//栈

void Visit(const koopa_raw_program_t &);
void Visit(const koopa_raw_slice_t &);
void Visit(const koopa_raw_function_t &);
void Visit(const koopa_raw_basic_block_t &);
void Visit(const koopa_raw_value_t &);
void Visit(const koopa_raw_return_t &);
//void Visit(const koopa_raw_integer_t &);
void Visit(const koopa_raw_binary_t &);

void Visit(const koopa_raw_store_t &);
stack_value Visit(const koopa_raw_load_t &);
void Visit(const koopa_raw_branch_t &);
void Visit(const koopa_raw_jump_t &);


string lw_cmd(string dst,long stacksize)
{
  stacksize*=4;
  string cmd="";
  if (stacksize>=2048)
  {
    cmd+= "  li\tt3, "+to_string(stacksize)+"\n";
    cmd+="  add\tt3, sp, t3\n";
    cmd+= "  lw\t"+dst+", 0(t3)\n";
  }
  else
  {
    cmd += "  lw\t"+dst+", " + to_string(stacksize) + "(sp)\n";
  }   
  return cmd; 
}

string sw_cmd(string dst,long stacksize)
{
  stacksize*=4;
  string cmd="";
  if (stacksize>=2048)
  {
    cmd = cmd + "  li\tt3, "+to_string(stacksize)+"\n";
    cmd+="  add\tt3, sp, t3\n";
    cmd += "  sw\t"+dst+", 0(t3)\n";
  }
  else
  {
    cmd += "  sw\t"+dst+", "+to_string(stacksize)+"(sp)\n";
  }   
  return cmd; 
}



void local_alloc(const koopa_raw_value_t &value,stack_value &ret,long functype){
  if (value->ty->data.pointer.base->tag==KOOPA_RTT_INT32)
  {
    ret.depth=stackptr; 
    ret.val=0; 
    ret.type=0;
    if (functype==1)
    {
      stackptr++;
      stackmap.emplace(value,ret);
    }
    else if (functype==0)
      num_ins++;
  }
 
}


void Visit(const koopa_raw_program_t &program)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice)
{
  for (size_t i = 0; i < slice.len; ++i)
  {
    auto ptr = slice.buffer[i];
    switch (slice.kind)
    {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func)
{
  bool libfunc=0;
    for (int i=0;i<8;i++)
  {
    if (func->name==funclist[i])
    {
      libfunc=1;
      break;
    }
  }
  if(!libfunc){
    // 执行一些其他的必要操作
    if(detect_depth){//探测完stack深度了吗？
      riscv_code +="\n  .text\n  .globl ";
      riscv_code += (func->name+1);//@main
      riscv_code += '\n';
      riscv_code += (func->name + 1);
      riscv_code += ":\n";
      if (num_ins > 0)
      {
        long stacksize=16*((num_ins*4+15)/16);
        if (stacksize>=2048)
        {
          riscv_code = riscv_code + "  li\tt0, -"+to_string(stacksize)+"\n";
          riscv_code+="  add\tsp, sp, t0\n";
        }
        else
        {
          riscv_code+="  addi\tsp, sp, -"+to_string(stacksize)+"\n";
        }
      }
    }
    else{
      num_ins=0;
    }
    
    Visit(func->bbs);
  }
  
  
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  if(detect_depth){
    string name(bb->name+1);
    if (name!="%entry")
      riscv_code+=name+":\n";
  }
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value)
{
  // 根据指令类型判断后续需要如何访问
  stack_value ret;
  const auto &kind = value->kind;
  if(!detect_depth){
    if (kind.tag==KOOPA_RVT_ALLOC)
    {
      local_alloc(value,ret,0);
      return;
    }
    if (value->ty->tag!=KOOPA_RTT_UNIT)
      num_ins++;
    
    return;

  }
  switch (kind.tag)
  {
  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret);
    break;
  // case KOOPA_RVT_INTEGER:
  //   // 访问 integer 指令
  //   Visit(kind.data.integer);
  //   break;
  case KOOPA_RVT_BINARY:
    Visit(kind.data.binary);
    ret.depth=stackptr; 
    ret.val=0; 
    ret.type=2;
     riscv_code+=sw_cmd("t0",stackptr );
    stackptr++;
    stackmap.emplace(value,ret);
    break;
  case KOOPA_RVT_ALLOC:
    local_alloc(value,ret,1);
    break;
  case KOOPA_RVT_LOAD:
    ret=Visit(kind.data.load);
    riscv_code+=sw_cmd("t0",stackptr );
    stackptr++;
    stackmap.emplace(value,ret);
    break;
  case KOOPA_RVT_STORE:
    Visit(kind.data.store);
    break;
  case KOOPA_RVT_BRANCH:
    Visit(kind.data.branch);
    break;
  case KOOPA_RVT_JUMP:
    Visit(kind.data.jump);
    break;
  }
}

void Visit(const koopa_raw_return_t &ret)
{
  registerptr = 0;
  if (ret.value!=0)//可能有 return; 即void 类型
  {
    if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
    {
       riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(ret.value->kind.data.integer.value) + '\n';
    }
    else
    {
       riscv_code+=lw_cmd(registers[registerptr],stackmap.find(ret.value)->second.depth ); 
    }
  }
    if(num_ins>0){
      long stacksize=16*((num_ins*4+15)/16);
      if (stacksize>=2048)
      {
        riscv_code+="  li\tt0, "+to_string(stacksize)+"\n";
        riscv_code+="  add\tsp, sp, t0\n";
      }
      else
      {
        riscv_code+="  addi\tsp, sp, "+to_string(stacksize)+"\n";
      }
    }
    
  
   riscv_code += "  ret\n";
}



void Visit(const koopa_raw_binary_t &binary)
{
  if (binary.lhs->kind.tag == KOOPA_RVT_INTEGER)
  {
    registerptr=2;
     riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(binary.lhs->kind.data.integer.value) + '\n';
  }
  else
  {
     riscv_code+=lw_cmd("t0",stackmap.find(binary.lhs)->second.depth );
  }

  if (binary.rhs->kind.tag == KOOPA_RVT_INTEGER)
  {
     registerptr=3;
     riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(binary.rhs->kind.data.integer.value) + '\n';
    //Visit(binary.rhs);
  }
  else
  {
     riscv_code+=lw_cmd("t1",stackmap.find(binary.rhs)->second.depth );
  }
  switch (binary.op)//查询enum koopa_raw_binary_op
  {
  case 0:
     riscv_code += "  xor\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  snez\tt0, t0\n";
    break;
  case 1:
     riscv_code += "  xor\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  seqz\tt0, t0\n";
    break;
  case 2:
     riscv_code += "  sgt\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  snez\tt0, t0\n";
    break;
  case 3:
     riscv_code += "  slt\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  snez\tt0, t0\n";
    break;
  case 4:
     riscv_code += "  slt\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  seqz\tt0, t0\n";
    break;
  case 5:
     riscv_code += "  sgt\t";
     riscv_code+="t0, t0, t1\n";
     riscv_code += "  seqz\tt0, t0\n";
    break;
  case 6:
     riscv_code += "  add\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 7:
     riscv_code += "  sub\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 8:
     riscv_code += "  mul\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 9:
     riscv_code += "  div\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 10:
     riscv_code += "  rem\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 11:
     riscv_code += "  and\t";
     riscv_code+="t0, t0, t1\n";
    break;
  case 12:
     riscv_code += "  or\t";
     riscv_code+="t0, t0, t1\n";
    break;
  }

}

void Visit(const koopa_raw_store_t &s)
{
  registerptr=2;
  if (s.value->kind.tag == KOOPA_RVT_INTEGER)
  {
     riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(s.value->kind.data.integer.value) + '\n';
  
  }
  else{
     riscv_code+=lw_cmd("t0",stackmap.find(s.value)->second.depth );   
  }
     riscv_code += sw_cmd("t0",stackmap.find(s.dest)->second.depth );
     
}

stack_value Visit(const koopa_raw_load_t &l)
{
  registerptr=2;
  stack_value ret;
  ret.depth=stackptr; ret.val=0;  

  //局部变量
  stack_value src=stackmap.find(l.src)->second;
  riscv_code+=lw_cmd("t0",src.depth );
  
  return ret;
}

void Visit(const koopa_raw_branch_t &b)
{
  registerptr=2;
  string t_block=string(b.true_bb->name+1);
  string f_block=string(b.false_bb->name+1);
   riscv_code+=lw_cmd("t0",stackmap.find(b.cond)->second.depth );
   riscv_code += "  bnez\tt0, " +t_block+"\n";
   riscv_code += "  j "+f_block+"\n";
}

void Visit(const koopa_raw_jump_t &j)
{
  registerptr=2;
  string j_block=string(j.target->name+1);
   riscv_code += "  j "+j_block+"\n";
}



void p2(const char* str){



  koopa_program_t program;
  koopa_error_code_t kpret = koopa_parse_from_string(str, &program);
  assert(kpret == KOOPA_EC_SUCCESS); // 确保解析时没有出错
  
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  Visit(raw);
  detect_depth=1;
  Visit(raw);
  koopa_delete_raw_program_builder(builder);
  cout <<riscv_code << endl;
  
}
