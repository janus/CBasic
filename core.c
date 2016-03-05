/*
 Copy Right:: Freely we received , freely do we give. By Emeka 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define NumberValue 1
#define StringValue 2
#define V_value 3
#define V_operator 4
#define V_expression 5
#define S_ifthen  6
#define S_goto 7
#define S_input 8
#define S_print 9
#define S_assign 10
#define S_array 11
#define S_for 12


#define DIM 50
#define NEXT 51
#define FOR 52
#define TO 53
#define PRINT 54
#define IF 55
#define INPUT 56
#define GOTO 57
#define THEN 58
#define LET 59
#define STEP 60
#define READ 61
#define DATA 62
#define END 63

#define ASSIGN_STRING(o,p,v,s) o->p = malloc(s * sizeof(char)); memcpy( (char*) o->p,v,s);


#define CAPACITY 100

int statsize = 1;
int trastatpos = 0;
int chpos = 0;
char tknlb [30][20];
int  dim = 30;
int  currentpos = 0;
int sizeelem = 0;
int savesize[5];
int setinjump = 0;
int chrule = 0;
int fc = 0;
int afirst = 0;
int ascendingnum = 0;
int lfirst = 0;
int suspend = 0;

struct optoken
{
    char cop[3];
    int nop;
};

struct opcodeWord
{
	char word[9];
	int opcode;
};

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
    int kind;
    void *  value;
}CValue;

struct CExpression
{
    int kind;
    union
    {
        struct COperatorExpression* operatorexpression;
        struct CVariableExpression* variableexpression;
        CValue* valueexpression;

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

typedef struct Cvariables
{
    struct CExpression* key;
    CValue* vval;
    struct Cvariables* next;
}Cvariables;


typedef struct CPrintStatement
{
    struct CExpression* expression;
}CPrintStatement ;

typedef struct CInputStatement
{
    struct CExpression* name;
}CInputStatement;

typedef struct CAssignStatement
{
    struct CExpression* name;
    struct CExpression* value;
}CAssignStatement;

typedef struct CGotoStatement
{
    struct CExpression* label;
}CGotoStatement;

typedef struct CIfThenStatement
{
    struct CExpression* condition;
    struct CExpression* lnum;
}CIfThenStatement;

typedef struct CAElements
{
    struct CExpression* valexp;
    struct CExpression* varname;
    struct CAElements* next;
}CAElements;
	
typedef struct CArrayStatement
{
	CAElements* elem;
	CAElements* ttar;
    int nelements;
    int upcount;
}CArrayStatement;

typedef struct CStatement
{
    int kind;
    void* statement;
    struct CStatement* next;
}CStatement;

typedef struct CForStatement
{
    struct CExpression* initexp;
    struct CExpression* endexp;
    struct CExpression* indexp;
    struct CExpression* step;
    CStatement* lforstat;    
}CForStatement;

typedef struct NStatement 
{
	int statnum;
	CStatement* stats;
}NStatement ;

typedef struct LStatement
{
	NStatement* nstats;
	int nelem;
	int capacity;
	
}LStatement;
	
	
Cvariables* henv = 0;
Cvariables* tenv = 0;
/*This is the base for env */
Cvariables** foo = 0;
CStatement* hstat = 0;
CStatement* tstat = 0;
CStatement* currentstat = 0;
LStatement* lstatement ;
CArrayStatement* arrhead ;
CArrayStatement* arraypos = NULL ;
CAElements* trackelem = NULL;


struct CExpression* expression();
struct CExpression* catomic ();
struct CExpression* coperator();

/*Various states token might be in.*/
enum TokenizeState {
      TS_DEFAULT , TS_WORD, TS_NUMBER, TS_STRING, TS_COMMENT};

/*Token types */
enum TokenType {
      TT_WORD , TT_NUMBER, TT_STRING, TT_LABEL, TT_LINE,TT_CONCAT_OPERATOR,
      TT_EQ_OPERATOR, TT_PLUS_OPERATOR, TT_MINUS_OPERATOR, TT_MULT_OPERATOR,TT_DIV_OPERATOR,TT_LE_OPERATOR,TT_LEQ_OPERATOR,
      TT_GE_OPERATOR, TT_MOD_OPERATOR, TT_ASSIGN_OPERATOR,TT_GEQ_OPERATOR,TT_LEFT_PAREN, TT_RIGHT_PAREN, TT_EOF,TT_NO_OP};

struct optoken aroptoken[15] = {{"\n", TT_LINE}, {"==", TT_EQ_OPERATOR},{"+", TT_PLUS_OPERATOR},{"-", TT_MINUS_OPERATOR},{"*",TT_MULT_OPERATOR},{"/", TT_DIV_OPERATOR},
                            {"<", TT_LE_OPERATOR},{"<=", TT_LEQ_OPERATOR},{">", TT_GE_OPERATOR}, {"=", TT_ASSIGN_OPERATOR},{"%", TT_MOD_OPERATOR},
                            {">=", TT_GEQ_OPERATOR},{"(", TT_LEFT_PAREN}, {")", TT_RIGHT_PAREN} , {",", TT_CONCAT_OPERATOR}};
const int aroptokenlen = 15;


struct opcodeWord  opword[]  = {{"END", END}, {"NEXT", NEXT}, {"IF", IF}, {"THEN", THEN}, {"GOTO", GOTO},{"INPUT", INPUT},{"READ", READ},
								{"PRINT", PRINT}, {"TO", TO}, {"FOR", FOR}, {"DIM", DIM}, {"DATA", DATA},
								{"LET", LET},{"STEP", STEP}, {"NULL", 0}};

int getoptype (char* cop)
{
    int i;
    for(i = 0; i < aroptokenlen; i++)
    {
        if(strcmp(cop, aroptoken[i].cop) == 0)
            return aroptoken[i].nop;
    }
    return TT_NO_OP;

}

int getpriWord(char* sword)
{
	int i = 0;
	while(1)
	{
		if(strcmp(opword[i].word, sword) == 0)
		{
			return opword[i].opcode;
			
		}
		if(strcmp(opword[i].word, "NULL") == 0)
		{
			return 0;
		}
		i++;
	}
	
}

void cerror (void* obj, char* message)
{
    if(!obj)
	{
		printf("\n %s\n", message);
		exit(1);
	}
}

void insertlabel(char* label)
{
    if(currentpos < dim)
	{
		strcpy(tknlb[currentpos++], label);
	}
    return;
}

int contains (char* label)
{
    int i = 0;
    while(i < currentpos)
	{
		if(strcmp(label, tknlb[i] ) == 0)
		{
			return 1;
		}
		i++;
	}
    return 0;
}

Clist* head  = 0;
Clist* tail = 0;
Clist* stepback = 0;
Clist* currenttoken = 0;
Clist* double_step_back = 0;


struct CExpression* makenumval(double val)
{
    CNumberValue* dval;
    CValue* result;
    struct CExpression* rstexpress;
    dval= (CNumberValue *) malloc(sizeof(CNumberValue));
    result = (CValue* ) malloc(sizeof(CValue));
    rstexpress = (struct CExpression* ) malloc(sizeof(struct CExpression*));
    if(!dval || !rstexpress || !result)
	{
		cerror(0,"ERROR---Not enough space" );
	}
    dval->value = val;
    result->kind = NumberValue;
    result->value = dval;
    rstexpress->kind = V_value;
    rstexpress->valueexpression = result;
    return rstexpress;
}


struct CExpression* makestrval(char* str)
{
    CStringValue* sval;
    CValue* result;
    int lstr = strlen(str) + 1;
    struct CExpression* rstexpress;
    sval = (CStringValue *)malloc(sizeof(CStringValue));
    result = (CValue*)malloc(sizeof(CValue));
    rstexpress = (struct CExpression*)malloc(sizeof(struct CExpression));
    if(!sval || !result || !rstexpress)
	{
		cerror(0,"ERROR---Not enough space" );
	}
    ASSIGN_STRING(sval,value,str, lstr);
    sval->value[lstr] = '\0';
    result->kind = StringValue;
    result->value = sval;
    rstexpress->kind = V_value;
    rstexpress->valueexpression = result;
    return rstexpress;
}


void nexttk()
{
	double_step_back = stepback;
	stepback = currenttoken;
	currenttoken = currenttoken->next;
}
	
 Clist* consume (int typ)
{
    if(currenttoken->type != typ)
    {
        printf("ERROR---Expected type %i", typ);
        exit(1);
    }
    if(!currenttoken)
    {	
        printf("ERROR--You have reached the end");
        exit(1);			
	}
    nexttk();
    return currenttoken;

}

int match(int typ)
{
	if(currenttoken->type != typ)
	{
		return 0;
	}
	if (TT_RIGHT_PAREN == typ)
	{
		printf("THis is where the issue is");
	}
	nexttk();
	return 1;
}

void expected(int typ)
{
	if(currenttoken->type != typ)
	{
        printf("ERROR--Parser failed: syntax error ");
        exit(1);		
	}
	nexttk();
}
 
int peek(int typ)
{
	if(currenttoken->type != typ)
	{
		return 0;
	}
	return 1;
}

struct CExpression* makeoperatorexpress(struct CExpression* left, int op, struct CExpression* right)
{
    struct COperatorExpression* cop ;
    struct CExpression* result;
    cop = (struct COperatorExpression*)malloc(sizeof(struct COperatorExpression));
    result = (struct CExpression* ) malloc(sizeof(struct CExpression));  
    if(!cop || !result)
	{
		cerror(0,"ERROR---Not enough space" );
	}
    cop->left = left;
    cop->right = right;
    cop->coperator = op;
    result->kind = V_operator;
    result->operatorexpression = cop;
    return result;
}


struct CExpression* makevarexpression(char* text)
{
    struct CVariableExpression* temp;
    struct CExpression* result;
    int lstr = strlen(text) + 1;
    temp = (struct CVariableExpression* ) malloc(sizeof(struct CVariableExpression));
    result = (struct CExpression* ) malloc(sizeof(struct CExpression));
    if(!temp || !result )
	{
		cerror(0, "ERROR---Not enough space");
	}
    ASSIGN_STRING(temp,name,text,lstr);
    temp->name [lstr] = '\0';
    result->kind = V_expression;
    result->variableexpression = temp;
    return result;
}


CStatement* lookupstat(int num)
{
	int i;
	for(i = 0; i < lstatement->nelem; i++)
	{
		
		if (num ==  lstatement->nstats[i].statnum)
		{
			return lstatement->nstats[i].stats;
		}
	}
	cerror(0, "ERROR--statement looking for is not in the program");
}
		
void addstat(void* statf, int kind, int pnum)
{
	if(pnum < 0 )
	{
		printf("ERROR -- statement identification line number should be postive integer");
		exit(1);
	}
	if ((ascendingnum != 0) && pnum < ascendingnum) 
	{
		if(!chrule)
		{
			printf("ERROR -- previous (%i), present val (%i) statement ident line number in increasing order", ascendingnum, pnum);
			exit(1);
		}
		chrule = 0;
	}
	if(chrule && pnum < ascendingnum)
	{
		printf("ERROR -- previous (%i), present val (%i) statement ident line number in increasing order", ascendingnum, pnum);
		exit(1);
	}
	ascendingnum = pnum;
    CStatement* ttstat = (CStatement *)malloc(sizeof(CStatement));
    cerror(ttstat, "ERROR--not enough memory");
    ttstat->statement = (CStatement *)statf;
    ttstat->kind = kind;
    ttstat->next = 0;
    if(!afirst)
    {
		lstatement = (LStatement *)malloc(sizeof(LStatement));
		cerror(lstatement, "ERROR--not enough memory");
		lstatement->capacity = CAPACITY;
		lstatement->nelem = 1;
		lstatement->nstats = (NStatement *)malloc(sizeof(NStatement)* lstatement->capacity) ;
		cerror(lstatement->nstats, "ERROR--not enough memory");
		lstatement->nstats[lstatement->nelem -  1].stats = ttstat;
		lstatement->nstats[lstatement->nelem - 1].statnum = pnum;
        afirst = 1;
    }
    else
    {
		if (lstatement->nelem + 1  == lstatement->capacity)
		{
			LStatement* lnew =  (LStatement *)malloc(sizeof(LStatement));
    		cerror(lnew, "ERROR--not enough memory");
			lnew->capacity = lstatement->capacity * 2;
			lnew->nstats = (NStatement *)malloc(sizeof(NStatement)* lnew->capacity) ;
			int i;
			cerror(lnew->nstats, "ERROR--not enough memory");
			for(i = 0; i < lstatement->nelem - 2; i++)
			{
				lnew->nstats[i].stats = lstatement->nstats[i].stats;
				lnew->nstats[i].statnum = lstatement->nstats[i].statnum;
			}
			lnew->nelem = lstatement->nelem;
		    free(lstatement->nstats);
		    free(lstatement);
		    lstatement = lnew;
		}
		lstatement->nelem++;
		lstatement->nstats[lstatement->nelem -  1].stats = ttstat;
		lstatement->nstats[lstatement->nelem - 1].statnum = pnum;	
	}
}


CIfThenStatement* makeifthenstat(struct CExpression* exp, struct CExpression* dirval)
{
    CIfThenStatement* ifstat = (CIfThenStatement*)malloc(sizeof(CIfThenStatement));
    cerror(ifstat,"ERROR-not enough memory to make IF statement" );
    ifstat->condition = exp;
    ifstat->lnum = dirval;
    return ifstat;
}

CGotoStatement* makegotostat(struct CExpression* namelabel)
{
    CGotoStatement* gotostat = (CGotoStatement*)malloc(sizeof(CGotoStatement));
    cerror(gotostat, "ERROR-not enough memory to make GOTO statement");
    gotostat->label = namelabel;
    return gotostat;
}

CAssignStatement* makeassignstat(struct CExpression* name, struct CExpression* value)
{
    CAssignStatement* assignstat = (CAssignStatement*)malloc(sizeof(CAssignStatement));
    cerror(assignstat, "ERROR-not enough memory to make assignment");
    assignstat->value = value;
    assignstat->name = name;
    return assignstat;
}


CForStatement* makeforstat(CStatement* stats,  struct CExpression* initexpr, struct CExpression* endexpr, struct CExpression* indexpr, struct CExpression* fstep)
{
    CForStatement* forstat = (CForStatement*)malloc(sizeof(CForStatement));
    cerror(forstat, "ERROR-not enough memory assignment");
    forstat->step = fstep;
    forstat->indexp = indexpr;
    forstat->endexp = endexpr;
    forstat->initexp = initexpr;
    forstat->lforstat = stats;
    return forstat;
}

CInputStatement* inputstat(struct CExpression* name)
{
    CInputStatement* instat = (CInputStatement*)malloc(sizeof(CInputStatement));
    cerror(instat, "ERROR-not enough memory assignment");
    instat->name = name;
    return instat;
}

CPrintStatement* printstat(struct CExpression* exp)
{
    CPrintStatement* pstat = (CPrintStatement*)malloc(sizeof(CPrintStatement));
    cerror(pstat, "ERROR-not enough memory assignment");
    pstat->expression = exp;
    return pstat;
}


CValue* lookup(struct CExpression* key, Cvariables** varenv)
{
    Cvariables* varhead =  *varenv;
    while(varhead)
    {
        if(strcmp((char*)varhead->key->variableexpression->name,(char*)key->variableexpression->name) == 0)
        {
            return varhead->vval;
        }
        varhead = varhead->next;
    }
    return 0;
}

int tfirst = 0;

void putvar(struct CExpression* key , CValue* val)
{
    Cvariables* lenv = (Cvariables *)malloc(sizeof(Cvariables));
    cerror(lenv, "ERROR-not enough memory assignment");
    lenv->vval = val;
    lenv->next = 0;
    lenv->key = key;
    if(!tfirst)
    {
    henv = lenv;
    tenv = lenv;
    }
    else
    {
        tenv->next = lenv;
        tenv = lenv;
    }

}


CValue* eval(struct CExpression* express, struct Cvariables** varenv)
{
    switch(express->kind)
    {
		case V_expression:
		{
			/*struct CVariableExpression* varexpress = express->variableexpression;*/
			CValue* cval = lookup(express, varenv);
			if(cval)
			{
				switch(cval->kind)
				{  
					case NumberValue:
						return cval;
							
					case StringValue:
						return cval;
							
					default:
						cerror(0, "ERROR--unexpected value type");
						exit(1);
				}
			}
            else
            {
                return 0;
            }

        }
        case V_value:
        {
            CValue* vval = express->valueexpression;
            switch(vval->kind)
            {
                case NumberValue:
                    return vval;

                case StringValue:
                    return vval;
                   
                default:
                    cerror(0, "ERROR--unexpected value type");
                    exit(1);
            }

        }
        case V_operator:
        {
			struct COperatorExpression* cope = express->operatorexpression;
			CValue* leftval, *rightval, *rop;
			leftval = eval(cope->left, varenv);
			rightval = eval(cope->right, varenv);
			rop   =    (CValue*)malloc(sizeof(CValue));
			CNumberValue* dlvalue,*drvalue, *detemp;
			CStringValue* slvalue,*srvalue ;
            cerror(rop, "ERROR--not enough memory to create CValue");
            switch(cope->coperator)
			{
                case TT_EQ_OPERATOR:
				{
					if(leftval->kind == NumberValue && rightval->kind == NumberValue)
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						cerror(detemp, "ERROR--not enough memory to create CNumberValue");
						if(dlvalue->value == drvalue->value)
						{
							detemp->value = 1.0;
							rop->value = detemp;
							return rop;
						}
						else
						{
							detemp->value = 0.0;
							rop->value = detemp;
							return rop;
                        }
					}
					else if (leftval->kind == StringValue && rightval->kind == StringValue)
                    {
						rop->kind = StringValue;
						slvalue = (CStringValue*)leftval->value;
						srvalue = (CStringValue*)rightval->value;
						detemp =  (CNumberValue* )malloc(sizeof(CNumberValue));
						cerror(detemp, "ERROR--not enough memory to create CNumberValue");
						if(strcmp((char*)slvalue->value, (char*)srvalue->value) == 0)
						{
							detemp->value = 1.0;
							rop->value = detemp;
							return rop;
						}
						else
						{
							detemp->value = 0.0;
							rop->value = detemp;
							return rop;
						}
					}
					else 
					{
						printf("The types must the same and it could only be number and string");
						exit(1);
                    }
				}
                case TT_PLUS_OPERATOR:
				{
					if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp =  (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = dlvalue->value + drvalue->value;
						rop->value = detemp;
						return rop;
					}
					if((leftval->kind == StringValue)&&(rightval->kind == StringValue))
					{
						rop->kind = StringValue;
						slvalue = (CStringValue*)leftval->value;
						srvalue = (CStringValue*)rightval->value;
						CStringValue* temp = (CStringValue*)malloc(sizeof(CStringValue));
						int llstr = strlen(slvalue->value);
						int rlstr = strlen(srvalue->value);
						int lstr = llstr + rlstr + 1;
						ASSIGN_STRING(temp, value, slvalue->value, llstr);
						strncpy(temp->value + llstr, srvalue->value, rlstr);
						temp->value [lstr] = '\0';
						rop->value = temp;
						return rop;
					}
                    cerror(0,"ERROR--Don't mix up types" );
				}
                case TT_MINUS_OPERATOR:
				{
					if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = dlvalue->value - drvalue->value;
						rop->value = detemp;
						return rop;
					}
					cerror(0,"ERROR--Don't mix up types" );
				}
                case TT_MULT_OPERATOR:
				{
					if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = dlvalue->value * drvalue->value;
						rop->value = detemp;
						return rop;
					}
					cerror(0, "ERROR--Don't mix up types");
				}
                case TT_DIV_OPERATOR:
                {
                    if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						if (drvalue->value == 0.0)
						{
							cerror(0,"ERROR---division by zero is not allowed" );
						}
						detemp->value = dlvalue->value / drvalue->value;
						rop->value = detemp;
						return rop;
					}
					cerror(0,"ERROR--Don't mix up types" );
				}
                case TT_MOD_OPERATOR:
				{
                    if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						if (drvalue->value == 0.0)
						{
							cerror(0,"ERROR---Modulo by zero is not allowed" );
						}
						detemp->value = (int)dlvalue->value % (int)drvalue->value;
						rop->value = detemp;
						return rop;
					}
					cerror(0, "ERROR--Don't mix up types");
				}
                case TT_LE_OPERATOR:
				{
					if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = (dlvalue->value < drvalue->value) ? 1.0 : 0.0;
						rop->value = detemp;
						return rop;
					}
				    if((leftval->kind == StringValue)&& (rightval->kind == StringValue))
					{
					    rop->kind = NumberValue;
						slvalue = (CStringValue*)leftval->value;
						srvalue = (CStringValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = (strcmp((char*)slvalue->value, (char*)srvalue->value) < 0)? 1.0 : 0.0;
						rop->value = detemp;
						return rop;
					}
				    cerror(0,"ERROR--Don't mix up types");
				}
				case TT_GE_OPERATOR:
				{
					if((leftval->kind == NumberValue)&& (rightval->kind == NumberValue))
					{
						rop->kind = NumberValue;
						dlvalue = (CNumberValue*)leftval->value;
						drvalue = (CNumberValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = (dlvalue->value > drvalue->value) ? 1.0 : 0.0;
						rop->value = detemp;
						return rop;
					}
					if((leftval->kind == StringValue)&& (rightval->kind == StringValue))
					{
						rop->kind = NumberValue;
						slvalue = (CStringValue*)leftval->value;
						srvalue = (CStringValue*)rightval->value;
						detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
						detemp->value = (strcmp((char*)slvalue->value, (char*)srvalue->value) > 0)? 1.0 : 0.0;
						rop->value = detemp;
						return rop;
					}
					cerror(0,"ERROR--Don't mix up types");
				}
            }
        }
    }
	cerror(0,"ERROR--Don't mix up types");
	return NULL;
}

void exec ()
{
    switch(currentstat->kind)
    {
		case S_print:
        {
            CPrintStatement* ptstat = (CPrintStatement*)currentstat->statement;
            CValue* rval =   eval(ptstat->expression,&henv);
            if(rval->kind == NumberValue)
            {
                CNumberValue* num = (CNumberValue*)rval->value;
                printf("\n%f\n", num->value);
                return;
            }
            if(rval->kind == StringValue)
            {
                CStringValue* str = (CStringValue*)rval->value;
                printf("\n%s\n",str->value);
                return;
            }
            cerror(0,"ERROR-in your grammar" );
        }
		case S_assign:
		{
			CAssignStatement* astat = (CAssignStatement *)currentstat->statement;
			CValue* val = (CValue*)malloc(sizeof(CValue));
			if(!val)
			{
				cerror(0, "ERROR--not enough memory");
			}
			val = eval(astat->value, &henv);
			putvar(astat->name, val);
			return;
		}
		case S_goto:
		{
			CGotoStatement* gotostat = (CGotoStatement*)currentstat->statement;
			CNumberValue* jval =   eval(gotostat->label,&henv)->value;
			lookupstat((int)jval->value);
			/* TP Be done switch Program control */
			return;
		}
		case S_ifthen:
		{
			CIfThenStatement* ifstat = (CIfThenStatement*)currentstat->statement;
			CNumberValue* dval = eval(ifstat->condition, &henv)->value;
			double dnum = dval->value;
			if(dnum != 0.0)
			{
				CNumberValue* jumpval = eval(ifstat->lnum, &henv)->value;
				/*TO DO  jump function argumet jumpval*/
			}
			return;
		}
		case S_input:
		{
			printf("\nuser input>>");
			CInputStatement* ip = (CInputStatement*)currentstat->statement;
			char buf[256];
			if (fgets(buf, 256, stdin) == NULL)
			{
				printf("stdinYou have not enter your inputstdin");
				return;
			}
			int slen = strlen(buf);
			int i = 1;
			while(i <= slen)
			{
				if(isdigit(buf[i]))
				{
					i++;
					continue;
				}
				break;
			}
			if(slen == i)
			{
				double lval = atof(buf);
				printf("Input value %f\n", lval);
				CValue* vexp = makenumval(lval)->valueexpression;
				putvar(ip->name, vexp);
				return;
			}
			else
			{
				CValue* sexp = makestrval(buf)->valueexpression;
				putvar(ip->name, sexp);
				return;
			}
		}
    }
}



CStatement* fstatements()
{ 
	printf("parser");
	int done = 0;
	int nump = 0;
	struct CExpression* kname;
	CAssignStatement* astat;
	/*struct CExpression* var;*/
    while(currenttoken)
    {	
		printf("\nstart string %s, type %i\n", currenttoken->text, currenttoken->type);
		
		if (done) 
		{
			break;
		}
		if(currenttoken->type == TT_LINE)
		{
			    nexttk();
				continue;
		}
		if(currenttoken->type != TT_NUMBER)
		{
			printf("Poorly structured syntax, each statement line must start with an integer");
			exit(1);
		}
		nump = atoi(currenttoken->text);
		nexttk();
		printf("\nstart string %s, type %i\n", currenttoken->text, currenttoken->type);		
		switch(currenttoken->type)
		{
			
			case TT_LINE:
				nexttk();
				break;
			case TT_LABEL:
			{
				++statsize;
				if(setinjump)
				{
					cerror(0, "ERROR- setting label inside a For loop not allowed");
				}
				break;
			}
			case READ:
			{
				nexttk();
				struct CExpression* e = makevarexpression(currenttoken->text);
				CAElements* tra;
				int icount;
				if(!arraypos)
				{
					arrhead = (CArrayStatement *)malloc(sizeof(CArrayStatement));
					cerror(arrhead, "ERROR- Not enough memory to create array object"); 
					tra = (CAElements *)malloc(sizeof(CAElements));
					cerror(tra, "ERROR- Not enough memory to create array element"); 
					arraypos = (CArrayStatement *)malloc(sizeof(CArrayStatement));
					cerror(arraypos, "ERROR- Not enough memory to create array element"); 
					
					tra->varname = e;		
					arrhead->elem = tra;
					arraypos->elem = tra;
					arrhead->upcount = 0;
					icount = 1;
				}
				else
				{
					tra = (CAElements *)malloc(sizeof(CAElements));
					cerror(tra, "ERROR- Not enough memory to create array element"); 
					tra->varname = e;
					arraypos->elem->next = tra;
					tra = arraypos->elem->next;
					icount = arrhead->nelements + 1;
					printf("\nDATAzeroposition\n");		
				}
				nexttk();
				while(peek(TT_CONCAT_OPERATOR))
				{
					nexttk();
					e = makevarexpression(currenttoken->text);
					tra->next = (CAElements *)malloc(sizeof(CAElements));
					cerror(tra->next, "ERROR- Not enough memory to create array element"); 
					tra->next->varname = e;
					tra = tra->next;
					printf("\nREADING VARIABLES %s\n" ,  arrhead->elem->varname->variableexpression->name);
					nexttk();
					icount++;
				}
				arrhead->nelements = icount;
				arraypos->elem = tra;
				expected(TT_LINE);
				break;
			}
			case DATA:
			{
				if (!arrhead)
				{
					cerror(0, "ERROR- Array variables not created ");
				}
				nexttk();
				suspend = 1;
				int telems = 0;
				int upcount = 0;
				if(arrhead->nelements - arrhead->upcount == 0)
				{
					while(!peek(TT_LINE))
					{
						nexttk();
					}
					nexttk();
					break;
				}
				struct CExpression* e = expression();
				if(!arrhead->ttar)
				{
					/*arrhead->ttar = (CAElements *)malloc(sizeof(CAElements));*/
					arrhead->ttar  = arrhead->elem;
					telems = arrhead->nelements - 1;
					upcount++;
					printf("\nDATAzero\n");
					if (!arrhead->elem->next)
					{
						printf("\nDATAzeroplue\n");
						exit(1);
					}
				}
				else
				{
					telems = arrhead->nelements - arrhead->upcount - 1;
					upcount = arrhead->upcount + 1;
				}
				arrhead->ttar->valexp = e;
				/*arrhead->ttar = arrhead->ttar->next;*/
				printf("\nDATA\n");
				while(peek(TT_CONCAT_OPERATOR))
				{
					printf("\nDATA2\n");
					if (!telems)
					{
						break;
					}
					telems--;
					upcount++;
					nexttk();
					printf("You (%i)hDATA type%s\n", telems, currenttoken->text);
					arrhead->ttar->next->valexp = (struct CExpression*)malloc(sizeof(struct CExpression));
					printf("\nDATA4\n");
					arrhead->ttar->next->valexp = expression();;
					arrhead->ttar = arrhead->ttar->next;
					printf("You (%i)hDATA type%s\n", telems, currenttoken->text);
					
				}
				printf("\nDATA66\n");
				suspend = 0;
				arrhead->upcount = upcount;
				expected(TT_LINE);
				break;
			}
			case LET:
			{
				nexttk();
				printf("\nassigment\n");
				if(currenttoken->type == TT_WORD)
				{
					nexttk();
					expected(TT_ASSIGN_OPERATOR);
					kname =  makevarexpression(double_step_back->text);
					astat= makeassignstat(kname, expression());
					addstat(astat, S_assign, nump);
					++statsize;
					expected(TT_LINE);
					break;
					
				}
				printf("\nassigment after\n");
				exit(1);

			}
            case DIM:
            {
				nexttk();
				printf("\nDIM\n");
				/*Todo .. rework everything here later */
				if (currenttoken->type == TT_WORD)
				{
					kname =  makevarexpression(currenttoken->text);
					nexttk();
					expected(TT_LEFT_PAREN);
					/*CArrayStatement* arstat = makearraystat(kname, expression(), 1);*/
					/*addstat(arstat, S_array, nump);*/
					++statsize;
					expected(TT_RIGHT_PAREN);
					expected(TT_LINE);
					printf("You have syntax error, DIM is to be followed by a Word type%s\n", currenttoken->text);
					break;
				}
				printf("You have syntax error, DIM is to be followed by a Word type");
				exit(1);
			}
 			case FOR:
			{
				nexttk();
				printf("\nFOR\n");
				struct CExpression* initial = expression();
				expected(TO);
				struct CExpression* end = expression();
				struct CExpression* step = NULL;
				printf("\nFORone\n");
				if (peek(STEP))
				{
					nexttk();
					step = expression();
				}
				expected(TT_LINE);
				CStatement* dfor = fstatements();
				nexttk();
				struct CExpression* index = expression();
				CForStatement* fstat = makeforstat(dfor, initial, end, index, step);
				addstat(fstat, S_for, nump);
				expected(TT_LINE);
				break;
			}
			case PRINT:
			{
				printf("\nPRINT\n");
				nexttk();
				struct CExpression* value = expression();
				CPrintStatement* pst = printstat(value);
				addstat(pst, S_print,nump);
				++statsize;
				expected(TT_LINE);
				printf("You have syntax error, Print is to be followed by a Word type%s\n", currenttoken->text);
				break;
			}
			case INPUT:
			{
				nexttk();
				printf("\nINPUT\n");
				kname =  makevarexpression(consume(TT_WORD)->text);
				CInputStatement* in = inputstat(kname);
				addstat(in, S_input, nump);
				++statsize;
				expected(TT_LINE);
				printf("You have syntax error, Print is to be followed by a Word type%s\n", currenttoken->text);
				break;
			}
			case GOTO:
			{
				nexttk();
				struct CExpression* gotoexpr =  expression();
				CGotoStatement* gstat = makegotostat(gotoexpr);
				addstat(gstat, S_goto, nump);
				++statsize;
				printf("GOTO\n");
				expected(TT_LINE);
				break;
			}
			case IF:
			{
				nexttk();
				struct CExpression* cond = expression();
				consume(THEN);
				printf("Inside IF");
				struct CExpression* vl = expression();
				CIfThenStatement* ifst = makeifthenstat(cond, vl);
				addstat(ifst, S_ifthen, nump);
				++statsize;
				expected(TT_LINE);
				break;
			}
			case END:
				nexttk();
				break;
			default:
				if(peek(NEXT))
				{
					chrule = 1;
					return hstat;
				} 
				if (!(currenttoken))
				{
				done = 1;
				break;
				}
				else
				{
					cerror(0, "ERROR--unknown token");
				}
		}
		printf("\nNEXTTK\n")	
    }
    return hstat;
}
	
	
CStatement* parser()
{
	return fstatements();
	
}
	

Clist* makeToken(int tsize, char* text, int type)
{
    Clist* temp = (Clist* )malloc(sizeof(Clist));
    char* stkn;
    int itest;
    cerror(temp, "ERROR: not enough memory available");
    temp->next = 0;
    stkn =  (char*)malloc(sizeof(char)*(tsize + 1));
    cerror(stkn, "ERROR: not enough memory available");
    strncpy(stkn, text, tsize);
    stkn[tsize] = '\0';
    temp->text = stkn;
    if(TT_WORD == type)
    {
        insertlabel(stkn);
    }
    itest = getpriWord(temp->text);
    temp->type =  itest > 49 ? itest : type ;
    printf("string  %s: type %d\n", stkn, temp->type);
    return temp;
}

int tkfirst = 1;

void addToList(Clist* token)
{
    if(tkfirst)
    {
        head = token;
        currenttoken = token;
        tail = token;
        tkfirst = 0;
    }
    else
    {
        tail->next = token;
        tail = token;
    }
}

void  tokenize(char* source)
{
    char c;
    int state = TS_DEFAULT;
    int tkntype;
    int charcount = 0;
    char* strptr = 0;
    char ttop[2];
    char charTokens[] = "\n=+-*/%<>(),";
    while((c = *source++) != 0)
    {
        switch(state)
        {
			case TS_DEFAULT:

                if((strptr = strchr(charTokens, c)) != NULL)
                {
                    char cc = *source;
                    if ((cc == c) && (c == '=')){
						tkntype = getoptype("==");
						addToList(makeToken(2, "==",tkntype ));
						source++;
						
					}
                    else if((c == '>') && (cc == '='))
                    {    
 						tkntype = getoptype(">=");
						addToList(makeToken(2, ">=",tkntype )); 
						source++;
					}
					else if((c == '<') && (cc == '='))
					{
 						tkntype = getoptype("<=");
						addToList(makeToken(2, "<=",tkntype ));
						source++;
					} else
					{	
						if (c == '\n')
						{
							printf("\nNew Line\n");
						}
						ttop[0] = c;
						ttop[1] = '\0';			
						tkntype = getoptype(ttop);					
						addToList(makeToken(1, ttop,tkntype ));
						
					}
                    state = TS_DEFAULT;
                }
                else if (isalpha(c))
                {
                    strptr = source - 1;
                    ++charcount;
                    state = TS_WORD;
                }
                else if (isdigit(c))
                {
                    strptr = source - 1;
                    ++charcount;
                    state = TS_NUMBER;
                }
                else if (c == '\"')
                {
                   strptr = source;
                   state = TS_STRING;
                }
                else if (c == '\'')
                {
                    state = TS_COMMENT;
                }
                else
                {
                   if(isspace(c))
                    {

						while(isspace(c))
                    {
						
                        c = *source++;
                        if (c == '\n')
                        {
							break;
							
						}
                    }
                    
						source--;
                    }
                    
                }

                break;

			case TS_WORD:

                if (c == ':')
                {
                    addToList(makeToken(charcount, strptr, TT_LABEL));
                    charcount = 0;
                    state = TS_DEFAULT;
                    /*source--;*/
                }
                else if ((*source == '\0')&& isalnum(c))
                {
                    ++charcount;
                    
                    addToList(makeToken(charcount,strptr,TT_WORD));
                    charcount = 0;
                    state = TS_DEFAULT;
                    /*source--; this should be removed*/
                }
                else if(isalnum(c))
                {
                    ++charcount;
                }
                else
                {
                    addToList(makeToken(charcount,strptr,TT_WORD));
                    charcount = 0;
                    state = TS_DEFAULT;
                    strptr = 0;
                    source--;
                }
                break;

			case TS_NUMBER:
			
                if(isdigit(c))
                {
                    ++charcount;
                }
                if ((*source == '\0') || (!isdigit(c)))
                {
                    addToList(makeToken(charcount,strptr,TT_NUMBER));
                    charcount = 0;
                    state = TS_DEFAULT;
                    
                    if(*source != '\0')
                    {
						source--;
					}
                }
                break;

			case TS_STRING:

                if(c == '\"')
                {
                    addToList(makeToken(charcount , strptr,TT_STRING));
                    charcount = 0;
                    state = TS_DEFAULT;
                }
                else
                    ++charcount;

               break;

			case TS_COMMENT:

                if(c == '\n')
                   state = TS_DEFAULT;
                //source--;
                break;
        }

    }
}

struct CExpression* catomic ()
{
    //printf()
    if(!currenttoken) return 0;


    if(match(TT_WORD))
    {
        return makevarexpression(stepback->text);
    }
    else if (match(TT_NUMBER))
    {

        return makenumval(strtod(stepback->text, 0));
    }
    else if (match(TT_STRING))
    {
        return makestrval(stepback->text);
    }
    else if (match(TT_LEFT_PAREN))
    {
        struct CExpression* express = expression();
        consume(TT_RIGHT_PAREN);
        return express;
    }

    printf("ERROR--could not parse :( %s", currenttoken->text);
    exit(1);
}

struct CExpression* term()
{
	int cop;
	struct CExpression*  lfactor = catomic();
	while(match(TT_MULT_OPERATOR)||match(TT_DIV_OPERATOR)||match(TT_MOD_OPERATOR))
	{
		cop = stepback->type;
		struct CExpression* rfactor = catomic();
		lfactor = makeoperatorexpress(lfactor , cop, rfactor);
		
	}
	return lfactor;
}
	
		
struct CExpression* subexpress()
{
	int cop;
	struct CExpression*  lterm = term();
	while(match(TT_MINUS_OPERATOR)|| match(TT_PLUS_OPERATOR))
	{
		cop = stepback->type;
		struct CExpression* rterm = term();
		lterm = makeoperatorexpress(lterm , cop, rterm);
	}
	return lterm;
}

struct CExpression* coperator()
{
	int cop;
    struct CExpression* cterm = subexpress();
    if (suspend)
    {
		return cterm;
	}
    while(match(TT_LEQ_OPERATOR)|| match(TT_EQ_OPERATOR)||match(TT_GEQ_OPERATOR)|| match(TT_GE_OPERATOR)||match(TT_LE_OPERATOR)||match(TT_CONCAT_OPERATOR))
    {
        cop = stepback->type;
        if(stepback->type == TT_CONCAT_OPERATOR)
        {
			printf("CONCAT");
		}
        struct CExpression* rterm = subexpress();
        cterm = makeoperatorexpress(cterm, cop, rterm);
    }
    return cterm;
}

struct CExpression* expression()
{
    return coperator();
}

int filesize(char* fname){
	int fsize;
	FILE* c = fopen(fname, "r");
	if (fseek(c, 0, SEEK_END)){
		return -1;
	}
	fsize = ftell(c);
	fclose(c);
	return fsize;
}

void testtokenize(){
	char testlist[][9] = {"test1.b", "test2.b"};
	FILE* f;
	int flen = filesize(testlist[0]);
	char* fsource = (char*)malloc(sizeof(char)*(flen + 1));
	f = fopen(testlist[0], "r");
	fread(fsource, 1, flen, f);
	fclose(f);
	fsource[flen] = '\0';
	tokenize(fsource);
	if(!currenttoken)
	{
		printf("Failed becuase current tokem is empty");
		exit(1);
	}
	printf("%i  %s", currenttoken->type, currenttoken->text);
	parser();
	
}
	
int main()
{
	testtokenize();
	
	Clist* ctoken  = head;
	Clist* temptoken;
	printf("\npinally\n");
	while(ctoken){
		temptoken = ctoken;
		ctoken = ctoken->next;
		free(temptoken->text);
		free(temptoken);
	}
    printf("\nFinally\n");
    return 0;
}

