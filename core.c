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


int statsize = 1;
int trastatpos = 0;
int chpos = 0;
char tknlb [30][20];
int  dim = 30;
int  currentpos = 0;
int sizeelem = 0;
int savesize[5];
int setinjump = 0;


struct optoken
{
    char cop;
    int nop;
};


struct CExpression* expression();
struct CExpression* catomic ();
struct CExpression* coperator();
//Various states token might be in.
enum TokenizeState {
      TS_DEFAULT , TS_WORD, TS_NUMBER, TS_STRING, TS_COMMENT};

// Token types
enum TokenType {
      TT_WORD , TT_NUMBER, TT_STRING, TT_LABEL, TT_LINE,
      TT_EQUAL, TT_OPERATOR, TT_LEFT_PAREN, TT_RIGHT_PAREN, TT_EOF};

struct optoken aroptoken[10] = {{'\n', TT_LINE}, {'=', TT_EQUAL},{'+', TT_OPERATOR},{'-', TT_OPERATOR},{'*',TT_OPERATOR},{'/', TT_OPERATOR},
                            {'<', TT_OPERATOR},{'>', TT_OPERATOR}, {'(', TT_LEFT_PAREN}, {')', TT_RIGHT_PAREN}};
const int aroptokenlen = 10;



char getoptype (char cop)
{
    int i;
    for(i = 0; i < aroptokenlen; i++)
    {
            if(cop == aroptoken[i].cop)
                return aroptoken[i].nop;
    }
    return '\0';

}
void cerror (void* obj, char* message)
{

        if(!obj)
    {
        printf(message);
        exit(0);
    }
}


void insertlabel(char* label)
{
    if(currentpos < dim)
    {
        strcpy(tknlb[currentpos++], (const char*)label);
    }

    return;
}

int contains (char* label)
{
    int i = 0;
    while(i < currentpos)
    {
        if(strcmp((const char*)label, (const char*)tknlb[i] ) == 0)
        {
            return 1;
        }
        i++;
    }
    return 0;
}



typedef struct  Clist
{
    char* text;
    int type;
    struct Clist* next;

}Clist;

static Clist* head, *tail,*stepback, *currenttoken, *doubleback;
//head = tail = stepback = currenttoken = 0;
//static Clist* tail = 0;
//static Clist* stepback = 0;
//static Clist* doubleback = 0;
//static Clist* currenttoken = 0;

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



struct CExpression
{
    int extype;
    union
    {
        struct COperatorExpression* operatorexpression;
        struct CVariableExpression* variableexpression;
        CValue* valueexpression;

    };
};

typedef struct Clabels
{
    char* key;
    int ival;
}Clabels;



struct COperatorExpression
{
    struct CExpression* left;
    struct CExpression* right;
    char coperator;

};


struct CVariableExpression
{
    char* name;
};



struct CExpression* makenumval(double val)
{
    CNumberValue* dval;
    CValue* result;
    struct CExpression* rstexpress;

    dval= (CNumberValue *) malloc(sizeof(CNumberValue));
    result = (CValue* ) malloc(sizeof(CValue));
    rstexpress = (struct CExpression* ) malloc(sizeof(struct CExpression*));
    if(!dval || !dval || !result)
    {
        cerror(0,"ERROR---Not enough space" );

    }
    dval->value = val;
    result->vtype = NumberValue;
    result->value = dval;
    rstexpress->extype = V_value;
    rstexpress->valueexpression = result;
    return rstexpress;
}


struct CExpression* makestrval(char* str)
{
    CStringValue* sval;
    CValue* result;
    struct CExpression* rstexpress;
    sval = (CStringValue *)malloc(sizeof(CStringValue));
    result = (CValue*)malloc(sizeof(CValue));
    rstexpress = (struct CExpression*)malloc(sizeof(struct CExpression));
    if(!sval || !sval || !rstexpress)
    {
        cerror(0,"ERROR---Not enough space" );
    }
    sval->value = str;
    result->value = sval;
    result->vtype = StringValue;
    rstexpress->extype = V_value;
    rstexpress->valueexpression = result;
    return rstexpress;

}

int match(int tkntype)
{
    if(!currenttoken)
        return 0;
    if(currenttoken->type != tkntype)
        return 0;

    doubleback = stepback;
    stepback = currenttoken;
    currenttoken = currenttoken->next;
    return 1;
}



int matchtypes(int tpa, int tpb)
{
    Clist* temp,*ctemp;
    if(!currenttoken)
        return 0;
    if(currenttoken->type != tpa)
        return 0;
    ctemp = currenttoken;
    temp = currenttoken->next;
    if(temp->type != tpb)
        return 0;
    doubleback = ctemp;
    stepback = temp;
    currenttoken = temp->next;
    return 1;
}


int matchstring(char* str)
{
    if(!currenttoken)
        return 0;
    if(currenttoken->type != TT_WORD)
        return 0;
    if(strcmp((const char* ) currenttoken->text, (const char*)str) != 0)
        return 0;
    doubleback = stepback;
    stepback = currenttoken;
    currenttoken = currenttoken->next;
    return 1;
}


struct CExpression* makeoperatorexpress(struct CExpression* left, char charop, struct CExpression* right)
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
    cop->coperator = charop;
    result->extype = V_operator;
    result->operatorexpression = cop;
    return result;
}



struct CExpression* makevarexpression(char* text)
{
    struct CVariableExpression* temp;
    struct CExpression* result;
    temp = (struct CVariableExpression* ) malloc(sizeof(struct CVariableExpression));
    result = (struct CExpression* ) malloc(sizeof(struct CExpression));
    if(!temp || !result )
    {
        cerror(0, "ERROR---Not enough space");
    }
    temp->name = text;
    result->extype = V_expression;
    result->variableexpression = temp;
    return result;


}
typedef struct Cvariables
{
    char* key;
    CValue* vval;
    struct Cvariables* next;
}Cvariables;

Cvariables* henv = 0;
Cvariables* tenv = 0;
//This is the base for env
Cvariables** foo = 0;


typedef struct CPrintStatement
{
    struct CExpression* expression;
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

}CGotoStatement;

typedef struct CIfThenStatement
{
    struct CExpression* condition;
    char* label;

}CIfThenStatement;

typedef struct CArrayStatement
{
    struct CExpression* exp;
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
    Cvariables* lenv;
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

CStatement* hstat = 0;
CStatement* tstat = 0;
CStatement* currentstat = 0;
CSave hfor[5] ;
CSFor fsfor[5];
int fc = 0;



void addstat(void* statf, int stype)

{
    CStatement* ttstat = (CStatement *)malloc(sizeof(CStatement));
    cerror(ttstat, "ERROR--not enough memory");
    if (S_for == stype)
    {
        fc = fc - 1;
        sizeelem--;
        hstat = hfor[fc].shead;
        tstat = hfor[fc].stail;

    }

    ttstat->statement = statf;
    ttstat->stype = stype;
    ttstat->next = 0;
    if(!tstat)
          {
             hstat = ttstat;
             tstat = ttstat;

             return;
          }
         else
         {
          tstat->next = ttstat;
          tstat = ttstat;
          return;
        }

}


struct labels
{
    int stsize;
    char* label;
    struct labels* next;
};

struct labels* hlabels = 0;
struct labels* tlabels = 0;

void addlabel(int ssize, char* label)
{
    struct labels* locallab = (struct labels *)malloc(sizeof(struct labels));
    cerror(locallab, "ERROR--not enough memory");
    if(!hlabels)
    {
        locallab->stsize = ssize;
        locallab->next = 0;
        locallab->label = label;
        tlabels = locallab;
        hlabels = locallab;
        return;
    }
    else
    {
        locallab->stsize = ssize;
        locallab->label = label;
        locallab->next = 0;
        tlabels->next = locallab;
        tlabels = locallab;
        return;
    }
}

int lookuplabel(char* strlab,int tcheck)
{
    struct labels** slab =  &hlabels;
    CStatement** sstat = &hstat;
    struct labels* lab = *slab;
    CStatement* stats = *sstat;
    while(lab)
    {
         if(strcmp((const char*)lab->label, (const char*)strlab)  == 0)
        {
            if(!tcheck)
            {
                return 1;
            }
            int k = 1;
            while( k < lab->stsize - 2  )
            {
                stats = stats->next;
                k++;
            }
            currentstat = stats;
            chpos = 1;
            return 1;

        }
         lab = lab->next;
    }

    if(!tcheck)
    {
        return 0;
    }
    cerror(0,"ERROR-- goto label not seen before" );
}


CIfThenStatement* makeifthenstat(struct CExpression* exp, char* label)
{
    CIfThenStatement* ifstat = (CIfThenStatement*)malloc(sizeof(CIfThenStatement));
    cerror(ifstat,"ERROR-not enough memory" );
    ifstat->condition = exp;
    ifstat->label = label;
    return ifstat;


}
CGotoStatement* makegotostat(char* label)
{
    CGotoStatement* gotostat = (CGotoStatement*)malloc(sizeof(CGotoStatement));
    cerror(gotostat, "ERROR-not enough memory");
    gotostat->label = label;
    return gotostat;

}

CAssignStatement* makeassignstat(char* name, struct CExpression* value)
{
    CAssignStatement* assignstat = (CAssignStatement*)malloc(sizeof(CAssignStatement));
    cerror(assignstat, "ERROR-not enough memory assignment");
    assignstat->name = name;
    assignstat->value = value;
    return assignstat;
}

CArrayStatement* makearraystat(char* name, struct CExpression* value)
{
    CArrayStatement* arraystat = (CArrayStatement*)malloc(sizeof(CArrayStatement));
    cerror(arraystat, "ERROR-not enough memory assignment");
    arraystat->varname = name;
    arraystat->exp = value;
    return arraystat;
}

void savecurrentfor()
{
    printf("FOR STOP2\n");
    if(fc > 4)
    {
        cerror(0, "ERROR--passed accepted number of For in a loop");
    }
    printf("FOR STOP2\n");
    hfor[fc].shead = hstat;
    hfor[fc].stail = tstat;
    printf("FOR STOP2\n");
    fc++;
    hstat = 0;
    tstat = 0;
    return;
}

CForStatement* makeforstat(char* name, struct CExpression* inex, struct CExpression* endex)
{

    if(!forstat)
    {
        forstat = (CForStatement*)malloc(sizeof(CForStatement));
        cerror(forstat, "ERROR-not enough memory assignment");
    }

    if(setinjump)
    {
       if ((sizeelem  > 1) && (sizeelem - 1 >= 4))
       {
            fsfor[sizeelem].lcfor = forstat;
            forstat = (CForStatement*)malloc(sizeof(CForStatement));
            cerror(forstat, "ERROR-not enough memory assignment");
       }

    }
    else
      setinjump = 1;

    forstat->index = name;
    forstat->indexpress = endex;

    return forstat;
}


CInputStatement* inputstat(char* name)
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


CValue* lookup(char* key, Cvariables** varenv)
{
    Cvariables* varhead =  *varenv;
    while(varhead)
    {
        if(strcmp((const char*)varhead->key,(const char*)key) == 0)
           {
               return varhead->vval;
           }
        varhead = varhead->next;
    }
    return 0;

}

void putvar(char* key , CValue* val)
{
        Cvariables* lenv = (Cvariables *)malloc(sizeof(Cvariables));
        cerror(lenv, "ERROR-not enough memory assignment");
        lenv->vval = val;
        lenv->next = 0;
        lenv->key = key;

        if(!tenv)
        {

           henv = lenv;
           tenv = lenv;

        return;
        }
        else
        {
            tenv->next = lenv;
            tenv = lenv;
            return;
        }

}

Clist* consume (int typ)
{
    Clist* temp;
    if(currenttoken->type != typ)
    {
        printf("ERROR---Expected type %i", typ);
        exit(0);
    }
    temp = currenttoken;
    doubleback = stepback;
    stepback = temp;
    currenttoken = currenttoken->next;
    return temp;
}

Clist* consumestr (char* str)
{
    if(!matchstring(str))
    {
        printf("ERROR--expected %s", str);
        exit(0);
    }
    return stepback;
}


CValue* eval(struct CExpression* express, struct Cvariables** varenv)
{
    switch(express->extype)
    {

    case V_expression:
        {
            struct CVariableExpression* varexpress = express->variableexpression;
            CValue* cval = lookup(varexpress->name, varenv);
            if(cval)
            {
                switch(cval->vtype)
                {
                case NumberValue:
                    {
                        return cval;
                    }

                case StringValue:
                    {
                        return cval;
                    }
                default:
                    {
                        cerror(0, "ERROR--unexpected value type");

                    }
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
                switch(vval->vtype)
                {
                case NumberValue:
                    {
                        return vval;
                    }

                case StringValue:
                    {
                        return vval;
                    }
                default:
                    {
                        cerror(0, "ERROR--unexpected value type");
                     }
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
                case '=':
                    {
                        if(leftval->vtype == NumberValue)
                        {
                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            if(dlvalue == drvalue)
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
                            rop->vtype = StringValue;
                            slvalue = (CStringValue*)leftval->value;
                            srvalue = (CStringValue*)rightval->value;
                            detemp =  (CNumberValue* )malloc(sizeof(CNumberValue));
                            if(strcmp((const char*)slvalue->value, (const char*)srvalue->value) == 0)
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

                    }

                case '+':
                    {
                        if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp =  (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = dlvalue->value + drvalue->value;
                            rop->value = detemp;
                            return rop;
                        }
                        if((leftval->vtype == StringValue)&&(rightval->vtype == StringValue))
                        {
                            rop->vtype = StringValue;
                            slvalue = (CStringValue*)leftval->value;
                            srvalue = (CStringValue*)rightval->value;
                            strcat(slvalue->value, (const char*)srvalue->value);
                            rop->value = slvalue;
                            return rop;
                        }

                        cerror(0,"ERROR--Don't mix up types" );

                    }
                case '-':
                    {
                         if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = dlvalue->value - drvalue->value;
                            rop->value = detemp;
                            return rop;
                        }
                         cerror(0,"ERROR--Don't mix up types" );
                    }

                case '*':
                    {
                        if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = dlvalue->value * drvalue->value;
                            rop->value = detemp;
                            return rop;
                        }
                         cerror(0, "ERROR--Don't mix up types");
                    }
                case '/':
                {


                      if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
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
                case '%':
                    {


                      if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
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
                case '<':
                    {
                         if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = (dlvalue->value < drvalue->value) ? 1.0 : 0.0;
                            rop->value = detemp;
                            return rop;
                        }
                          if((leftval->vtype == StringValue)&& (rightval->vtype == StringValue))
                        {

                            rop->vtype = NumberValue;
                            slvalue = (CStringValue*)leftval->value;
                            srvalue = (CStringValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = (strcmp((const char*)slvalue->value, (const char*)srvalue->value) < 0)? 1.0 : 0.0;
                            rop->value = detemp;
                            return rop;
                        }

                         cerror(0,"ERROR--Don't mix up types");

                    }

                    case '>':
                    {


                         if((leftval->vtype == NumberValue)&& (rightval->vtype == NumberValue))
                        {

                            rop->vtype = NumberValue;
                            dlvalue = (CNumberValue*)leftval->value;
                            drvalue = (CNumberValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = (dlvalue->value > drvalue->value) ? 1.0 : 0.0;
                            rop->value = detemp;
                            return rop;
                        }
                          if((leftval->vtype == StringValue)&& (rightval->vtype == StringValue))
                        {

                            rop->vtype = NumberValue;
                            slvalue = (CStringValue*)leftval->value;
                            srvalue = (CStringValue*)rightval->value;
                            detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
                            detemp->value = (strcmp((const char*)slvalue->value, (const char*)srvalue->value) > 0)? 1.0 : 0.0;
                            rop->value = detemp;
                            return rop;
                        }

                         cerror(0,"ERROR--Don't mix up types");
                    }

                    }

            }

    }

            cerror(0,"ERROR--Don't mix up types");

}

void exec ()
{
    switch(currentstat->stype)
    {
    case S_print:
        {
            CPrintStatement* ptstat = (CPrintStatement*)currentstat->statement;
            CValue* rval =   eval(ptstat->expression,&henv);
            if(rval->vtype == NumberValue)
            {
                CNumberValue* num = rval->value;
                printf("\n%f\n", num->value);
                return;
            }
            if(rval->vtype == StringValue)
            {
                CStringValue* str = rval->value;
                printf("\n%s\n",str->value);

                return;
            }
            cerror(0,"ERROR-in your grammar" );

        }
    case S_assign:
        {
            CAssignStatement* astat = (CAssignStatement *)currentstat->statement;
            CValue* val = (CValue*)malloc(sizeof(CValue));
            if(!astat || !val)
            {
                cerror(0, "ERROR--not enough memory");
            }
            CValue* lval = eval(astat->value, &henv);
            *val = *lval;
            char* name = astat->name;
            putvar(name, val);
            return;
        }
    case S_goto:
        {
            CGotoStatement* gotostat = (CGotoStatement*)currentstat->statement;
            lookuplabel(gotostat->label, 1);
            return;
        }
    case S_ifthen:
        {
            CIfThenStatement* ifstat = (CIfThenStatement*)currentstat->statement;
            CNumberValue* dval = eval(ifstat->condition, &henv)->value;

            double dnum = dval->value;
            if(dnum != 0.0)
            {
                lookuplabel(ifstat->label, 1);

            }
        }   return;
    case S_input:
        {
            printf("\nuser input>>");
            CInputStatement* ip = (CInputStatement*)currentstat->statement;
            char buf[256];
            gets(buf);
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


CStatement* parser ()
{
    //CStatement* restat = 0;

    while(1)
    {
        while(match(TT_LINE)){ printf("break out new line");}

        if(match(TT_LABEL))
        {
            ++statsize;
            if (setinjump)
            {
                cerror(0, "ERROR- setting label inside a For loop not allowed");
            }
            addlabel(statsize, stepback->text);

        }
        else if(matchtypes(TT_WORD, TT_EQUAL))
        {

            printf("\nassign statement\n");

            char* name = doubleback->text;
            CAssignStatement* astat = makeassignstat(name, expression());
             addstat(astat, S_assign);
            ++statsize;
        }
        else if(matchstring("DIM") && match(TT_WORD))
        {

                     char* name = stepback->text;
                     CArrayStatement* arstat = makearraystat(name, expression());
                     addstat(arstat, S_array);
                     ++statsize;
                     printf(name);
                     printf("\narray statement\n");
        }
        else if(matchstring("FOR")&& match(TT_WORD)&& match(TT_EQUAL))
        {
            char* name = doubleback->text;
            struct CExpression* inex = expression();
            struct CExpression* texp;
            consumestr("TO");
            printf("\nFOR STOP\n");
            //exit(0);
            texp = expression();
            savecurrentfor();
            printf("FOR STOP222\n");
            savesize[sizeelem++] = statsize;
            printf("sizeelem  %d", sizeelem);
            statsize = 0;
            printf("FOR STOP552\n");
            CForStatement* dfor = makeforstat(name, inex, texp);
            printf("FOR STOP3\n");
            //exit(0);
            parser();
            printf("FOR STOP4\n");
            dfor->lforstat =  hstat;
            addstat(dfor, S_for);
            printf("sizeelem1  %d", sizeelem);
            statsize = savesize[sizeelem] + 1;
            printf("FOR STOP5\n");
            if(sizeelem)
            {
                forstat = fsfor[sizeelem].lcfor;
                printf("\nsomething wrong  %d\n", sizeelem);
            }
            else
            {
                setinjump = 0;
                //sizeelem = 0;
            }


        }
        else if(matchstring("print"))
        {
            printf("\nprint statement77\n");
            struct CExpression* value = expression();
            CPrintStatement* pst = printstat(value);
            addstat(pst, S_print);
            ++statsize;
        }
        else if(matchstring("input"))
        {
          printf("\ninput statement\n");
          Clist* tk = consume(TT_WORD);
          CInputStatement* in = inputstat(tk->text);
          addstat(in, S_input);
          ++statsize;

        }
        else if(matchstring("goto"))
        {
            printf("\ngoto statement\n");
            if(contains(currenttoken->text))
            {
                CGotoStatement* gstat = makegotostat(consume(TT_WORD)->text);
                addstat(gstat, S_goto);
                ++statsize;
            }
            else
            {
                cerror(0, "ERROR--this label was not declared ");
            }
        }
        else if(matchstring("if"))
        {
            printf("\nIf statement\n");
            struct CExpression* cond = expression();
            consumestr("then");
            if(contains(currenttoken->text))
               {
                Clist* label = consume(TT_WORD);
                char* wlab = label->text;
                CIfThenStatement* ifst = makeifthenstat(cond, wlab);
                addstat(ifst, S_ifthen);
                ++statsize;
                }
             else
                {
                   cerror(0, "ERROR--this label was not declared ");
                }


        }
        else if(matchstring("NEXT"))
        {
            printf("\nNEXT statement\n");
            struct CExpression* var = expression();
            if(strcmp((const char*)forstat->index, (const char*)stepback->text ) == 0)
           {
                consumestr("END");
                printf(currenttoken->text);
                return 0;

           }
           else
           {
               cerror(0, "ERROR--missing the index used at the For");

           }
            //restore();

        }
        if (!(currenttoken))
        {
            break;
        }
    }

    return hstat;
}
Clist* makeToken(int tsize, char* text, int type)
{
    Clist* temp = (Clist* )malloc(sizeof(Clist));
    char* stkn;
    cerror(temp, "ERROR: not enough memory available");
    temp->next = 0;
    stkn =  (char*)malloc(sizeof(char)*(tsize + 1));
    cerror(stkn, "ERROR: not enough memory available");
    strncpy(stkn, (const char*)text, (size_t)tsize);
    stkn[tsize] = '\0';
    temp->text = stkn;
    if(TT_WORD == type)
    {
        insertlabel(stkn);
    }
    if(text[0] == '*')
    {
        printf("%c\n", text[0]);
    }
    temp->type = type;
    return temp;

}

void addToList(Clist* token)

{
    Clist* temp;
    if(!tail)
    {
        head = token;
        tail = token;
           }
    else
    {
        temp = token;
        tail->next = token;
        tail = temp;

    }
}

void  tokenize(char* source)
{
    //char* token;
    char c;
    int state = TS_DEFAULT;
    int tkntype;
    int charcount = 0;
    char* strptr = 0;
    char mcop;
    char charTokens[] = "\n=+-*/%<>()";

    while((c = *source++) != 0)
    {
        switch(state)
        {
        case TS_DEFAULT:



                if((strptr = strchr(charTokens, c)))
                {
                    mcop = *strptr;
                    tkntype = getoptype(mcop);
                    addToList(makeToken(1, strptr,tkntype ));
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

                else if (c == '"')
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
                    source--;
                }
                else if ((*source == '\0')&& isalnum(c))
                {
                    ++charcount;
                    addToList(makeToken(charcount,strptr,TT_WORD));
                    charcount = 0;
                    state = TS_DEFAULT;
                    source--;
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

                    //printf()
                    source--;
                }
                break;

        case TS_NUMBER:
                if(isdigit(c))
                {

                    ++charcount;

                }
                if ((*source == 0) || (!isdigit(c)))
                {

                    addToList(makeToken(charcount,strptr,TT_NUMBER));
                    charcount = 0;
                    state = TS_DEFAULT;
                    source--;
                }
                break;


        case TS_STRING:

                if(c == '"')
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




return  ;
}



struct CExpression* expression()
{
    return coperator();
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

    printf("ERROR--could not parse :(");
    exit(0);
}


struct CExpression* coperator()
{
    struct CExpression* express = catomic();

    while(match(TT_OPERATOR)|| match(TT_EQUAL))
    {
        char cop = stepback->text[0];
        struct CExpression* right = catomic();
        if(!right)
        {
            break;
        }
        express = makeoperatorexpress(express, cop, right);
    }

    return express;
}

int main()
{
    char* ytest  = "foo = (314159 * 10) - 3000000 \n DIM A(17) \n print foo \n  \n tin = 3 \n if tin > tin then fin    \n  print foo  \n input guess \n print foo / guess  \n fin: \n  print \"Well Done\" \n print tin ";
    char* xtest = "  \"Hello\" + \" Emeka\" + \" Nwankwo\" Hold FOR I = 1 TO 10 \n print 44 \n NEXT I \n END";
    char* loo = " ";
    //printf(ytest);
    tokenize(ytest);

    Clist** hold = &head;
    Clist* lhead = *hold;

    while(lhead )
    {   printf("\n");
        printf(lhead->text);
        printf("tap\n");
        printf("%i", lhead->type);
        lhead = lhead->next;
        if(lhead) printf("empty head");
    }

        currenttoken = head;
        currentstat = parser();
        while(currentstat)
        {

        exec(foo);
        currentstat = currentstat->next;
        }


     Clist* temp;

    while(head)
    {
        free(head->text);
        temp = head;
        head = head->next;
        free(temp);
    }

    return 0;
}

