#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "koopa.h"
#include <unordered_map>
#include <queue>

using namespace std;


string  riscv_code = "";

long numins=0;
long stackptr=0;

string registers[16] = {"a0","x0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

long registerptr = 2;
long val_zero=0;
struct sitem{
  long pos;
  long val; 
  long dim;
  long mtype; //0 i32 1 array 2 pointer
  vector<long> asize;
  sitem(){
    pos=0;
    dim=0;
    val=0;
  }
  sitem(long Pos,long Val){
    pos=Pos;
    val=Val;
  }
};
unordered_map<koopa_raw_value_t, long> riscvmap;
unordered_map<koopa_raw_value_t, sitem> stackmap;
unordered_map<koopa_raw_value_t, long>::iterator it;
void Visit(const koopa_raw_program_t &);
void Visit(const koopa_raw_slice_t &);
void Visit(const koopa_raw_function_t &);
void Visit(const koopa_raw_basic_block_t &);
void Visit(const koopa_raw_value_t &);
void Visit(const koopa_raw_return_t &);
void Visit(const koopa_raw_integer_t &);
void Visit(const koopa_raw_binary_t &);

void Visit(const koopa_raw_store_t &);
sitem Visit(const koopa_raw_load_t &);
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



void local_alloc(const koopa_raw_value_t &value,sitem &ret,long functype){
  int flag=1;
  if (value->ty->data.pointer.base->tag==KOOPA_RTT_INT32)
  {
    ret.pos=stackptr; ret.val=0; ret.mtype=0;
    if (functype==1)
    {
      stackptr++;
      stackmap.emplace(value,ret);
    }
    else if (functype==0)
    numins++;
  }
 
}


void Visit(const koopa_raw_program_t &program)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  //Visit(program.values);
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
  // 执行一些其他的必要操作
    riscv_code +="\n  .text\n  .globl ";
    riscv_code += (func->name+1);//@main
    riscv_code += '\n';
    riscv_code += (func->name + 1);
    riscv_code += ":\n";
    if (numins > 0)
    {
      long stacksize=16*((numins*4+15)/16);
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
    Visit(func->bbs);
  
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
    string name(bb->name+1);
    if (name!="%entry")
       riscv_code+=name+":\n";
  
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value)
{
  // 根据指令类型判断后续需要如何访问
  sitem ret;
  const auto &kind = value->kind;
  //cout << kind.tag << endl;
  switch (kind.tag)
  {
  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    // 访问 integer 指令
    Visit(kind.data.integer);
    if (!val_zero)
    {
      riscvmap.emplace(value, registerptr);
      registerptr++;
    }
    else {
      riscvmap.emplace(value, 1);//默认x0
      val_zero=0;
    }
    break;
  case KOOPA_RVT_BINARY:
    Visit(kind.data.binary);
    ret.pos=stackptr; 
    ret.val=0; 
    ret.mtype=2;
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
       riscv_code+=lw_cmd(registers[registerptr],stackmap.find(ret.value)->second.pos ); 
    }
  }
    if(numins>0){
      long stacksize=16*((numins*4+15)/16);
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

long zeroblocks=0;
void Visit(const koopa_raw_integer_t &minteger)
{
  if (minteger.value!=0)
  {
    if (zeroblocks>4)
    {
       riscv_code+="  .zero "+to_string(zeroblocks)+"\n";
      zeroblocks=0;
    }
    else if (zeroblocks==4)
    {
       riscv_code+="  .word "+to_string(0)+"\n";
      zeroblocks=0;
    }
     riscv_code+="  .word "+to_string(minteger.value)+"\n";
  }
  else
  {
    zeroblocks+=4;
  }
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
     riscv_code+=lw_cmd("t0",stackmap.find(binary.lhs)->second.pos );
  }

  if (binary.rhs->kind.tag == KOOPA_RVT_INTEGER)
  {
     registerptr=3;
     riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(binary.rhs->kind.data.integer.value) + '\n';
    //Visit(binary.rhs);
  }
  else
  {
     riscv_code+=lw_cmd("t1",stackmap.find(binary.rhs)->second.pos );
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

void Visit(const koopa_raw_store_t &mstore)
{
  registerptr=2;
  if (mstore.value->kind.tag == KOOPA_RVT_INTEGER)
  {
     riscv_code += "  li\t" + registers[registerptr] + ", " + to_string(mstore.value->kind.data.integer.value) + '\n';
  
  }
  else{
     riscv_code+=lw_cmd("t0",stackmap.find(mstore.value)->second.pos );   
  }
     riscv_code += sw_cmd("t0",stackmap.find(mstore.dest)->second.pos );
     
}

sitem Visit(const koopa_raw_load_t &mload)
{
  registerptr=2;
  sitem ret;
  ret.pos=stackptr; ret.val=0;  

  //局部变量
  sitem src=stackmap.find(mload.src)->second;
  riscv_code+=lw_cmd("t0",src.pos );
  
  return ret;
}

void Visit(const koopa_raw_branch_t &mbranch)
{
  registerptr=2;
  string t_block=string(mbranch.true_bb->name+1);
  string f_block=string(mbranch.false_bb->name+1);
   riscv_code+=lw_cmd("t0",stackmap.find(mbranch.cond)->second.pos );
   riscv_code += "  bnez\tt0, " +t_block+"\n";
   riscv_code += "  j "+f_block+"\n";
}

void Visit(const koopa_raw_jump_t &mjump)
{
  registerptr=2;
  string j_block=string(mjump.target->name+1);
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
  koopa_delete_raw_program_builder(builder);
  cout <<riscv_code << endl;
  
}
