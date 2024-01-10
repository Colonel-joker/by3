#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa.h"

class AssignmentAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;
    AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp)
    {
        lval = std::move(_lval);
        exp = std::move(_exp);
    }
};

class BranchAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exp;
    InstSet true_instset;
    InstSet false_instset;
    BranchAST(std::unique_ptr<BaseAST> &_exp, InstSet &_true_insts)
    {
        for (auto &inst : _true_insts)
            true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
    BranchAST(std::unique_ptr<BaseAST> &_exp, InstSet &_true_insts, InstSet &_false_insts)
    {
        for (auto &inst : _true_insts)
            true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        for (auto &inst : _false_insts)
            false_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
};

class WhileAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exp;
    InstSet body_insts;
    WhileAST(std::unique_ptr<BaseAST> &_exp, InstSet &_body_insts)
    {
        for (auto &inst : _body_insts)
            body_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
};

class BreakAST : public BaseAST
{
public:
};
class ContinueAST : public BaseAST
{
public:
};