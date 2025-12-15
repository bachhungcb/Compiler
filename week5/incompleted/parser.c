/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type *intType;
extern Type *charType;
extern SymTab *symtab;

void scan(void)
{
  Token *tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType)
{
  if (lookAhead->tokenType == tokenType)
  {
    scan();
  }
  else
    missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void)
{
  Object *obj; // Khai bao doi tuong chuong trinh
  eat(KW_PROGRAM);
  eat(TK_IDENT);

  // [Semantic] Create program object and enter block
  obj = createProgramObject(currentToken->string);
  enterBlock(obj->progAttrs->scope);

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);

  //[Semantic] exit Block
  exitBlock();
}

void compileBlock(void)
{
  Object *obj;

  if (lookAhead->tokenType == KW_CONST)
  {
    eat(KW_CONST);

    do
    {
      eat(TK_IDENT);

      //[Semantic] Create Constant Object with name
      obj = createConstantObject(currentToken->string);

      eat(SB_EQ);

      // [Semantic] Assign registered value into symbol table
      obj->constAttrs->value = compileConstant();
      declareObject(obj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  }
  else
    compileBlock2();
}

void compileBlock2(void)
{
  Object *obj;
  if (lookAhead->tokenType == KW_TYPE)
  {
    eat(KW_TYPE);

    do
    {
      eat(TK_IDENT);
      //[Semantic] Create Type Object
      obj = createTypeObject(currentToken->string);

      eat(SB_EQ);
      //[Semantic] Assign Actualtype object
      obj->typeAttrs->actualType = compileType();

      //[Semantic] Declare Object
      declareObject(obj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  }
  else
    compileBlock3();
}

void compileBlock3(void)
{
  Object *obj;

  if (lookAhead->tokenType == KW_VAR)
  {
    eat(KW_VAR);

    do
    {
      eat(TK_IDENT);

      //[Semantic] Create variable Object
      obj = createVariableObject(currentToken->string);

      eat(SB_COLON);

      //[Semantic] Xac dinh kieu va register
      obj->varAttrs->type = compileType();
      declareObject(obj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock4();
  }
  else
    compileBlock4();
}

void compileBlock4(void)
{
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void)
{
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void)
{
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE))
  {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
  }
}
// Finished
void compileFuncDecl(void)
{
  Object *obj; // Function object

  eat(KW_FUNCTION);
  eat(TK_IDENT);

  // [Semantic] Create and register Function Object
  obj = createFunctionObject(currentToken->string);
  declareObject(obj);

  //[Semantic] Prepare for input block of function
  enterBlock(obj->funcAttrs->scope);

  compileParams();

  eat(SB_COLON);

  //[Semantic] Assign return value
  obj->funcAttrs->returnType = compileBasicType();

  eat(SB_SEMICOLON);
  compileBlock();

  //[Semantic] Exit function scope
  exitBlock();
  eat(SB_SEMICOLON);
}

void compileProcDecl(void)
{
  Object *obj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);

  //[Semantic] Create, register and input procedure block
  obj = createProcedureObject(currentToken->string);
  declareObject(obj);
  enterBlock(obj->procAttrs->scope);

  compileParams();

  eat(SB_SEMICOLON);
  compileBlock();

  //[Semantic] Exit block
  exitBlock();

  eat(SB_SEMICOLON);
}

ConstantValue *compileUnsignedConstant(void)
{
  ConstantValue *constValue = NULL;
  Object *obj = NULL;

  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(atoi(currentToken->string));
    break;

  case TK_IDENT:
    eat(TK_IDENT);
    // [Semantic] Sửa lỗi implicit declaration tại đây
    obj = lookupObject(currentToken->string);

    if (obj != NULL && obj->kind == OBJ_CONSTANT)
    {
      constValue = duplicateConstantValue(obj->constAttrs->value);
    }
    else
    {
      // Báo lỗi nếu dùng định danh không hợp lệ làm hằng số
      error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;

  case TK_CHAR:
    eat(TK_CHAR);
    // Lấy ký tự đầu tiên của chuỗi token
    constValue = makeCharConstant(currentToken->string[0]);
    break;

  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue *compileConstant(void)
{
  ConstantValue *constValue;

  switch (lookAhead->tokenType)
  {
  case SB_PLUS:
    eat(SB_PLUS);
    // [Semantic] get positive constant value
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    // [Semantic] Get value and Integer Negation
    constValue = compileConstant2();
    if (constValue->type == TP_INT)
    {
      constValue->intValue = -constValue->intValue;
    }
    else
    {
      // [Security] Type Mismatch Check
      error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    //[Semantic] Create constant char
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue *compileConstant2(void)
{
  ConstantValue *constValue = NULL;
  Object *obj = NULL;

  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    // [Semantic] Tạo hằng số nguyên
    constValue = makeIntConstant(atoi(currentToken->string));
    break;

  case TK_IDENT:
    eat(TK_IDENT);
    // [Semantic] Tra cứu định danh trong bảng ký hiệu
    // Thay thế checkDeclaredConstant bằng lookupObject
    obj = lookupObject(currentToken->string);

    // [Semantic] Kiểm tra:
    // 1. Đối tượng phải tồn tại (obj != NULL)
    // 2. Đối tượng phải là hằng số (obj->kind == OBJ_CONSTANT)
    if (obj != NULL && obj->kind == OBJ_CONSTANT)
    {
      // Sao chép giá trị để đảm bảo an toàn
      constValue = duplicateConstantValue(obj->constAttrs->value);
    }
    else
    {
      // Nếu định danh không tìm thấy hoặc không phải là hằng số -> Báo lỗi
      // Trong an toàn phần mềm, đây là bước ngăn chặn sử dụng biến chưa khởi tạo hoặc sai loại
      error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;

  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}
Type *compileType(void)
{
  Type *type;
  Type *elementType;
  int arraySize;
  Object *obj;

  switch (lookAhead->tokenType)
  {
  case KW_INTEGER:
    eat(KW_INTEGER);
    // [Semantic] Tạo kiểu số nguyên
    type = makeIntType();
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    // [Semantic] Tạo kiểu ký tự
    type = makeCharType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);

    // [Semantic] Lấy kích thước mảng
    // Cần đảm bảo kích thước > 0 để tránh cấp phát 0 byte (Zero-length allocation)
    arraySize = atoi(currentToken->string);

    eat(SB_RSEL);
    eat(KW_OF);

    // [Semantic] Đệ quy để lấy kiểu phần tử (Element Type)
    elementType = compileType();

    // [Semantic] Tạo kiểu mảng
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // [Semantic] Xử lý kiểu người dùng tự định nghĩa (Type Aliasing)
    // Tra cứu xem định danh này có phải là một kiểu (TYPE) đã khai báo không
    obj = lookupObject(currentToken->string);

    // Sao chép kiểu thực tế (Actual Type)
    type = duplicateType(obj->typeAttrs->actualType);
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type *compileBasicType(void)
{
  Type *type;

  switch (lookAhead->tokenType)
  {
  case KW_INTEGER:
    eat(KW_INTEGER);
    //[Semantic]
    type = makeIntType();
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    //[Semantic]
    type = makeCharType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void)
{
  if (lookAhead->tokenType == SB_LPAR)
  {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON)
    {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void)
{
  Object *obj; // Param Object
  Type *type;  // Data Type
  switch (lookAhead->tokenType)
  {
  case TK_IDENT: // PARAM_VALUE
    eat(TK_IDENT);

    //[Semantic] Create Param value
    obj = createParameterObject(currentToken->string, PARAM_VALUE, symtab->currentScope->owner);

    eat(SB_COLON);
    type = compileBasicType();
    obj->paramAttrs->type = type;

    // [Semantic] Register param into table
    declareObject(obj);
    break;

  case KW_VAR: // PARAM_REFERENCE
    eat(KW_VAR);
    eat(TK_IDENT);

    // [Semantic] Tạo tham số tham biến
    obj = createParameterObject(currentToken->string, PARAM_REFERENCE, symtab->currentScope->owner);

    eat(SB_COLON);
    type = compileBasicType();
    obj->paramAttrs->type = type;

    //[Semantic] register param
    declareObject(obj);
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileStatements(void)
{
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON)
  {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileLValue(void)
{
  eat(TK_IDENT);
  compileIndexes();
}

void compileAssignSt(void)
{
  compileLValue();
  eat(SB_ASSIGN);
  compileExpression();
}

void compileCallSt(void)
{
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
}

void compileGroupSt(void)
{
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void)
{
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE)
    compileElseSt();
}

void compileElseSt(void)
{
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void)
{
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void)
{
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
}

void compileArgument(void)
{
  compileExpression();
}

void compileArguments(void)
{
  switch (lookAhead->tokenType)
  {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument();

    while (lookAhead->tokenType == SB_COMMA)
    {
      eat(SB_COMMA);
      compileArgument();
    }

    eat(SB_RPAR);
    break;
    // Check FOLLOW set
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void)
{
  compileExpression();
  switch (lookAhead->tokenType)
  {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  compileExpression();
}

void compileExpression(void)
{
  switch (lookAhead->tokenType)
  {
  case SB_PLUS:
    eat(SB_PLUS);
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileExpression2();
    break;
  default:
    compileExpression2();
  }
}

void compileExpression2(void)
{
  compileTerm();
  compileExpression3();
}

void compileExpression3(void)
{
  switch (lookAhead->tokenType)
  {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression3();
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileTerm(void)
{
  compileFactor();
  compileTerm2();
}

void compileTerm2(void)
{
  switch (lookAhead->tokenType)
  {
  case SB_TIMES:
    eat(SB_TIMES);
    compileFactor();
    compileTerm2();
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    compileFactor();
    compileTerm2();
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileFactor(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    switch (lookAhead->tokenType)
    {
    case SB_LPAR:
      compileArguments();
      break;
    case SB_LSEL:
      compileIndexes();
      break;
    default:
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileIndexes(void)
{
  while (lookAhead->tokenType == SB_LSEL)
  {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName)
{
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program, 0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;
}
