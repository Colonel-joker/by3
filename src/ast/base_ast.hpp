
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual void DumpAST() const = 0;
   virtual std::string to_string() const
    {
        return "(Not Implement)";
    }
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    func_def->Dump();
  }
  void DumpAST() const override {
    std::cout << "CompUnitAST { ";
    func_def->DumpAST();
    std::cout << " }";
  }
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "fun ";
    std::cout << "@" << ident << "(): ";
    func_type->Dump();
    std::cout << "{ " << std::endl;
    block->Dump();
    std::cout << "} " << std::endl;
  }
  void DumpAST() const override {
    std::cout << "FuncDefAST { ";
    func_type->DumpAST();
    std::cout << ", " << ident << ", ";
    block->DumpAST();
    std::cout << " }";
  }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  
  void Dump() const override {
    std::cout << "i32" << " ";
  }
  void DumpAST() const override {
    std::cout << "FuncTypeAST { ";
    std::cout << type;
    std::cout << " }";
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  void Dump() const override {
    std::cout << "\%entry" << ": " << std::endl;
    stmt->Dump();
    std::cout << std::endl;
  }
  void DumpAST() const override {
    std::cout << "BlockAST { ";
    stmt->DumpAST();
    std::cout << " }";
  }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> newexp;

  void Dump() const override {
    std::cout << "  ret ";
    newexp->Dump();
  }
  void DumpAST() const override {
    std::cout << "StmtAST { ";
    newexp->DumpAST();
    std::cout << " }";
  }
};

class NumberAST : public BaseAST {
 public:
  int number;

  void Dump() const override {
    std::cout << number;
  }
  void DumpAST() const override {
    std::cout << "NumberAST { " << number << " }";
  }
};