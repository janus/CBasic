#define NumberValue 1
#define StringValue 2
#define V_value 3
#define V_operator 4
#define V_expression 5
#define V_call 6


#define S_ifthen  6
#define S_goto 7
#define S_input 8
#define S_print 9
#define S_assign 10
#define S_array 11
#define S_for 12
#define S_gosub 13
#define S_return 14
#define S_next 15

#define CTRUE 20
#define CFALSE 21
int statsize = 1;
int trastatpos = 0;
int chpos = 0;
char tknlb [30][20];
int  dim = 30;
int  currentpos = 0;
int sizeelem = 0;
int savesize[5];
int setinjump = 0;
int fntabidx = 0;

int untilReturn = 0;

#define FN_SIZE 100

#define STAT_MAX 3000

#define STACK_SIZE 200


struct optoken
{
    char cop;
    int nop;
};


struct CExpression* expression();
struct CExpression* catomic ();
struct CExpression* coperator();
struct CExpression* condexpression();
//Various states token might be in.
enum TokenizeState {
      TS_DEFAULT , TS_WORD, TS_NUMBER, TS_STRING, TS_COMMENT};

// Token types
enum TokenType {
      TT_WORD = 20, TT_NUMBER, TT_STRING, TT_LABEL, TT_LINE,
      TT_EQUAL, TT_OPERATOR, TT_LEFT_PAREN, TT_RIGHT_PAREN, 
      TT_GREATEROREQUAL, TT_LESSOREQUAL,TT_REQUAL, TT_NOTEQUAL,
      TT_GREATER, TT_LESS,
      TT_COMMA, TT_TT_EOF};


struct optoken aroptoken[12] = {{'\n', TT_LINE}, {'=', TT_EQUAL},{'+', TT_OPERATOR},{'-', TT_OPERATOR},{'*',TT_OPERATOR},{'/', TT_OPERATOR},{'%', TT_OPERATOR},{',', TT_COMMA}, 
                            {'<', TT_OPERATOR},{'>', TT_OPERATOR}, {'(', TT_LEFT_PAREN}, {')', TT_RIGHT_PAREN}};
const int aroptokenlen = 12;

enum Reword {
	DIM, FOR, PRINT, DEF, IF, READ, INPUT, NEXT, RETURN, STOP, END, GOTO, GOSUB, THEN, LET, DATA, TO,STEP, REM};


typedef struct  Clist
{
    char* text;
    int type;
    struct Clist* next;

}Clist;

typedef struct CStringValue
{
    char* value;
}CStringValue;

typedef struct CNumberValue
{
    double value;
}CNumberValue;

typedef struct CValue
{
    int vtype;
    void *  value;
}CValue;

typedef struct LExpression
 {
	struct CExpression* arg;
	struct LExpression* next;
} LExpression;

typedef struct FuncCall
{
	int argSize;
	struct LExpression* LexpList;
	char *name;
	int tableIndex;
} FuncCall;

struct CExpression
{
    int extype;
    union
    {
        struct COperatorExpression* operatorexpression;
        struct CVariableExpression* variableexpression;
        CValue* valueexpression;
        FuncCall* funCall;

    };
};


struct COperatorExpression
{
    struct CExpression* left;
    struct CExpression* right;
    int coperator;

};

struct CVariableExpression
{
    char* name;
};

typedef struct env
{
    char* key;
    CValue* vval;
    struct env* next;
}env;

typedef struct CPrintStatement
{
    struct LExpression* expression;
}CPrintStatement ;

typedef struct CInputStatement
{
    char* name;

}CInputStatement;

typedef struct CAssignStatement
{
    char* name;
    struct CExpression* value;
}CAssignStatement;

typedef struct CGotoStatement
{
    char* label;
    int nlab;

}CGotoStatement;

typedef struct CGosubStatement
{
    int label;
    int jumpto;
}CGosubStatement;


typedef struct CNextStatement
{
	struct CExpression* var;
}CNextStatement;

typedef struct CReturnStatement
{
	char dummy;
}CReturnStatement;

typedef struct CIfThenStatement
{
    struct CExpression* condition;
    char* label;

}CIfThenStatement;

typedef struct CArrayStatement
{
    struct CExpression* exp;
    struct CExpression* opt;
    char* varname;
}CArrayStatement;


typedef struct CStatement
{
    int stype;
    void* statement;
    struct CStatement* next;

}CStatement;

typedef struct CForStatement
{
    struct CExpression* indexpress;
    struct CExpression* endex;
    CStatement* lforstat;
    env* lenv;
    int ssize;
    char* index;
}CForStatement;


CForStatement* forstat;

typedef struct CSave
{
    CStatement* shead;
    CStatement* stail;
}CSave;

typedef struct CSFor
{
    CForStatement* lcfor;

}CSFor;

typedef struct parList
{
	struct CExpression* name;
	struct parList* next;
} parList;

typedef struct FuncAbstraction
{
	int parSize;
	parList* plist;
	struct CExpression* body;
	char* name;
} FuncAbstraction;

typedef struct Clabels
{
    int key;
    int cindex;
    CStatement* st; 
}Clabels;

struct labelList
{
	int top;
	Clabels* clabs[STAT_MAX];
};

struct labels
{
    int stsize;
    char* label;
    struct labels* next;
};

typedef struct ReturnStack
{
    int idx;
    int stack[STACK_SIZE];
}ReturnStack;
