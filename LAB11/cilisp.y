%{
    #include "cilisp.h"
    #define ylog(r, p) {printf("BISON: %s ::= %s \n", #r, #p);}
    int yylex();
    void yyerror(char*, ...);
%}

%union {
    char *ident;
    double dval;
    int ival;
    struct ast_node *astNode;
    struct symbol_table_node *symNode;
};

%token <ival> FUNC TYPE
%token <dval> INT
%token <dval> DOUBLE
%token <ident> SYMBOL
%token QUIT EOL EOFT COND
%token LPAREN RPAREN LET LAMBDA

%type <astNode> s_expr f_expr number s_expr_list
%type <astNode> s_expr_section
%type <symNode> let_elem let_list let_section arg_list

%%

program:
    s_expr EOL {
        //ylog(program, s_expr EOL);
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
        YYACCEPT;
    }
    | s_expr EOFT {
        //ylog(program, s_expr EOFT);
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
        exit(EXIT_SUCCESS);
    }
    | EOL {
        //ylog(program, EOL);
        YYACCEPT;  // paranoic; main skips blank lines
    }
    | EOFT {
        //ylog(program, EOFT);
        exit(EXIT_SUCCESS);
    };


s_expr:
    QUIT {
        //ylog(s_expr, QUIT);
        exit(EXIT_SUCCESS);
    }
    | f_expr {
        //ylog(s_expr, f_expr);
        $$ = $1;
    }
    | number {
        //ylog(s_expr, number);
        $$ = $1;
    }
    | SYMBOL {
        //ylog(s_expr, SYMBOL);
        $$ = createSymbolNode($1);
    }
    | LPAREN let_section s_expr RPAREN {
        //ylog(s_expr, let_section);
        $$ = createScopeNode($2, $3);
    }
    | LPAREN COND s_expr s_expr s_expr RPAREN {
        //ylog(s_expr, COND);
        $$ = createConditionNode($3, $4, $5);
    }
    | error {
        //ylog(s_expr, error);
        yyerror("unexpected token");
        $$ = NULL;
    };

let_section:
    LPAREN LET let_list RPAREN {
        //ylog(let_section, let_list);
        $$ = $3;
    };

let_list:
    let_elem {
        //ylog(let_list, let_elem);
        $$ = $1;
    }
    | let_elem let_list {
        //ylog(let_list, let_list);
        $$ = createLetList($1, $2);
    };

let_elem:
    LPAREN SYMBOL s_expr RPAREN {
        //ylog(let_elem, SYMBOL);
        //ylog(let_elem, s_expr);
        $$ = createLetElem(VAR_TYPE, NO_TYPE, $2, $3, NULL);
    }
    | LPAREN TYPE SYMBOL s_expr RPAREN {
        $$ = createLetElem(VAR_TYPE, $2, $3, $4, NULL);
    }
    | LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        $$ = createLetElem(LAMBDA_TYPE, NO_TYPE, $2, $7, $5);
    }
    | LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        $$ = createLetElem(LAMBDA_TYPE, $2, $3, $8, $6);
    };

f_expr:
      LPAREN FUNC s_expr_section RPAREN {
        //ylog(f_expr, s_expr_section);
        $$ = createFunctionNode(NULL, $2, $3);
    }
    | LPAREN SYMBOL s_expr_section RPAREN {
        //ylog(f_expr, s_expr_section);
        $$ = createFunctionNode($2, CUSTOM_FUNC, $3);
    };

s_expr_section:
    s_expr_list {
        //ylog(s_expr_section, s_expr_list);
        $$ = $1;
    }
    | /*empty*/ {
        //ylog(s_expr_section, empty);
        $$ = NULL;
    };

s_expr_list:
    s_expr {
        //ylog(s_expr_list, s_expr);
        $$ = $1;
    }
    | s_expr s_expr_list {
        //ylog(s_expr, s_expr_list);
        $$ = addExpressionToList($1, $2); 
    };

arg_list:
    SYMBOL {
        //ylog(arg_list, SYMBOL);
        $$ = createArg($1);
    }
    | SYMBOL arg_list {
        //ylog(arg_list, arg_list);
        $$ = createArgList($1, $2);
    }
    | /*empty*/ {
        $$ = NULL;
    };    

number:
      INT {
        //ylog(number, INT);
        $$ = createNumberNode($1, INT_TYPE);
    }
    | DOUBLE {
        //ylog(number, DOUBLE);
        $$ = createNumberNode($1, DOUBLE_TYPE);
    };
%%

