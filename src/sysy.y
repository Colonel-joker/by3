%code requires {
  #include <memory>
  #include <cstring>
  #include <string>
  #include "ast/ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include "ast/ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror( std::unique_ptr<BaseAST>  &ast, const char *s);

using namespace std;
int IR::registers=0;
int IR::arraydefing=0;
int IR::constdefing=0;
int IR::vardefing=0;
int IR::blocks=0;
int IR::uselessblocks=0;
int IR::shortcircuit=0;
int IR::globaldef=0;
int IR::t_tag=0;
int IR::f_tag=0;
int IR::while_end=-1;
int IR::while_cond=-1;
int IR::array_ptr=0;
int IR::array_block=0;
int IR::func_array=0;
int IR::blockreturn=0;
string IR::var_name="";
map<string,int> IR::constmap;
map<string,int> IR::globalname;
vector<int> IR::arraydef; 
vector<int> IR::asize;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }


// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT UNARYOP MULOP ADDOP RELOP EQOP LANDOP LOROP
%token <int_val> INT_CONST


//非终结符
%type <ast_val> FuncDef FuncType Block Stmt
%type <int_val>  Number 
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

%%
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;


// 同上, 不再解释
FuncType
  : INT {
        $$ = new FuncTypeAST("int");
    } | VOID {
        $$ = new FuncTypeAST("void");
    }
  ;

Block :
    '{' Stmt  '}' {
        auto s = std::unique_ptr<BaseAST>($2);
        $$=new BlockAST(s);
    };



Stmt
  : RETURN Exp ';' {
        auto number = std::unique_ptr<BaseAST>($2);
      
        $$ =new StmtAST(number,0);
    }
  ;
Exp 
    : LOrExp {
        auto add_exp = std::unique_ptr<BaseAST>($1);
        $$ = new ExpAST(add_exp);
    }
    ;

PrimaryExp
    : '(' Exp ')' {
        auto exp = std::unique_ptr<BaseAST>($2);
        $$ = new PrimaryExpAST(exp);
    }
    | Number {
        auto num=int($1);
        $$ = new PrimaryExpAST(num);
    }
    ;

Number
    : INT_CONST {
        $$=int($1);
    }
    ;

UnaryExp
    : PrimaryExp {
        auto primary_exp = std::unique_ptr<BaseAST>($1);
        $$ = new UnaryExpAST(primary_exp);
    }
    | '-' UnaryExp {
        string op ="-";
        auto unary_exp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(op.c_str(), unary_exp);
    }
    | '!' UnaryExp {
        string op ="!";
        auto unary_exp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(op.c_str(), unary_exp);
    }
    | '+' UnaryExp {
        string op ="+";
        auto unary_exp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(op.c_str(), unary_exp);
    }
    ;




MulExp
    : UnaryExp {
        auto unary_exp = std::unique_ptr<BaseAST>($1);
        $$ = new MulExpAST(unary_exp);
    }
    | MulExp '*' UnaryExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op = "*";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new MulExpAST(left_exp, op.c_str(), right_exp);
    }
    | MulExp '/' UnaryExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op = "/";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new MulExpAST(left_exp, op.c_str(), right_exp);
    }
    | MulExp '%' UnaryExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op = "%";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new MulExpAST(left_exp, op.c_str(), right_exp);
    }
    ;

AddExp
    : MulExp {
        auto mul_exp = std::unique_ptr<BaseAST>($1);
        $$ = new MulExpAST(mul_exp);
    }
    | AddExp '+' MulExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op ="+";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new AddExpAST(left_exp, op.c_str(), right_exp);
    }
    |AddExp '-' MulExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op = "-";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new AddExpAST(left_exp, op.c_str(), right_exp);
    };


RelExp
    : AddExp {
        auto add_exp = std::unique_ptr<BaseAST>($1);
        $$ = new RelExpAST(add_exp);
    }
    | RelExp RELOP AddExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(left_exp, op->c_str(), right_exp);
    };
    | RelExp '<' AddExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op="<";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(left_exp, op.c_str(), right_exp);
    };
    | RelExp '>' AddExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        string op=">";
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(left_exp, op.c_str(), right_exp);
    };
    

EqExp
    : RelExp {
        auto rel_exp = std::unique_ptr<BaseAST>($1);
        $$ = new EqExpAST(rel_exp);
    }
    | EqExp EQOP RelExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new EqExpAST(left_exp, op->c_str(), right_exp);
    };

LAndExp
    : EqExp {
        auto eq_exp = std::unique_ptr<BaseAST>($1);
        $$ = new LAndExpAST(eq_exp);
    }
    | LAndExp LANDOP EqExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new LAndExpAST(left_exp, op->c_str(), right_exp);
    };

LOrExp
    : LAndExp {
        auto land_exp = std::unique_ptr<BaseAST>($1);
        $$ = new LOrExpAST(land_exp);
    }
    | LOrExp LOROP LAndExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new LOrExpAST(left_exp, op->c_str(), right_exp);
    };


%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
    extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    int len=strlen(yytext);
    int i;
    char buf[512]={0};
    for (i=0;i<len;++i)
    {
        sprintf(buf,"%s%d ",buf,yytext[i]);
    }
    fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}
