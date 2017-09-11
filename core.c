/**
 * Copy Right:: Freely we received , freely do we give. By Emeka 
 * Print Statement, can take multiple data
 * 5 PRINT "Enter your test number"
 * Read data from the stdin, takes only one argument
 * 10  INPUT L 
 * Assignment statement
 * 15 LET D = 100 
 * Unconditional jump
 * 17 GOTO 45
 * 20 LET N = "Coder, you are welcome! " 
 * 30 LET M = "Hello Mr " 
 * 35 PRINT  M, N
 * Print statement, however when without argument prints new line
 * 38 PRINT
 * 40 PRINT L 
 * 43 PRINT
 * Conditional jump
 * 45 IF L > 30 THEN 130
 * Subroutine jumps to the label, however when it encounters return statement
 * It would return to the next statement after GoSub
 * 50 GOSUB 90
 * 60 PRINT "Good Bye"
 * 65 PRINT
 * 70 LET D = 100 
 * 80 PRINT "MIRACLE SEEDS"
 * 83 PRINT
 * 85 GOTO 140
 * 90  LET N = "Satapotus"
 * 100 LET M = " Ugunda "
 * 110 PRINT  N, M
 * 115 PRINT
 * 120 PRINT L
 * 125 RETURN
 * 130 PRINT "This is your day!"
 * 135 PRINT
 * 140 PRINT "GAME OVER!"
 * 150 PRINT
 * For Loop is just nice-to-have because goto and if statement could easily 
 * simulate it. However if STEP is missing then it would increment by 1
 * When it encounters Next statement , it would increment the  loop number and jump back to
 * the beginning of the FOR statement. There is a bug which means that FOR statement should 
 * not be the last statement. 
 * 160 FOR I = 1 TO 7  STEP 3
 * 170 PRINT I
 * 180 PRINT
 * 190 NEXT I
 * 195 PRINT
 * 200 PRINT "DONE"
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include "decl.h"


static Clist* head, *tail,*stepback, *currenttoken, *doubleback;
env* henv = NULL;
env* tenv = NULL;
//This is the base for env
env** foo = NULL;
ReturnStack* rstack = NULL;
struct labelList* llst;
struct labels* hlabels = 0;
struct labels* tlabels = 0;
FuncAbstraction* funcTable[FN_SIZE];
CStatement* hstat = NULL;
CStatement* tstat = NULL;
CStatement* currentstat = NULL;
CStatement* pc = NULL;
CSave hfor[5] ;
CSFor fsfor[5];
int fc = 0;

char* reservedword[] = { "DIM", "FOR", "PRINT", "DEF", "IF","READ","INPUT","NEXT", "RETURN","STOP", "END", "GOTO", "GOSUB", "THEN", "LET", "DATA", "TO", "STEP", "REM",NULL};

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

void cerror (void* obj, char* message, const char* name,  int line)
{
    if(!obj)
    {
        fprintf(stderr, "%s\ninside function %s \nand line %d\n", message, name, line);
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

int contains(char* label)
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


int findTableIndex(char* name){
	int idx = fntabidx;
	for(int i = 0; i < idx; i++){
		if(strcmp(name, funcTable[i]->name) == 0){
			return i;
		}
	}
	return -1;
}

void  addFunc(FuncAbstraction* fun){
	if(fntabidx == FN_SIZE){
		cerror(0,"ERROR---You have reached the maximum function size 100", __func__, __LINE__  );
	}
	fntabidx++;
	funcTable[fntabidx] = fun;	
	return;
}

		
struct CExpression* makefunCall(char* name, LExpression* Lexp, int nsize)
{
	FuncCall* fncall;
	struct CExpression* expcall;
	fncall = malloc(sizeof(FuncCall));
	int idx;
	if(!fncall){
		
		cerror(0,"ERROR---Not enough space to create FuncCall Object", __func__, __LINE__ );
		
	}
	fncall->LexpList = Lexp;
	fncall->name = name;
	fncall->argSize = nsize;
	idx = findTableIndex(name);
	fncall->tableIndex = idx;
	expcall = malloc(sizeof(struct CExpression*));
	if(!expcall){
		cerror(0,"ERROR---Not enough space to create Expression FuncCall Object" , __func__, __LINE__ );
		
	}
	expcall->extype = V_call;
	expcall->funCall = fncall;
	return expcall;
	
}
	
struct CExpression* makenumval(double val)
{
    CNumberValue* dval;
    CValue* result;
    struct CExpression* rstexpress;
    dval=  malloc(sizeof(CNumberValue));
    result = malloc(sizeof(CValue));
    rstexpress = (struct CExpression* ) malloc(sizeof(struct CExpression*));
    if(!dval || !dval || !result)
    {
        cerror(0,"ERROR---Not enough space", __func__, __LINE__ );

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
    sval = malloc(sizeof(CStringValue));
    result = malloc(sizeof(CValue));
    rstexpress = (struct CExpression*)malloc(sizeof(struct CExpression));
    if(!sval || !sval || !rstexpress)
    {
        cerror(0,"ERROR---Not enough space", __func__, __LINE__ );
    }
    sval->value = str;
    result->value = sval;
    result->vtype = StringValue;
    rstexpress->extype = V_value;
    rstexpress->valueexpression = result;
    return rstexpress;
}


struct CExpression* makeoperatorexpress(struct CExpression* left, int op, struct CExpression* right)
{
    struct COperatorExpression* cop;
    struct CExpression* result;
    cop = malloc(sizeof(struct COperatorExpression));
    result = malloc(sizeof(struct CExpression));
    if(!cop || !result)
    {
        cerror(0,"ERROR---Not enough space", __func__, __LINE__ );

    }
    cop->left = left;
    cop->right = right;
    cop->coperator = op;
    result->extype = V_operator;
    result->operatorexpression = cop;
    return result;
}

struct CExpression* makevarexpression(char* text)
{
    struct CVariableExpression* temp;
    struct CExpression* result;
    temp = malloc(sizeof(struct CVariableExpression));
    result = malloc(sizeof(struct CExpression));
    if(!temp || !result )
    {
        cerror(0, "ERROR---Not enough space" , __func__, __LINE__);
    }
    temp->name = text;
    result->extype = V_expression;
    result->variableexpression = temp;
    return result;
}


void addstat(void* statf, int stype,int ssize,char* labelnumber,int sty)
{
	int lnum = (int)strtol(labelnumber,(char**) NULL, 10);;
	Clabels* clb  = malloc(sizeof(Clabels));
	cerror(clb, "ERROR--not enough memory to create Clabels" , __func__, __LINE__);
	if(sty){
		clb->key = lnum;
		clb->cindex = ssize;
		clb->st = NULL;
		if(llst->top - 1 < STAT_MAX){
			llst->clabs[llst->top++] = clb;
		}
		return;
	}
    CStatement* ttstat = malloc(sizeof(CStatement));
    cerror(ttstat, "ERROR--not enough memory" , __func__, __LINE__);
    ttstat->statement = statf;
    ttstat->stype = stype;
    ttstat->next = NULL;
    clb->key = lnum;
    clb->cindex = ssize;
    clb->st = ttstat; 
    if(llst->top - 1 < STAT_MAX){
		llst->clabs[llst->top++] = clb;
	}
    if(!tstat)
    {
        hstat = ttstat;
        tstat = ttstat;
    }
     else
    {
        tstat->next = ttstat;
        tstat = ttstat; 
    }
    return;

}

void  jumpto(int jmp , int test)
{
	if (jmp < 0){
		for(int i = 0; i < llst->top; i++){
			if(llst->clabs[i]->cindex == test){
				
				pc = llst->clabs[i]->st;
				return;
			}
		}
		
	}
	else {
		for(int i = 0; i < llst->top; i++){
			if(llst->clabs[i]->key == jmp){
				pc = test? llst->clabs[i + 1]->st :llst->clabs[i]->st ;
				return;
			}
		}
	}
	cerror(0, "The label to jump to is not in your code" , __func__, __LINE__);

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
    if(strcmp(currenttoken->text, str) != 0)
        return 0;
    doubleback = stepback;
    stepback = currenttoken;
    currenttoken = currenttoken->next;
    return 1;
}

CIfThenStatement* makeifthenstat(struct CExpression* exp, char* label)
{
    CIfThenStatement* ifstat = malloc(sizeof(CIfThenStatement));
    cerror(ifstat,"ERROR-not enough memory to create if statement object", __func__, __LINE__ );
    ifstat->condition = exp;
    ifstat->label = label;
    return ifstat;
}

CGotoStatement* makegotostat(char* label)
{
    CGotoStatement* gotostat = malloc(sizeof(CGotoStatement));
    cerror(gotostat, "ERROR-not enough memory to create goto object" , __func__, __LINE__);
    gotostat->label = label;
    gotostat->nlab = (int)strtol(label, (char**)NULL, 10);
    return gotostat;
}

CGosubStatement* makegosubstat(int jmp, int lbnum)
{
    CGosubStatement* gosubstat = malloc(sizeof(CGosubStatement));
    cerror(gosubstat, "ERROR-not enough memory to create gosub object" , __func__, __LINE__);
    gosubstat->label = lbnum;
    gosubstat->jumpto = jmp;
    return gosubstat;
}

CAssignStatement* makeassignstat(char* name, struct CExpression* value)
{
    CAssignStatement* assignstat = malloc(sizeof(CAssignStatement));
    cerror(assignstat, "ERROR-not enough memory assignment" , __func__, __LINE__);
    assignstat->name = name;
    assignstat->value = value;
    return assignstat;
}

CArrayStatement* makearraystat(char* name, struct CExpression* value, struct CExpression* valopt)
{
    CArrayStatement* arraystat = malloc(sizeof(CArrayStatement));
    cerror(arraystat, "ERROR-not enough memory  array object" , __func__, __LINE__);
    arraystat->varname = name;
    arraystat->exp = value;
    arraystat->opt = valopt;
    return arraystat;
}

CInputStatement* inputstat(char* name)
{
    CInputStatement* instat = malloc(sizeof(CInputStatement));
    cerror(instat, "ERROR-not enough memory Input Statement object" , __func__, __LINE__);
    instat->name = name;
    return instat;
}

CPrintStatement* printstat(struct LExpression* exp)
{
    CPrintStatement* pstat = malloc(sizeof(CPrintStatement));
    cerror(pstat, "ERROR-not enough memory Print Statement", __func__, __LINE__);
    pstat->expression = exp;
    return pstat;
}


//Find in a table(not done yet
CValue* lookup(char* key, env** varenv)
{
    env* varhead =  *varenv;
    while(varhead)
    {
        if(strcmp(varhead->key,key) == 0)
		{
		   return varhead->vval;
		}
        varhead = varhead->next;
    }
    return NULL;

}

void putvar(char* key , CValue* val)
{
	env* lenv = malloc(sizeof(env));
	cerror(lenv, "ERROR-not enough memory assignment" , __func__, __LINE__);
	lenv->vval = val;
	lenv->next = NULL;
	lenv->key = key;
	if(!tenv)
	{
	   henv = lenv;
	   tenv = lenv;
	}
	else
	{
		lenv->next = henv;
		henv = lenv;
	}
	return;
}

int isnumber(char* str){
	int sl = strlen(str);
	for(int i = 1; i < sl; i++){
		if(isdigit(str[i])){
			continue;
		}
		else {
			return 0;
		}
	}
	return 1;
}

void clean(env** envl)
{
	env* lenv = *envl;
	env* lenvl;
	CValue* val;
	while(1){
		
		if(strlen(lenv->key) < 7 && !isnumber(lenv->key)){
			lenvl =  lenv;
			if(lenvl->vval){
				free(lenvl->vval);
			}
			val = lenvl->vval;
			if (val->vtype == StringValue){
				CStringValue* sn = (CStringValue*)val->value;
				if(sn){
					free(sn);
				}
			}
			else {
				
				CNumberValue* nn = (CNumberValue*)val->value;
				if(nn){
					free(nn);
				}
			}
			if(val){
				free(val);
			}
			if(lenvl){
				free(lenvl);
			}
			lenv = lenv->next;
			continue;
		}
		else {
			for(int i = 0; i < 2 ; i++){
				lenvl =  lenv;
			
				if(lenvl->vval){
					free(lenvl->vval);
				}
				val = lenvl->vval;
				if (val->vtype == StringValue){
					CStringValue* sn = (CStringValue*)val->value;
					if(sn){
						free(sn);
					}
					
				}
				else {
					
					CNumberValue* nn = (CNumberValue*)val->value;
					if(nn){
						free(nn);
					}
					
				}
				if(val){
					free(val);
				}
				if(lenvl){
					free(lenvl);
				}
				
				lenv = lenv->next;
	
			}	
			
		}
	}

}
//Parser help functions
Clist* consume (int typ)
{
    Clist* temp;
    if(currenttoken->type != typ)
    {
        printf("ERROR---Expected type %i and text : %s", typ, currenttoken->text);
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

//Expression evaluation
CValue* eval(struct CExpression* express, struct env** varenv)
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
					case StringValue:
						return cval;
					default:

						cerror(0, "ERROR--unexpected value type", __func__, __LINE__);
					}
				}
				else
				{   
					return NULL;
				}
			}
        case V_value:
            {
                CValue* vval = express->valueexpression; 
                switch(vval->vtype)
                {
					case NumberValue:
					case StringValue:
						return vval;
					default:
						cerror(0, "ERROR--unexpected value type", __func__, __LINE__);
                }

            }
        case V_call:
			{
				FuncCall* fnCall = express->funCall;
				LExpression* exlst = fnCall->LexpList;
				env* cval;
				env* local = *varenv;
				int idx;
				parList* plst;
				CValue* rtval;
				FuncAbstraction* fndecl;
				if(fnCall->tableIndex == -1){
					idx = findTableIndex(fnCall->name);
				} else {
					cerror(0, "Calling Function that was not declared", __func__, __LINE__);
				}
				fnCall->tableIndex  = idx;
				fndecl = funcTable[idx];
				plst = fndecl->plist;
				if(fndecl->parSize != fnCall->argSize){
					cerror(0, "Arguments mismatch", __func__, __LINE__);
				}
				for(int i = 0;  i < fnCall->argSize; i++){
					cval = malloc(sizeof(env));
					if(!cval){
						cerror(0, "Failed to create Cvariable Object for arguments", __func__, __LINE__);
					}
					cval->vval = eval(exlst->arg, varenv);
					cval->key = plst->name->variableexpression->name;
					cval->next = local;
					plst = plst->next;
					local = cval;
					
					exlst = exlst->next;
				}
				rtval = eval(fndecl->body , &local);
				for(int i = 0; i < fnCall->argSize; i++){
					cval = local;
					local = local->next;
					if(cval->vval){
						free(cval->vval);
					}
					free(cval);
				}
				return rtval;	
			}
        case V_operator:
            {
                struct COperatorExpression* cope = express->operatorexpression;
                CValue* leftval, *rightval, *rop;
                leftval = eval(cope->left, varenv);
                rightval = eval(cope->right, varenv);
                rop   =    malloc(sizeof(CValue));
                CNumberValue* dlvalue,*drvalue, *detemp;
                CStringValue* slvalue,*srvalue ;
                cerror(rop, "ERROR--not enough memory to create CValue", __func__, __LINE__);
                switch(cope->coperator)
					{
						
						case TT_REQUAL:
						case TT_NOTEQUAL:
						    {
								int stt = 1;
								if(cope->coperator == TT_NOTEQUAL){
									stt = 0;
								}
								if(leftval->vtype == rightval->vtype && rightval->vtype == NumberValue){
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									if(dlvalue->value == drvalue->value)
									{
										rop->vtype =  stt == 0? CFALSE : CTRUE;
										return rop;
									}
									else
									{
										rop->vtype = stt == 0? CTRUE : CFALSE;
										return rop;
									}
								}
								else if (leftval->vtype == rightval->vtype && rightval->vtype == StringValue){
									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									if(strcmp(slvalue->value, srvalue->value) == 0)
									{
										rop->vtype = stt == 0? CFALSE : CTRUE;
										return rop;
									}
									else
									{
										rop->vtype = stt == 0? CTRUE : CFALSE;
										return rop;
									}
								}
								else
								{
									cerror(0, "Type error: comparing two different type ", __func__, __LINE__);
								}
						    }
						case TT_GREATEROREQUAL:
							{
								if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
								    rop->vtype = dlvalue->value >= drvalue->value? CTRUE : CFALSE;
								   return rop;
						 
								}
								else if(leftval->vtype == StringValue && rightval->vtype == StringValue)
								{
								
									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									rop->vtype = strcmp(slvalue->value, srvalue->value) >= 0? CTRUE : CFALSE;
									return rop;
			
								}
								else
								{
									
									cerror(0, "Type error: comparing two different type " , __func__, __LINE__);
									
								}

							}
						case TT_LESSOREQUAL:
							{
								if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
								    rop->vtype = dlvalue->value <= drvalue->value? CTRUE : CFALSE;
								    return rop;
						 
								}
								else if(leftval->vtype == StringValue && rightval->vtype == StringValue)
								{
								
									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									rop->vtype = strcmp(slvalue->value, srvalue->value) <= 0? CTRUE : CFALSE;
									return rop;
			
								}
								else
								{
									
									cerror(0, "Type error: comparing two different type ", __func__, __LINE__);
									
								}

							}
						case TT_LESS:
							{
								 if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{
								 
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									rop->vtype = dlvalue->value < drvalue->value ? CTRUE : CFALSE;
									return rop;
								}
								else if((leftval->vtype == StringValue)&& (rightval->vtype == StringValue))
								{

									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									rop->vtype = strcmp(slvalue->value, srvalue->value) < 0? CTRUE: CFALSE;
									return rop;
								}
								else {
									cerror(0,"ERROR--Don't mix up types" , __func__, __LINE__);
								}

							}
						case TT_GREATER:
							{
								 if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{

									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									rop->vtype = dlvalue->value > drvalue->value ? CTRUE : CFALSE;
									return rop;
								}
								  if(leftval->vtype == StringValue && rightval->vtype == StringValue)
								{
									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									rop->vtype = strcmp(slvalue->value, srvalue->value) > 0? CTRUE : CFALSE;
									return rop;
								}

								 cerror(0,"ERROR--Don't mix up types" , __func__, __LINE__);
							}
						case '+':
							{
								if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{

									rop->vtype = NumberValue;
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									detemp =  (CNumberValue* )malloc(sizeof(CNumberValue));
									detemp->value = dlvalue->value + drvalue->value;
									rop->value = detemp;
									return rop;
								}
								else if(leftval->vtype == StringValue && rightval->vtype == StringValue)
								{
									rop->vtype = StringValue;
									slvalue = (CStringValue*)leftval->value;
									srvalue = (CStringValue*)rightval->value;
									strcat(slvalue->value, srvalue->value);
									rop->value = slvalue;
									return rop;
								}
								else {

								cerror(0,"ERROR--Don't mix up types for + operator" , __func__, __LINE__);

								}
							}
						case '-':
							{
								if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{

									rop->vtype = NumberValue;
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
									detemp->value = dlvalue->value - drvalue->value;
									rop->value = detemp;
									return rop;
								}
								 cerror(0,"ERROR--Don't mix up types for - operator", __func__, __LINE__ );
							}
						case '*':
							{
								if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{

									rop->vtype = NumberValue;
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
									detemp->value = dlvalue->value * drvalue->value;
									rop->value = detemp;
									return rop;
								}
								 cerror(0, "ERROR--Don't mix up types" , __func__, __LINE__);
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
										cerror(0,"ERROR---division by zero is not allowed", __func__, __LINE__ );
									}
									detemp->value = dlvalue->value / drvalue->value;
									rop->value = detemp;
									return rop;
								}
								 cerror(0,"ERROR--Don't mix up types", __func__, __LINE__ );
							}
						case '%':
							{
							  if(leftval->vtype == NumberValue && rightval->vtype == NumberValue)
								{

									rop->vtype = NumberValue;
									dlvalue = (CNumberValue*)leftval->value;
									drvalue = (CNumberValue*)rightval->value;
									detemp = (CNumberValue* )malloc(sizeof(CNumberValue));
									if (drvalue->value == 0.0)
									{
										cerror(0,"ERROR---Modulo by zero is not allowed", __func__, __LINE__ );
									}
									detemp->value = (int)dlvalue->value % (int)drvalue->value;
									rop->value = detemp;
									return rop;
								}

								cerror(0, "ERROR--Don't mix up types", __func__, __LINE__);

							}


					}

            }

    }

        cerror(0,"ERROR--Don't mix up types!!" , __func__, __LINE__);

}

void exec ()
{
    switch(currentstat->stype)
    {
    case S_print:
        {
            CPrintStatement* ptstat = (CPrintStatement*)currentstat->statement;
            LExpression* largs = ptstat->expression;
            while(largs != NULL){
				CValue* rval =   eval(largs->arg,&henv);
				switch(rval->vtype){
					case NumberValue:
						printf("%f", ((CNumberValue *)rval->value)->value);
						break;
					case StringValue:
						printf("%s", ((CStringValue *)rval->value)->value);
						break;
				}
				largs = largs->next;
			}
			if(ptstat->expression == NULL){
				printf("\n");
			}
			return;
        }
    case S_assign:
        {
            CAssignStatement* astat = (CAssignStatement *)currentstat->statement;
            if(!astat)
            {
                cerror(0, "ERROR--not enough memory" , __func__, __LINE__);
            }
            CValue* val = eval(astat->value, &henv);
            putvar(astat->name, val);
            return;
        }
    case S_goto:
        {
            CGotoStatement* gotostat = (CGotoStatement*)currentstat->statement;
            int jmp = (int)strtol(gotostat->label, (char**)NULL, 10);
            if(jmp < 0){
				//clean(&henv);
				jumpto(jmp, -1 * jmp);
			}
			else {
				jumpto(jmp, 0);
			}
            return;
        }
    case S_ifthen:
        {
            CIfThenStatement* ifstat = (CIfThenStatement*)currentstat->statement;
            CValue* dval = eval(ifstat->condition, &henv);
            int jmp;
            if(dval->vtype == CTRUE)
            {
                //To do change label to int, and make return switch to pc. check out henv
                //pc = jumpto(ifstat->label, 1);
                jmp = (int)strtol(ifstat->label, (char**)NULL, 10);
                jumpto(jmp, 0);

            }
            return;

        }   
    case S_return:
        {
			int jmp = rstack->stack[--rstack->idx];
            jumpto(jmp, 1);
            return;
        }
    case S_gosub:
        {
			CGosubStatement* gosubstat = (CGosubStatement*)currentstat->statement;
            jumpto(gosubstat->jumpto, 0);
			rstack->stack[rstack->idx++] = gosubstat->label;
            return;
		}
    case S_input:
        {
            printf("\nuser input>>");
            CInputStatement* ip = (CInputStatement*)currentstat->statement;
            int buffsize = 256;
            char buf[buffsize];
            fgets(buf, buffsize, stdin);
            int slen = strlen(buf);
            int i = 0;
            while(i <= slen)
            {
                if(isdigit(buf[i]))
                {
                    i++;
                    continue;
                }
                break;
            }
            if(slen - 1 == i)
            {
                double lval = strtod(buf, (char**)NULL);
                CValue* vexp = makenumval(lval)->valueexpression;
                putvar(ip->name, vexp);
                return;
            }
            else
            {
				char *tmp = malloc(sizeof(char)*slen);
				strncpy(tmp, buf, slen-1);
				tmp[slen] = '\0';
                CValue* sexp = makestrval(tmp)->valueexpression;
                putvar(ip->name, sexp);
                return;
            }
        }


    }
}


CStatement* parser ()
{
    char* labelnumber;
    const int LOCAL_STAT_MAX = 100;
    int stepindex = 0;
    struct labelList* locallst = malloc(sizeof(struct labelList));
    cerror(locallst, "ERROR- setting label inside a For loop not allowed" , __func__, __LINE__);
    locallst->top = 0;
    while(1)
    {
        while(match(TT_LINE)){ }
        // printf("break out new line");
        if(match(TT_NUMBER))
        {
            if (setinjump)
            {
                cerror(0, "ERROR- setting label inside a For loop not allowed" , __func__, __LINE__);
            }
            labelnumber = stepback->text;
			if(match(DEF))
			{   
				char* funame = consume(TT_WORD)->text;
				if(!match(TT_LEFT_PAREN)){
					cerror(0, "ERROR- Expected opening paren for function" , __func__, __LINE__);
				}
				FuncAbstraction* funabs = malloc(sizeof(FuncAbstraction));
				funabs->name = funame;
				funabs->parSize = 0;
				funabs->plist = malloc(sizeof(parList*));
				parList* params = funabs->plist;
				if(!match(TT_RIGHT_PAREN)){
					params->name = expression();
					++funabs->parSize;
					while(matchstring(",")){
						params->next = 	malloc(sizeof(parList));
						params->next->name = expression();
						params = params->next;
						++funabs->parSize;
						if(match(TT_RIGHT_PAREN)){
							break;
						}
					}

				}
				else
				{
					params->name = NULL;
					consume(TT_RIGHT_PAREN);
				}
				consume(TT_EQUAL);
				struct CExpression* body = expression();
				funabs->body = body;
				funcTable[fntabidx++] = funabs;
				addstat(NULL, 0,statsize,labelnumber,1);
				++statsize;
			}
			else if(match(LET))
			{
				char* name = consume(TT_WORD)->text;
				consume(TT_EQUAL);
				CAssignStatement* astat = makeassignstat(name, expression());
				addstat(astat, S_assign,statsize,labelnumber,0);
				++statsize;
			}
			else if(match(DIM))
			{
				char* name = consume(TT_WORD)->text;
				struct CExpression* fst;
				struct CExpression* optional = NULL;
				consume(TT_LEFT_PAREN);
				fst = expression();
				if(matchstring(",")){
					optional = expression();
				}
				consume(TT_RIGHT_PAREN);
				CArrayStatement* arstat = makearraystat(name, fst, optional);
				addstat(arstat, S_array, statsize,labelnumber,0);
				++statsize;
			}
			else if(match(FOR))
			{   
				if(match(TT_WORD))
				{
					if(match(TT_EQUAL)){
						
						char* name = doubleback->text;
						char xname[60];
						char stepname[60];
						struct CExpression* inex, *texpf, *texpss, *cond ,*valex, *valop;
						CAssignStatement* astat;
						CIfThenStatement* ifst;
						Clabels* clb;
						time_t seconds;
						seconds = time(NULL);
						//consume(TT_EQUAL);
						astat = makeassignstat(name, expression());
						addstat(astat, S_assign,statsize,labelnumber,0);
						++statsize;
						consume(TO);
						sprintf(xname, "%s%ld",  name, seconds);
						inex = expression();
						astat = makeassignstat(strdup(xname), inex);
						addstat(astat, S_assign,statsize,labelnumber,0);
						++statsize;
						texpf = makevarexpression(name);
						texpss = makevarexpression(strdup(xname));
						cond = makeoperatorexpress(texpss, TT_LESSOREQUAL, texpf);
						ifst = makeifthenstat(cond, "0");
				        addstat(ifst, S_ifthen, statsize,labelnumber,0);
				       	clb  = malloc(sizeof(Clabels));
						cerror(clb, "ERROR--not enough memory to create Clabels" , __func__, __LINE__);
				        clb->key = strtol(labelnumber, (char**)NULL, 10);
						clb->cindex = statsize ;
						clb->st = (void *)ifst;
						if(locallst->top - 1 < LOCAL_STAT_MAX){
							locallst->clabs[locallst->top++] = clb;
						}
				        ++statsize;
				        valex = makenumval(1);
				        if(match(STEP)){
							valex = expression();

						}
						valop = makeoperatorexpress(texpf, '+', valex);
						astat = makeassignstat(name, valop);
				        clb  = malloc(sizeof(Clabels));
						cerror(clb, "ERROR--not enough memory to create Clabels" , __func__, __LINE__);
				        clb->key = strtol(labelnumber, (char**)NULL, 10);
						clb->cindex = statsize - 1;
						clb->st = (void *)astat;
    
						if(locallst->top - 1 < LOCAL_STAT_MAX){
							locallst->clabs[locallst->top++] = clb;
						}
				        
					}
					else
					{
						cerror(0, "Wrongly structured FOR statement" , __func__, __LINE__);
						
					}
				}
				else
				{
					cerror(0, "Wrongly structured FOR statement", __func__, __LINE__);
				}
			}
			else if(match(PRINT))
			{
				LExpression* hd;
				LExpression* tl;
				struct CExpression* value;
				CPrintStatement* pst;
				if(match(TT_LINE)){
					hd = NULL;
				}
				else
				{
					hd = malloc(sizeof(LExpression));
				    cerror(hd, "Failed to create expression list" , __func__, __LINE__);
				    value = expression();
				    hd->arg = value;
				    tl = hd;
				    
				    while(match(TT_COMMA)){
						value = expression();
						tl->next = malloc(sizeof(LExpression));
						tl->next->arg = value;
						tl = tl->next;
					}
					tl->next = NULL;
				}
				pst = printstat(hd);
				addstat(pst, S_print, statsize,labelnumber,0);
				++statsize;
			}
			else if(match(INPUT))
			{
			  Clist* tk = consume(TT_WORD);
			  CInputStatement* in = inputstat(tk->text);
			  addstat(in, S_input,statsize,labelnumber,0);
			  ++statsize;

			}
			else if(match(GOTO))
			{
				if(!match(TT_LINE))
				{
					CGotoStatement* gstat = makegotostat(consume(TT_NUMBER)->text);
					addstat(gstat, S_goto, statsize,labelnumber,0);
					++statsize;
				}
				else
				{
					cerror(0, "ERROR--this label was not declared ", __func__, __LINE__ );
				}
				continue;
			}
			else if(match(GOSUB))
			{
				if(!match(TT_LINE))
				{
					int jmpto = (int)strtol(consume(TT_NUMBER)->text, (char**)NULL, 10);
					int llnum = (int)strtol(labelnumber, (char**)NULL, 10);
					CGosubStatement* gosub = makegosubstat(jmpto, llnum);
					addstat(gosub, S_gosub, statsize,labelnumber, 0);
					++statsize;
				}
				else
				{
					cerror(0, "ERROR--this label was not declared " , __func__, __LINE__ );
				}
				continue;
				
			}
			else if(match(RETURN))
			{
				CGotoStatement* rtn = makegotostat("-1");
				addstat(rtn , S_return, statsize,labelnumber, 0);
				++statsize;
				continue;

			}
			else if(match(DATA))
			{
				
				
			}
			else if(match(IF))
			{
				char* wlab;
				struct CExpression* cond = condexpression();
				
				consume(THEN);
				wlab = consume(TT_NUMBER)->text;
				CIfThenStatement* ifst = makeifthenstat(cond, wlab);
				addstat(ifst, S_ifthen, statsize,labelnumber,0);
				++statsize;
				while(match(TT_LINE)){ }
				continue;
			}
			else if(match(NEXT))
			{
				char govl[20];
				if(locallst->top <= 0){
					cerror(0, "Semantics error, there should be a FOR before NEXT", __func__, __LINE__);
				}
				CAssignStatement* astat = (CAssignStatement* )locallst->clabs[--locallst->top]->st;
				int nkey = locallst->clabs[locallst->top]->key;
				int cidx = locallst->clabs[locallst->top]->cindex;
				free(locallst->clabs[locallst->top]);
				sprintf(govl, "-%d", cidx);
				if(strcmp(astat->name, currenttoken->text) != 0){
					cerror(0, "Semantics error, there should be a FOR before NEXT" , __func__, __LINE__);
				}
				consume(TT_WORD);
				addstat(astat, S_assign, statsize,labelnumber,0);
				statsize++;
				CGotoStatement* gstat = makegotostat(strdup(govl));
				addstat(gstat, S_goto, statsize,labelnumber,0);
				statsize++;
				while(match(TT_LINE)){ }
				if(currenttoken->type == TT_NUMBER){
					CIfThenStatement* ifstat = (CIfThenStatement* )locallst->clabs[--locallst->top]->st;
				    ifstat->label = currenttoken->text;
				    free(locallst->clabs[locallst->top]);
				}
				else {
					cerror(0, "Semantics error  FOR - NEXT statements " , __func__, __LINE__);
				}
	
			}   
		}
		if (!(currenttoken))
		{
			break;
		}	
    }
	if(locallst->top != 0){
		cerror(0, "Smantics error, there should be a FOR before NEXT" , __func__, __LINE__);
	} 
    free(locallst);
    return hstat;
}
Clist* makeToken(int tsize, char* text, int type)
{
    Clist* temp = (Clist* )malloc(sizeof(Clist));
    char* stkn;
    int ntyp = type;
    cerror(temp, "ERROR: not enough memory available" , __func__, __LINE__);
    temp->next = NULL;
    stkn =  (char*)malloc(sizeof(char)*(tsize + 1));
    cerror(stkn, "ERROR: not enough memory available" , __func__, __LINE__);
    strncpy(stkn, (const char*)text, (size_t)tsize);
    stkn[tsize] = '\0';
    temp->text = stkn;
	
	for(int i = 0; reservedword[i] != NULL; i++){
		if(strcmp(reservedword[i], stkn) == 0){
			ntyp = i;
		}
	}	
    if(TT_WORD == ntyp)
    {
        insertlabel(stkn);
    }
    temp->type = ntyp;
    return temp;

}

int isRelational(char c){
	
	switch(c){
		case '=':
		case '>':
		case '<':
			return 1;
		default:
			return 0;
			
	}
	
}

int matchrelational(){
	if(currenttoken->type == 13 && (strcmp("THEN", currenttoken->text) == 0)){
		return 0;
		//hacky ..to work.
	}
	switch(currenttoken->type){
		case TT_GREATEROREQUAL:
		case TT_LESSOREQUAL:
		case TT_REQUAL:
		case TT_NOTEQUAL:
		case TT_GREATER:
		case TT_LESS:
			break;
		default:
			return 0;
	}
	doubleback = stepback;
    stepback = currenttoken;
    currenttoken = currenttoken->next;
    return 1;
}
		
int updateOp(char c, char e){
	if(c == '<' && e == '>'){
		return TT_NOTEQUAL;
	}
	else if(c == '>' || c == '<'){
		if(e == '='){
			if(c == '>'){
				return TT_GREATEROREQUAL;
			}
			else {
				return TT_LESSOREQUAL;
			}
		}
		else {
			cerror(0 , "First Poorly structured relational operator", __func__, __LINE__);
		}
	}
	else if(c == '=' && e == '='){
		return TT_REQUAL;
	}
	else {
		cerror(0 , "Poorly structured relational operator", __func__, __LINE__);
	}
	
}


char* cconcat(char c, char e){
	char* tmp = malloc(sizeof(char) * 3);
	tmp[0] = c;
	tmp[1] = e;
	tmp[2] = '\0';
	return tmp;
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
    char c;
    int state = TS_DEFAULT;
    int tkntype;
    int charcount = 0;
    char* strptr = 0;
    char mcop;
    char charTokens[] = "\n=+-*/%<>(),";
    while((c = *source++) != 0)
    {
        switch(state)
        {
        case TS_DEFAULT:
			if(strptr = strchr(charTokens, c))
			{
				mcop = *strptr;
				char sgn;
				char* sstrptr = 0;
				int ttkntype;
				tkntype = getoptype(mcop);
				int limit = 1;
				if(isRelational(mcop)){
					sgn =  source[0];
					if(mcop == '>'){
						tkntype = TT_GREATER;
					}
					if(mcop == '<'){
						tkntype = TT_LESS;
					}
					if((sstrptr = strchr(charTokens, sgn))){
						if(isRelational(*sstrptr)){
							tkntype = updateOp(mcop, *sstrptr);
							limit = 2;
							source++;
							strptr = cconcat(mcop, sgn);
							
						}
					}
					
				}
				addToList(makeToken(limit, strptr,tkntype));
				if(limit == 2){
					free(strptr);
				}
				state = TS_DEFAULT;

			}
			else if(isalpha(c))
			{
				strptr = source - 1;
				++charcount;
				state = TS_WORD;
			}
			else if(isdigit(c))
			{
				strptr = source - 1;
				++charcount;
				state = TS_NUMBER;

			}
			else if(c == '"')
			{
			    strptr = source;
			    state = TS_STRING;
			}

			else if(c == '\'')
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
                if(c == ':')
                {
                    addToList(makeToken(charcount, strptr, TT_LABEL));
                    charcount = 0;
                    state = TS_DEFAULT;
                    source--;
                }
                else if((*source == '\0')&& isalnum(c))
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

	return;
}



struct CExpression* expression()
{
    return coperator();
}


struct CExpression* catomic ()
{
    if(!currenttoken) return 0;
    if(match(TT_WORD))
    {   
		char* nm = stepback->text;
		if(match(TT_LEFT_PAREN)){
			LExpression* el = NULL;
			int cnt = 0;
			if(!match(TT_RIGHT_PAREN)){
				el = malloc(sizeof(LExpression));
				LExpression* tl;
				LExpression* tp;
				cnt++;
				cerror(0, "Failed to build list expressions", __func__, __LINE__);
				el->arg = expression();
				tl = el;
				while(matchstring(",")){
					tp = malloc(sizeof(LExpression));
					tp->arg = expression();
					tl->next = tp;
					tl = tp;
					cnt++;
				}
				tl->next = NULL;
				consume(TT_RIGHT_PAREN);
				
			}
			return makefunCall(nm, el, cnt);	
		}
		else {
			return makevarexpression(nm);
		}
        
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
    printf("ERROR--could not parse :( %s\n" , currenttoken->text);
    exit(0);
}


struct CExpression* coperator()
{
    struct CExpression* express = catomic();
    int cop;
    while(match(TT_OPERATOR))
    {
        cop = stepback->text[0];
        struct CExpression* right = catomic();
        if(!right)
        {
            break;
        }
        express = makeoperatorexpress(express, cop, right);
    }

    return express;
}

struct CExpression* condexpression()
{
    struct CExpression* express = expression();
    struct CExpression* right;
    int cop;
    int intw = 1;
    while(matchrelational())
    {
		intw = 0;
        cop = stepback->type;
        right = expression();
        if(!right)
        {
			intw = 1;
            break;
        }
        express = makeoperatorexpress(express, cop, right);
        
    }
    if(intw == 1){
		cerror(0, "Expected binary operator fro conditional expressions" , __func__, __LINE__);
	}
    return express;
}

char* readfile(char* filename){
	int flen = strlen(filename);
	FILE* fd;
	int buflen;
	char* buf;
	int byteslen;
	if(flen > 3 && strcmp(".cb", filename + flen - 3) == 0){
		fd = fopen(filename, "r");
		if(!fd){
			cerror(0, "Failed to open source file" , __func__, __LINE__);
	    }
		fseek(fd, 0L, SEEK_END);
		buflen = ftell(fd);
		if(buflen == -1){
			cerror(0, "Seek failed to get file offset" , __func__, __LINE__);
		}
		rewind(fd);
		buf = malloc(buflen * sizeof(char));
		if(!buf){
			cerror(0, "Failed to create buf for source file contain" , __func__, __LINE__);
		}
		byteslen = fread(buf, 1 , buflen + 1, fd);
		if(byteslen == 0){
			cerror(
			0, "Failed to read source file" , __func__, __LINE__);
		}
		buf[buflen] = '\0';
		fclose(fd);
		return buf;
			
	}
	else{
		cerror(0, "cbasic files must end with .cb" , __func__, __LINE__);
	}
}
	
int main(int argc, char** argv)
{
	if(argc < 2){
		cerror(llst, "Source file not included" , __func__, __LINE__);
	}
    char* filedata = readfile(argv[1]);
    tokenize(filedata);
    Clist** hold = &head;
    Clist* lhead = *hold;
	llst = malloc(sizeof(struct labelList));
	rstack = malloc(sizeof(ReturnStack));
	cerror(rstack, "Failed to create ReturnStack object" , __func__, __LINE__);
	cerror(llst, "Failed to create labelList object" , __func__, __LINE__);
	llst->top = 0;
	rstack->idx = 0;
	/**
    while(lhead )
    {   printf("\n");
        printf("%s", lhead->text);
        printf("tap\n");
        printf("%i", lhead->type);
        lhead = lhead->next;
        if(lhead) printf("empty head");
    }
    **/
	currenttoken = head;
	parser();
	currentstat = hstat;
	//pc = currentstat;
	while(currentstat)
	{
		exec();
		if(pc){
			currentstat = pc;
			pc = NULL;
		}
		else {
			currentstat = currentstat->next;
		}
	
	}
    Clist* temp;
    while(head)
    {
        free(head->text);
        temp = head;
        head = head->next;
        free(temp);
    }  
    free(filedata);
    free(rstack);
    return 0;
}

