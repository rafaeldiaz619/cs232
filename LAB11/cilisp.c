#include "cilisp.h"
#include "math.h"

#define RED             "\033[31m"
#define RESET_COLOR     "\033[0m"

// yyerror:
// Something went so wrong that the whole program should crash.
// You should basically never call this unless an allocation fails.
// (see the "yyerror("Memory allocation failed!")" calls and do the same.
// This is basically printf, but red, with "\nERROR: " prepended, "\n" appended,
// and an "exit(1);" at the end to crash the program.
// It's called "yyerror" instead of "error" so the parser will use it for errors too.
void yyerror(char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 255, format, args);

    printf(RED "\nERROR: %s\nExiting...\n" RESET_COLOR, buffer);
    fflush(stdout);

    va_end (args);
    exit(1);
}

// warning:
// Something went mildly wrong (on the user-input level, probably)
// Let the user know what happened and what you're doing about it.
// Then, move on. No big deal, they can enter more inputs. ¯\_(ツ)_/¯
// You should use this pretty often:
//      too many arguments, let them know and ignore the extra
//      too few arguments, let them know and return NAN
//      invalid arguments, let them know and return NAN
//      many more uses to be added as we progress...
// This is basically printf, but red, and with "\nWARNING: " prepended and "\n" appended.
void warning(char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 255, format, args);

    printf(RED "WARNING: %s\n" RESET_COLOR, buffer);
    fflush(stdout);

    va_end (args);
}

FUNC_TYPE resolveFunc(char *funcName)
{
    // Array of string values for function names.
    // Must be in sync with members of the FUNC_TYPE enum in order for resolveFunc to work.
    // For example, funcNames[NEG_FUNC] should be "neg"
    char *funcNames[] = {
            "neg",
            "abs",
            "add",
            "sub",
            "mult",
            "div",
            "remainder",
            "exp",
            "exp2",
            "pow",
            "log",
            "sqrt",
            "cbrt",
            "hypot",
            "max",
            "min",
            "rand",
            "read",
            "equal",
            "less",
            "greater",
            "print",
            "custom",

            // TODO complete the array
            // the empty string below must remain the last element
            ""
    };
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], funcName) == 0)
        {
            return i;
        }
        i++;
    }
    return CUSTOM_FUNC;
}

NUM_TYPE resolveType(char * type)
{
    if(strcmp(type, "int") == 0 ) return 0;
    else if(strcmp(type, "double") == 0) return 1;
}

AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    // TODO complete the function finished
    // Populate "node", the AST_NODE * created above with the argument data.
    // node is a generic AST_NODE, don't forget to specify it is of type NUMBER_NODE
    node->data.number.value = value;
    node->data.number.type = type;
    node->type = NUM_NODE_TYPE;

    return node;
}


AST_NODE *createFunctionNode(FUNC_TYPE func, AST_NODE *opList)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    // TODO complete the function finished
    // Populate the allocated AST_NODE *node's data
    node->type = FUNC_NODE_TYPE;
    node->data.function.func = func;
    node->data.function.opList = opList;
    while(opList != NULL)
    {
        opList->parent = node;
        opList = opList->next;
    }

    return node;
}

AST_NODE *createCustomFunctionNode(char *id, AST_NODE *opList)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    // TODO complete the function finished
    // Populate the allocated AST_NODE *node's data
    node->type = FUNC_NODE_TYPE;
    node->data.function.func = CUSTOM_FUNC;
    node->data.function.opList = opList;
    while(opList != NULL)
    {
        opList->parent = node;
        opList = opList->next;
    }

    return node;
}

AST_NODE *addExpressionToList(AST_NODE *newExpr, AST_NODE *exprList)
{

    newExpr->next = exprList;
    return newExpr;
}

AST_NODE *createSymbolNode(char *id)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE) + sizeof(AST_SYMBOL);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->type = SYM_NODE_TYPE;
    node->data.symbol.id = id;

    return node;
}

AST_NODE *createScopeNode(SYMBOL_TABLE_NODE *let_section, AST_NODE *s_expr)
{
    AST_NODE *scopeNode;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((scopeNode = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    scopeNode->type = SCOPE_NODE_TYPE;
    s_expr->parent = scopeNode;
    scopeNode->data.scope.child = s_expr;
    s_expr->symbolTable = let_section;
    while (let_section != NULL)
    {
        let_section->value->parent = s_expr;
        let_section = let_section->next;
    }



    return scopeNode;
}

AST_NODE *createCondNode(AST_NODE *condition, AST_NODE *trueValue, AST_NODE *falseValue)
{
    AST_NODE *cond;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((cond = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    cond->type = COND_NODE_TYPE;
    cond->data.conditional.condition = condition;
    cond->data.conditional.ifTrue = trueValue;
    cond->data.conditional.ifFalse = falseValue;
    condition->parent = cond;
    trueValue->parent = cond;
    falseValue->parent = cond;

    return cond;
}

// add symbol to the list
SYMBOL_TABLE_NODE *let_list(SYMBOL_TABLE_NODE *let_elem, SYMBOL_TABLE_NODE *let_list)
{
    SYMBOL_TABLE_NODE *table = let_list;

    int redeclaration = 0;
    while(table)
    {
        if(strcmp(let_elem->id, table->id) == 0)
        {
            warning("Duplicate assignment to symbol \"%s\" detected in the same scope!\n"
                    "Only the first assignment is kept!", table->id);
            redeclaration = 1;
            let_list = let_elem;    // let_list takes the value of the head of the let_elem
            break;
        }
        table = table->next;
    }

    // if it is not a redeclaration, place the symbol at the front of the let_list
    if(!redeclaration)
        let_elem->next = let_list;

    return let_elem;
}

// create symbol table node with data
SYMBOL_TABLE_NODE *createVariableTableNode(NUM_TYPE type, char *id, AST_NODE *s_expr)
{
    SYMBOL_TABLE_NODE *symbolTableNode;
    size_t nodeSize;

    nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((symbolTableNode = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    symbolTableNode->id = id;
    symbolTableNode->value = s_expr;
    symbolTableNode->type = type;
    symbolTableNode->symbolType = VAR_TYPE;

    return symbolTableNode;
}

SYMBOL_TABLE_NODE *createFunctionTableNode( NUM_TYPE type, char *id, SYMBOL_TABLE_NODE *arg_list, AST_NODE *s_expr)
{
    SYMBOL_TABLE_NODE *symbolTableNode;
    size_t nodeSize;

    nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((symbolTableNode = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    symbolTableNode->id = id;
    symbolTableNode->value = s_expr;
    s_expr->symbolTable = arg_list;
    symbolTableNode->type = type;
    symbolTableNode->symbolType = LAMBDA_TYPE;

    return symbolTableNode;
}

SYMBOL_TABLE_NODE *createArgTable(char *id, SYMBOL_TABLE_NODE *arg_list)
{
    SYMBOL_TABLE_NODE *table;

    size_t nodeSize;

    nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((table = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    table->id = id;
    table->type = NO_TYPE;
    table->symbolType = ARG_TYPE;
    table->next = arg_list;

    return table;
}

RET_VAL evalNeg(AST_NODE *node)
{
    if(!node)
    {
        warning("No operands in Neg function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("neg called with extra (ignored) operands!");
    }

    RET_VAL result;
    result = eval(node);
    result.value = -result.value;
    result.type = result.type;
    return result;

}

RET_VAL evalAbs(AST_NODE *node)
{

    if(!node)
    {
        warning("No operands in Neg function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Too many operands in Abs function");

    }
    RET_VAL result;
    result.value = 0, result.type = INT_TYPE;
    result = eval(node);
    result.value = fabs(result.value);
    result.type = result.type;
    return result;
}

RET_VAL evalAdd(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 0, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;

    if(node == NULL)
    {
        warning("Add called with no operands! 0 returned!");
        result.type = INT_TYPE;
        return result;
    }


    while(node != NULL)
    {
        result2 = eval(node);

        result.value += result2.value;
        if(result.type == DOUBLE_TYPE || result2.type == DOUBLE_TYPE)
            result.type = DOUBLE_TYPE;
        else
            result.type = INT_TYPE;

        node = node->next;
    }

    return result;
}


RET_VAL evalSub(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 0, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;
    if(!node)
    {
        warning("No operands in Sub function");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("WARNING: Sub called with only one arg!");
        return NAN_RET_VAL;
    }
    else if(node->next->next)
    {
        warning("Sub called with extra (ignored) operands!");
    }

    result = eval(node);
    result2 = eval(node->next);
    result.value -= result2.value;
    if(result.type == DOUBLE_TYPE || result2.type == DOUBLE_TYPE)
        result.type = DOUBLE_TYPE;
    else
        result.type = INT_TYPE;
    return result;



}

// TODO - DEBUGGING REQUIRED - DOES NOT PRINT RIGHT NUM_TYPE IN SPECIFIC CIRCUMSTANCES
RET_VAL evalMult(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 1, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;


    if(node == NULL)
    {
        warning("Mult called with no operands! 1 returned!");
        result.type = INT_TYPE;
        return result;
    }

    while(node != NULL)
    {
        result2 = eval(node);

        result.value *= result2.value;
        if(result.type == DOUBLE_TYPE || result2.type == DOUBLE_TYPE)
            result.type = DOUBLE_TYPE;
        else
            result.type = INT_TYPE;
        node = node->next;

    }

    return result;
}

RET_VAL evalDiv(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 1, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in div function");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("WARNING: div called with only one arg!");
        return NAN_RET_VAL;
    }
    else if(node->next->next)
    {
        warning("Sub called with extra (ignored) operands!");
    }

    result = eval(node);
    result2 = eval(node->next);

    if(result2.value == 0)
    {
        warning("You cannot divide by zero!");
        return NAN_RET_VAL;
    }

    if(result.type == INT_TYPE && result2.type == INT_TYPE)
    {
        if(remainder(result.value, result2.value) != 0)
        {
            result.value /= result2.value;
            result.value = floor(result.value);
        }
        else result.value /= result2.value;
    }
    else
        result.value /= result2.value;

    if(result.type == DOUBLE_TYPE || result2.type == DOUBLE_TYPE)
        result.type = DOUBLE_TYPE;
    else
        result.type = INT_TYPE;
    return result;
}

RET_VAL evalRemainder(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 1, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in remainder function");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("WARNING: Remainder called with only one arg!");
        return NAN_RET_VAL;
    }
    else if(node->next->next)
    {
        warning("Remainder called with extra (ignored) operands!");
    }
    result = eval(node);
    result2 = eval(node->next);
    result.value = remainder(result.value, result2.value);
    if (result.value < abs(result2.value) && result.value < 0)
    {
        result.value += abs(result2.value);
    }
    if(result.type == DOUBLE_TYPE || result2.type == DOUBLE_TYPE)
        result.type = DOUBLE_TYPE;
    else
        result.type = INT_TYPE;
    return result;
}

RET_VAL evalExp(AST_NODE *node)
{
    if(!node)
    {
        warning("No operands in Exp function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Exp called with extra (ignored) operands!");

    }

    RET_VAL result;
    result.value = 1, result.type = INT_TYPE;
    result = eval(node);
    result.value = exp(result.value);
    result.type = DOUBLE_TYPE;
    return result;
}

RET_VAL evalExp2(AST_NODE *node)
{
    if(!node)
    {
        warning("No operands in Exp2 function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Exp2 called with extra (ignored) operands!");

    }

    RET_VAL result;
    result.value = 1, result.type = INT_TYPE;
    result = eval(node);
    result.value = exp2(result.value);
    if (node->data.number.value < 0)
        result.type = DOUBLE_TYPE;
//    else
//        result->type = result->type;
    return result;
}

RET_VAL evalPow(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;

    result.value = 1, result.type = INT_TYPE;
    result2.value = 1, result2.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in Pow function");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("Pow called with no second operand!");
        return NAN_RET_VAL;
    }
    else if(node->next->next)
    {
        warning(" Pow called with extra (ignored) operands!");
    }

    result = eval(node);
    result2 = eval(node->next);
    result.type = result.type || result2.type;
    result.value = pow(result.value, result2.value);

    return result;
}

RET_VAL evalLog(AST_NODE *node)
{
    RET_VAL result;
    result.value = 1, result.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in Log function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Log called with extra (ignored) operands!");

    }
    result = eval(node);
    result.value = log(result.value);
    result.type = DOUBLE_TYPE;
    return result;
}

RET_VAL evalSqrt(AST_NODE *node)
{
    RET_VAL result;
    result.value = 1, result.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in Sqrt function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Sqrt called with extra (ignored) operands!");

    }

    result = eval(node);
    result.value = sqrt(result.value);
    result.type = DOUBLE_TYPE;
    return result;
}

RET_VAL evalCbrt(AST_NODE *node)
{
    RET_VAL result;
    result.value = 1, result.type = INT_TYPE;

    if(!node)
    {
        warning("No operands in Cbrt function");
        return NAN_RET_VAL;
    }
    else if(node->next)
    {
        warning("Cbrt called with extra (ignored) operands!");

    }

    result = eval(node);
    result.value = cbrt(result.value);
    result.type = DOUBLE_TYPE;
    return result;
}

RET_VAL evalHypot(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;
    result.value = 0;

    if(node == NULL)
    {
        warning("WARNING: No operands detected!");
        return NAN_RET_VAL;
    }


    while(node != NULL)
    {
        result2 = eval(node);
        result2.value = pow(result2.value, 2);
        result.value += result2.value;

        //printf("add %f", node->data.number.value);

        node = node->next;
    }
    result.type = DOUBLE_TYPE;
    result.value = sqrt(result.value);

    return result;
}

RET_VAL evalMax(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;
    result.value = -1000;

    if(!node)
    {
        warning("No operands detected!");
        return NAN_RET_VAL;
    }
    while(node != NULL)
    {
        result2 = eval(node);
        if(result.value < result2.value)
        {
            result.value = result2.value;
            result.type = result2.type;
        }

        node = node->next;
    }
    return result;
}

RET_VAL evalMin(AST_NODE *node)
{
    RET_VAL result;
    RET_VAL result2;
    result.value = 1000;

    if(!node)
    {
        warning("No operands detected!");
        return NAN_RET_VAL;
    }
    while(node != NULL)
    {
        result2 = eval(node);
        if(result.value > result2.value)
        {
            result.value = result2.value;
            result.type = result2.type;
        }

        node = node->next;
    }
    return result;
}

RET_VAL evalRand()
{
    RET_VAL result;
    result.value = (double)rand() / (double)RAND_MAX;
    result.type = DOUBLE_TYPE;
    return result;
}

// TODO
RET_VAL evalRead()
{
    RET_VAL result;
    printf("read :: ");
    if(read_target == 0)
    {
        double value;
        scanf("%lf", &value);
        result.type = DOUBLE_TYPE;
        result.value = value;
    }
    else
    {
        double value;
        fscanf(read_target,"%lf", &value);
        result.type = DOUBLE_TYPE;
        result.value = value;
    }

    return result;
}

RET_VAL evalEqual(AST_NODE *node)
{
    RET_VAL num1;
    RET_VAL num2;
    if(!node)
    {
        warning("No operands detected!");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("Second operand not detected in Equal!");
        return NAN_RET_VAL;
    }

    num1 = eval(node);
    num2 = eval(node->next);
    if(num1.value == num2.value)
        num1.value = 1;
    else
        num1.value = 0;

    num1.type = INT_TYPE;

    return num1;
}

RET_VAL evalLess(AST_NODE *node)
{
    RET_VAL num1;
    RET_VAL num2;
    if(!node)
    {
        warning("No operands detected!");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("Second operand not detected in Equal!");
        return NAN_RET_VAL;
    }

    num1 = eval(node);
    num2 = eval(node->next);

    if(num1.value < num2.value)
        num1.value = 1;
    else
        num1.value = 0;

    num1.type = INT_TYPE;

    return num1;
}

RET_VAL evalGreater(AST_NODE *node)
{
    RET_VAL num1;
    RET_VAL num2;
    if(!node)
    {
        warning("No operands detected!");
        return NAN_RET_VAL;
    }
    else if(!node->next)
    {
        warning("Second operand not detected in Equal!");
        return NAN_RET_VAL;
    }

    num1 = eval(node);
    num2 = eval(node->next);

    if(num1.value > num2.value)
        num1.value = 1;
    else
        num1.value = 0;

    num1.type = INT_TYPE;

    return num1;
}

RET_VAL evalPrint(AST_NODE *node)
{
    RET_VAL result;

    if(!node)
    {
        warning("print called with no operands!");
        return NAN_RET_VAL;
    }
    if(node->next != NULL)
        warning("print called with extra (ignored) operands!");
    result = eval(node);
    printRetVal(result);
    return result;
}
// TODO 
RET_VAL evalCustomFunction(AST_NODE *node)
{
//    RET_VAL result;
//    STACK_NODE *stack;
//    AST_NODE *functionNode;
//
//    if(!node)
//    {
//        warning("No operands detected!");
//        return NAN_RET_VAL;
//    }
//
//    functionNode = node->parent;
//
//    while(node != NULL)
//    {
//        result = eval(node);
//        stack->value = result;
//        stack = stack->next;
//        node = node->next;
//    }
//
//    while(functionNode->data.scope.child->symbolTable != NULL)
//    {
//        functionNode->data.scope.child->symbolTable->stack = stack;
//        functionNode->data.scope.child->symbolTable = functionNode->data.scope.child->symbolTable->next;
//        stack = stack->next;
//    }
//
//    stack->next = NULL;
//
//
//    return result;

}

RET_VAL evalFuncNode(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into evalFuncNode!");
        return NAN_RET_VAL; // unreachable but kills a clang-tidy warning
    }

    // TODO complete the function finished
    // HINT:
    // the helper functions that it calls will need to be defined above it
    // because they are not declared in the .h file (and should not be)
    RET_VAL result;
    switch(node->data.function.func)
    {
        case NEG_FUNC:
            result = evalNeg(node->data.function.opList);
            break;
        case ADD_FUNC:
            result = evalAdd(node->data.function.opList);
            break;
        case ABS_FUNC:
            result = evalAbs(node->data.function.opList);
            break;
        case SUB_FUNC:
            result = evalSub(node->data.function.opList);
            break;
        case MULT_FUNC:
            result = evalMult(node->data.function.opList);
            break;
        case DIV_FUNC:
            result = evalDiv(node->data.function.opList);
            break;
        case REM_FUNC:
            result = evalRemainder(node->data.function.opList);
            break;
        case EXP_FUNC:
            result = evalExp(node->data.function.opList);
            break;
        case EXP2_FUNC:
            result = evalExp2(node->data.function.opList);
            break;
        case POW_FUNC:
            result = evalPow(node->data.function.opList);
            break;
        case LOG_FUNC:
            result = evalLog(node->data.function.opList);
            break;
        case SQRT_FUNC:
            result = evalSqrt(node->data.function.opList);
            break;
        case CBRT_FUNC:
            result = evalCbrt(node->data.function.opList);
            break;
        case HYPOT_FUNC:
            result = evalHypot(node->data.function.opList);
            break;
        case MAX_FUNC:
            result = evalMax(node->data.function.opList);
            break;
        case MIN_FUNC:
            result = evalMin(node->data.function.opList);
            break;
        case RAND_FUNC:
            result = evalRand();
            break;
        case READ_FUNC:
            result = evalRead();
            break;
        case EQUAL:
            result = evalEqual(node->data.function.opList);
            break;
        case LESS:
            result = evalLess(node->data.function.opList);
            break;
        case GREATER:
            result = evalGreater(node->data.function.opList);
            break;
        case PRINT:
            result = evalPrint(node->data.function.opList);
            break;
        case CUSTOM_FUNC:
            result = evalCustomFunction(node->data.function.opList);
        default:
            warning("WARNING: Function not recognized!");


    }

    return result;
}

RET_VAL evalNumNode(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into evalNumNode!");
        return NAN_RET_VAL;
    }

    // TODO complete the function finished

    return node->data.number;
}

RET_VAL evalSymbolNode(AST_NODE *symbol)
{
    AST_NODE *scope = symbol;
    if (!symbol)
    {
        yyerror("NULL ast node passed into evalSymbolNode!");
        return NAN_RET_VAL;
    }
    RET_VAL result;

    while(scope != NULL)
    {
        SYMBOL_TABLE_NODE *current = scope->symbolTable;
        while(current)
        {
            if(strcmp(current->id, symbol->data.symbol.id) == 0)
            {
                result = eval(current->value);
                if(current->value->type != NUM_NODE_TYPE)
                {
                    current->value->type = NUM_NODE_TYPE;
                    current->value->data.number = result;
                }

                if(result.type == DOUBLE_TYPE && current->type == INT_TYPE)
                {
                    warning("Precision loss on int cast from %lf to %d", result.value, (int)round(result.value));
                    result.type = INT_TYPE;
                    result.value = round(result.value);
                }
                else if(result.type == INT_TYPE && current->type == DOUBLE_TYPE)
                {
                    result.type = DOUBLE_TYPE;
                }
                /// free Node
                return result;
            }
            else
            {
                current = current->next;
            }
        }
        scope = scope->parent;
    }

    warning("Undefined Symbol \"%s\" evaluated! NAN returned!", symbol->data.symbol.id);
    return NAN_RET_VAL;


}

RET_VAL evalCondNode(AST_NODE *node)
{
    RET_VAL result;

    if(!node)
    {
        warning("NULL ast node passed into evalCondNode!");
        return NAN_RET_VAL;
    }
    else if(!node->data.conditional.condition || !node->data.conditional.ifFalse || !node->data.conditional.ifTrue)
    {
        warning("Not enough expressions in evalCondNode!");
        return NAN_RET_VAL;
    }
    result = eval(node->data.conditional.condition);
    if(result.value != 0)
        result.value = node->data.conditional.ifTrue->data.number.value;
    else
        result.value = node->data.conditional.ifFalse->data.number.value;

    result.type = INT_TYPE;
    return result;

}

RET_VAL eval(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into eval!");
        return NAN_RET_VAL;
    }

    // TODO complete the function finished
    switch (node->type) {
        case NUM_NODE_TYPE:
            return evalNumNode(node);
        case FUNC_NODE_TYPE:
            return evalFuncNode(node);
        case SCOPE_NODE_TYPE:
            return eval(node->data.scope.child);
        case SYM_NODE_TYPE:
            return evalSymbolNode(node);
        case COND_NODE_TYPE:
            return evalCondNode(node);
        default:
            yyerror("TYPE not recognized!");
            return NAN_RET_VAL;

    }


}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val)
{
    switch (val.type)
    {
        case INT_TYPE:
            printf("Integer : %.lf\n", val.value);
            break;
        case DOUBLE_TYPE:
            printf("Double : %lf\n", val.value);
            break;
        default:
            printf("No Type : %lf\n", val.value);
            break;
    }
}
// TODO NEEDS DEBUGGING
void freeFunctionNode(AST_NODE *function)
{
    freeNode(function->data.function.opList);
}

void freeSymbolTableNode(AST_NODE *symbol)
{
    freeNode(symbol->symbolTable->value);
}

// TODO - DEBUGGING
void freeNode(AST_NODE *node)
{
    if (!node)
    {
        return;
    }

    if(node->type == FUNC_NODE_TYPE){
        freeFunctionNode(node);
    }
//    if(node->type == SYM_NODE_TYPE){
//        freeSymbolTableNode(node);
//    }
    freeNode(node->next);
    free(node);



}


