# CBasic
This is a simple Basic Language. It covers the subset of the language 
Version:: 0.0001
CBasic...a C port of Basic, the version popular in the 60s
Presently it lacks the following:
   FOR loop
    Array
   READ and DATA
   user function definition

We hope that you will use this version to learn how interpreter(if you ready don't know) are crafted, and also give us feedback in the area of code design. 

 To play with in go to main function and carefully edit this variable ..  
char* ytest  = "foo = (314159 * 10) - 3000000 \n print foo \n  \n tin = 3 \n if tin > tin then fin    \n  print foo  \n input guess \n print foo / guess  \n fin: \n  print \"Well Done\" \n print tin  ";
 You can also include in the main function the ability to read a whole file once and use the result to call function tokenize . However, you have to edit the first while loop inside tokenize.... instead of while((c = *source++) != 0) change to while((c = *source++) != EOF)

Tell us your findings. We intend to add Array, Read, and Data store in the next version(they are virtually done). We are also working to include user function definition and FOR loop.

Things it can do:
 Assignment Statement  :: foo = (314159 * 10) - 3000000
Print Statement :: -> print expression  :: print foo
Input Statement::-> input variable  :: input guess
If Statement and Label ::-> if  expression then label ... you declare label like goo: , so if you have  --if 8 < 10 then goo, you program will jump to goo
Goto Statement:: First label .. Koo: , for later just call goto Koo. Note that you can position label before or after goto.

On expression operator we have :
  + op for both number and string(concatenation) 
  -  op for only number
 /  op for only number
 * op for only number
< for both number and string
> for both number and string

We missed out freeing memory in some places .. see if you can't figure tha
