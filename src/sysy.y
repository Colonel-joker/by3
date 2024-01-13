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
int IR::constdefing=0;

int IR::blocks=0;
int IR::uselessblocks=0;

int IR::globaldef=0;
int IR::while_end=-1;
int IR::while_cond=-1;
string IR::var_name="";
int IR::blockreturn=0;

whileinfo wi=whileinfo();
whileinfo* IR::curwi=&wi;
map<string,int> IR::constmap;
map<string,int> IR::globalname;

blockmap bmap=blockmap();
blockmap* IR::curbmap=&bmap;
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
%type <ast_val> FuncDef  FuncType Block Stmt  LVal VarDecl VarDefs VarDef ConstDef ConstDecl BlockItems BlockItem Decl ConstDefs
%type <int_val>  Number  
%type <ast_val> Exp Exps PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp 
%type <ast_val> CompUnits FuncParams FuncParam FuncRParams GlobalDef
%%
//CompUnit必须写在开头！！！！！

CompUnit
  : CompUnits {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->comp_units = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
;
CompUnits
  : GlobalDef{
    auto ast = new CompUnitsAST();
    ast->type = 1;
    ast->global_def = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | CompUnits GlobalDef   {
    auto ast = new CompUnitsAST();
    ast->type = 0;
    ast->comp_units = unique_ptr<BaseAST>($1);
    ast->global_def = unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;
GlobalDef
  : Decl{
    auto ast = new GlobalDefAST();
    ast->type=1;
    ast->decl = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | FuncDef{
    auto ast = new GlobalDefAST();
    ast->type=0;
    ast->func_def = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

//lv4  利用一一对应关系，funcType换成btype,ConstInitVal和constexp直接用exp
Decl
  : 
  ConstDecl {
    auto ast = new DeclAST();
    ast->type=0;
    ast->constdecl = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->type=1;
    ast->vardecl = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


ConstDecl
  : CONST FuncType ConstDefs ';' {
    auto ast = new ConstDeclAST();
    ast->btype = std::unique_ptr<BaseAST>($2);
    ast->constdefs = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;



ConstDefs : ConstDefs ',' ConstDef {
    auto ast = new ConstDefsAST();
    ast->type=0;
    ast->constdefs=std::unique_ptr<BaseAST>($1);
    ast->constdef=std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | ConstDef {
    auto ast = new ConstDefsAST();
    ast->type=1;
    ast->constdef=std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


ConstDef
    : IDENT '=' Exp {  
        
        auto exp = std::unique_ptr<BaseAST>($3);
        $$ = new ConstDefAST($1->c_str(), exp);
    }
    ;

VarDecl
  : FuncType VarDefs ';' {
    auto ast = new VarDeclAST();
    ast->btype = std::unique_ptr<BaseAST>($1);
    ast->vardefs = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

VarDefs
  : VarDefs ',' VarDef {
    auto ast = new VarDefsAST();
    ast->type=0;
    ast->vardefs=std::unique_ptr<BaseAST>($1);
    ast->vardef=std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | VarDef {
    auto ast = new VarDefsAST();
    ast->type=1;
    ast->vardef=std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;



VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->type=0;
    ast->ident = *std::unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' Exp {
    auto ast = new VarDefAST();
    ast->type=1;
    ast->ident = *std::unique_ptr<string>($1);
    ast->exp = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LVal
    : IDENT {
        $$ = new LValAST($1->c_str(),0);

    }
    //| IDENT Exps{//数组
   //   auto exps = std::unique_ptr<BaseAST>($2);
   //   $$ = new LValAST($1->c_str(),exps,1);
   // }

    ;

Exps
  : '[' Exp ']' {
    auto ast = new ExpsAST();
    ast->type=0;
    ast->exp=std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | Exps '[' Exp ']' {//数组
    auto ast = new ExpsAST();
    ast->type=1;
    ast->exps=std::unique_ptr<BaseAST>($1);
    ast->exp=std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

BlockItems : BlockItem {
    auto ast = new BlockItemsAST();
    ast->type=1;
    ast->blockitem=std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | BlockItems BlockItem {
    auto ast = new BlockItemsAST();
    ast->type=0;
    ast->blockitems=std::unique_ptr<BaseAST>($1);
    ast->blockitem=std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
    ;

BlockItem :  Decl {
    auto ast = new BlockItemAST();
    ast->type=0;
    ast->decl = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;
  | Stmt {
    auto ast = new BlockItemAST();
    ast->type=1;
    ast->stmt = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->type=0;
    $$ = ast;
  }
  | FuncType IDENT '(' FuncParams ')' Block {
    auto ast = new FuncDefAST();
    ast->type=1;
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->func_params = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;
FuncParams
  : FuncParams ',' FuncParam {
    auto ast = new FuncParamsAST();
    ast->type=0;
    ast->funcparams=std::unique_ptr<BaseAST>($1);
    ast->funcparam=std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | FuncParam {
    auto ast = new FuncParamsAST();
    ast->type=1;
    ast->funcparam=std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

FuncParam
  : FuncType IDENT{
    auto ast = new FuncParamAST();
    ast->type=0;
    ast->btype=std::unique_ptr<BaseAST>($1);
    ast->ident=*unique_ptr<string>($2);
    $$ = ast;
  }
  | FuncType IDENT '[' ']'{
    auto ast = new FuncParamAST();
    ast->type=1;
    ast->btype=std::unique_ptr<BaseAST>($1);
    ast->ident=*unique_ptr<string>($2);
    $$ = ast;
  }
  | FuncType IDENT '[' ']' Exps{
    auto ast = new FuncParamAST();
    ast->type=2;
    ast->btype=std::unique_ptr<BaseAST>($1);
    ast->ident=*unique_ptr<string>($2);
    ast->exps=std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
        $$ = new FuncTypeAST("int");
    } | VOID {
        $$ = new FuncTypeAST("void");
    }
  ;



Block
  : '{' BlockItems '}' {
    auto stmt=std::unique_ptr<BaseAST>($2);
    $$ = new BlockAST(stmt);
  }
  | '{' '}'{
    $$ = new BlockAST();
  }




Stmt
  : LVal '=' Exp ';'{
    auto ast = new StmtAST();
    ast->type=0;
    ast->lval = std::unique_ptr<BaseAST>($1);
    ast->exp = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type=1;
    ast->exp = std::unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->type=2;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->type=3;
    ast->block = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->type=4;
    ast->exp = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->type=5;
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type=6;
    ast->exp=std::unique_ptr<BaseAST>($3);
    ast->true_stmt=std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->type=7;
    ast->exp=std::unique_ptr<BaseAST>($3);
    ast->true_stmt=std::unique_ptr<BaseAST>($5);
    ast->false_stmt=std::unique_ptr<BaseAST>($7);
    $$ = ast;
  } 
  | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type=8;
    ast->exp=std::unique_ptr<BaseAST>($3);
    ast->true_stmt=std::unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';'{
    auto ast = new StmtAST();
    ast->type=9;
    $$ = ast;
  }
  | CONTINUE ';'{
    auto ast = new StmtAST();
    ast->type=10;
    $$ = ast;
  };

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
    | LVal {
        auto lval = std::unique_ptr<BaseAST>($1);
        $$ = new PrimaryExpAST(lval,2);
    }
    ;

Number
    : INT_CONST {
        $$=int($1);
    }
    ;



FuncRParams
  : FuncRParams ',' Exp {
    auto ast=new FuncRParamsAST();
    ast->type=0;
    ast->funcrparams = std::unique_ptr<BaseAST>($1);
    ast->exp = std::unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Exp {
    auto ast=new FuncRParamsAST();
    ast->type=1;
    ast->exp = std::unique_ptr<BaseAST>($1);
    $$ = ast;
  }

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
    | IDENT '(' ')'{
    $$ = new UnaryExpAST($1->c_str());
    }
    | IDENT '(' FuncRParams ')'{
      auto funcrparams = std::unique_ptr<BaseAST>($3);
      $$ = new UnaryExpAST($1->c_str(),funcrparams,3);
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
