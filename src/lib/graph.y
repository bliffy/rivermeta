%code top{
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define YYDEBUG 1
}

%code requires{
#include <ast.h>
#include <list>
#include "wsqueue.h"

extern nhqueue_t *gFileQueue;
extern ASTNode *gASTRoot;
extern std::list<SymbolTable*> gSymTab;
extern uint32_t gInFuncDecl;
extern int pgdebug;
}


%union {
     ASTNode *node;
     char *sval;
     long long ival;
}

%code{

extern int pglex();
extern int pgerror(const char *);

nhqueue_t *gFileQueue;
ASTNode *gASTRoot = NULL;
std::list<SymbolTable*> gSymTab;
uint32_t gInFuncDecl = 0;

}


%token TK_INCLUDE
%token TK_LPAREN TK_RPAREN TK_LBRACE TK_RBRACE TK_PERIOD
%token TK_COLON TK_ENDSTMT TK_ATSIGN
%token TK_THREAD TK_FUNC TK_EXTERN TK_DOUBLEPIPE TK_PIPE
%token TK_ATDOUBLEPIPE TK_COMMA TK_ARROW TK_PERCENT
%token <sval> TK_WORD TK_STRINGLIT TK_VARREF
%token <ival> TK_NUMBER
%left TK_WORD TK_NUMBER TK_STRINGLIT
%type <ival> pipe_sep
%type <node> statement_list scoped_statements statement
%type <node> thread_decl func_decl func_header func_call extern_decl pipeline pipe_source
%type <node> sourceVars sourceVar varList varRef kid_list sinkVar kid_def
%type <node> optSourceVars optVarList
%type <sval> varFilter
%type <sval> varTarget
%%

root: statement_list { gASTRoot->addChild($1); };

statement_list: { $$ = new ASTStatementList(); }
              | statement_list statement TK_ENDSTMT { if ( $2 ) $1->addChild($2); $$ = $1; }
              | statement_list statement TK_RBRACE { if ( $2 ) $1->addChild($2); $$ = $1; yychar = TK_RBRACE; }
              | statement_list TK_ENDSTMT { $$ = $1; }
              ;

scoped_statements:  TK_LBRACE statement_list TK_RBRACE { $$ = $2; }
                 ;

statement: thread_decl { $$ = $1; }
         | func_decl { $$ = $1; }
         | func_call { $$ = $1; }
         | extern_decl { $$ = $1; }
         | pipeline { $$ = $1; }
         | include { $$ = NULL; }
         ;


include: TK_INCLUDE TK_WORD { queue_add(gFileQueue, $2); }
       | TK_INCLUDE TK_STRINGLIT { queue_add(gFileQueue, $2); }
       ;

thread_decl: TK_THREAD TK_LPAREN TK_NUMBER TK_COMMA TK_NUMBER TK_RPAREN scoped_statements { unsigned long long tid = (($3 << 32) | $5); $$ = new ASTThreadDecl(tid, true, $7); }
           | TK_THREAD TK_LPAREN TK_NUMBER TK_RPAREN scoped_statements { $$ = new ASTThreadDecl($3, false, $5); }
           ;

extern_decl: TK_EXTERN varList {
                    ASTVarList *l = (ASTVarList*)$2;
                    for ( size_t n = 0 ; n < l->childCount() ; n++ ) {
                         l->getVar(n)->setExtern();
                    }
                    delete l;
                    $$ = new ASTNULL();
               }
           ;

func_decl: func_header TK_LPAREN optVarList TK_ARROW varList TK_RPAREN scoped_statements {
                    ASTFuncDecl *decl = (ASTFuncDecl*)$1;
                    decl->addSource($3);
                    decl->addDests($5);
                    decl->addStatements($7);
                    decl->fixup();
                    gSymTab.pop_back();
                    gInFuncDecl--;
                    $$ = $1;
               }
         | func_header TK_LPAREN optVarList TK_RPAREN scoped_statements {
                    ASTFuncDecl *decl = (ASTFuncDecl*)$1;
                    decl->addSource($3);
                    decl->addDests(new ASTVarList());
                    decl->addStatements($5);
                    decl->fixup();
                    gSymTab.pop_back();
                    gInFuncDecl--;
                    $$ = $1;
               }
         ;

func_header: TK_FUNC TK_WORD {
                    ASTFuncDecl *decl = new ASTFuncDecl($2, gSymTab.back());
                    $$ = decl;
                    gSymTab.push_back(decl->getSymbolTable());
                    gInFuncDecl++;
               }
           ;

func_call: TK_PERCENT TK_WORD TK_LPAREN optSourceVars TK_ARROW varList TK_RPAREN { $$ = new ASTFuncCall($2, $4, $6, gSymTab.back()); }
         | TK_PERCENT TK_WORD TK_LPAREN optSourceVars TK_RPAREN { $$ = new ASTFuncCall($2, $4, new ASTVarList(), gSymTab.back()); }
         ;

pipeline: pipe_source kid_list sinkVar {
                    ASTVarList *svar = dynamic_cast<ASTVarList*>($1);
                    ASTKidDef  *skid = dynamic_cast<ASTKidDef*>($1);
                    ASTKidList *list = static_cast<ASTKidList*>($2);
                    if ( skid ) {
                        list->insertChild(skid);
                    }
                    if ( !list->hasChildren() ) {
                         /* $foo, $bar -> $baz  # is requested.  Insert a noop */
                         ASTKidDef *noop = new ASTKidDef(strdup("noop"));
                         noop->setInPipeType(ASTKidDef::PIPE);
                         list->insertChild(noop);
                    }
                    $$ = new ASTPipeline( svar, list, $3, (gInFuncDecl==0) );
               }
        ;


pipe_source: sourceVars { $$ = $1; }
           | kid_def { $$ = $1; }
           ;

sourceVars: sourceVar { $$ = new ASTVarList($1); }
          | sourceVars TK_COMMA sourceVar { $1->addChild($3); $$ = $1; }
          ;

optSourceVars: { $$ = new ASTVarList(); }
             | sourceVars { $$ = $1; }
             ;

sourceVar: varRef { $$ = $1; }
         | varRef varFilter { $$ = $1; ((ASTVar*)$1)->setFilter($2); }
         | varRef varFilter varTarget { $$ = $1; ((ASTVar*)$1)->setFilter($2);  ((ASTVar*)$1)->setTargetPort($3); }
         | varRef varTarget { $$ = $1; ((ASTVar*)$1)->setTargetPort($2); }
         | varRef varTarget varFilter { $$ = $1; ((ASTVar*)$1)->setFilter($3);  ((ASTVar*)$1)->setTargetPort($2); }
         ;

varFilter: TK_PERIOD TK_WORD { $$ = $2;; }
         ;

varTarget: TK_COLON TK_WORD { $$ = $2; }
         ;

varRef: TK_VARREF { $$ = new ASTVar($1, gSymTab.back()); }
      | TK_ATSIGN TK_VARREF { ASTVar *var = new ASTVar($2, gSymTab.back()); var->setBundle(); $$ = var; }
      ;

varList: varRef { $$ = new ASTVarList($1); }
       | varList TK_COMMA varRef { $1->addChild($3); $$ = $1; }
       ;

optVarList: { $$ = new ASTVarList(); }
          | varList { $$ = $1; }
          ;

sinkVar: TK_ARROW varRef { $$ = $2; }
       | { $$ = NULL; }
       ;


kid_list: { $$ = new ASTKidList(); }
        | pipe_sep kid_def kid_list {
               if ( $1 == (-1*ASTKidDef::DOUBLEPIPE) ) {
                    /* @|| was the separator.  Need bundle/unbundle */
                    ASTKidDef *bun = new ASTKidDef(strdup("bundle"));
                    ASTKidDef *unbun = new ASTKidDef(strdup("unbundle"));

                    bun->setInPipeType(ASTKidDef::PIPE);
                    unbun->setInPipeType(ASTKidDef::DOUBLEPIPE);
                    ((ASTKidDef*)$2)->setInPipeType(ASTKidDef::PIPE);
                    $3->insertChild($2);
                    $3->insertChild(unbun);
                    $3->insertChild(bun);
               } else {
                    ((ASTKidDef*)$2)->setInPipeType((ASTKidDef::KIDOUT)$1);
                    $3->insertChild($2);
               }
               $$ = $3;
          }
        ;

/* TODO:   PORT:kid */
kid_def: TK_WORD TK_COLON kid_def { ((ASTKidDef*)$3)->setSourcePort($1); $$ = $3; }
       | TK_WORD { $$ = new ASTKidDef($1); }
       | TK_WORD kid_def { ((ASTKidDef*)$2)->prefaceItem($1); $$ = $2; }
       | TK_NUMBER { $$ = new ASTKidDef($1); }
       | TK_NUMBER kid_def { ((ASTKidDef*)$2)->prefaceItem($1); $$ = $2; }
       | TK_STRINGLIT { $$ = new ASTKidDef($1); }
       | TK_STRINGLIT kid_def { ((ASTKidDef*)$2)->prefaceItem($1); $$ = $2; }
       ;

pipe_sep: TK_PIPE { $$ = ASTKidDef::PIPE; }
        | TK_DOUBLEPIPE { $$ = ASTKidDef::DOUBLEPIPE; }
        | TK_ATDOUBLEPIPE { $$ = -1* ASTKidDef::DOUBLEPIPE; }
        ;

%%

