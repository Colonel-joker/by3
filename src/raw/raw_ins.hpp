#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include "koopa.h"
#include <map>
#include <queue>

using namespace std;
queue<long> qins,rins,ains;//
string lw_cmd(string dst,long stacksize)
{
  string cmd="";
  if (stacksize>=2048)
  {
    cmd = cmd + "  li\tt3, "+to_string(stacksize)+"\n";
    cmd+="  add\tt3, sp, t3\n";
    cmd += "  lw\t"+dst+", 0(t3)\n";
  }
  else
  {
    cmd += "  lw\t"+dst+", " + to_string(stacksize) + "(sp)\n";
  }   
  return cmd; 
}

string sw_cmd(string dst,long stacksize)
{
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

// 所有 Riscv 的基类
class RiscvBase
{
public:
  virtual ~RiscvBase() = default;
  virtual std::string mRiscv() = 0;
};

class RiscvInstruction : public RiscvBase
{
public:
  std::string instruction;
  std::string mRiscv() override{
    return "  "+instruction+'\n';
  }
};

class RiscvBlock : public RiscvBase
{
public:
  std::string blockname;
  std::vector<RiscvInstruction> riscvinstructions;
  std::string mRiscv() override{
    std::string s=blockname;
    s=s+'\n'+blockname+":\n";
    for (int i=0;i<riscvinstructions.size();i++)
    {
      s=s+riscvinstructions[i].mRiscv();
    }
    return s;
  }
};

class RiscvFunc : public RiscvBase
{
public:
  RiscvBlock riscvblock;
  std::string mRiscv() override{
    return "  .text\n  .global "+riscvblock.mRiscv();
  }
};

