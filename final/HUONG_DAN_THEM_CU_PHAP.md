# Hướng Dẫn Thêm Cú Pháp Mới Vào Trình Biên Dịch KPL

Tài liệu này hướng dẫn chi tiết các bước để thêm một cú pháp/toán tử mới vào trình biên dịch KPL.

## 📋 Tổng Quan Quy Trình

Khi thêm một cú pháp mới, bạn cần cập nhật các thành phần theo thứ tự:

1. **Character Recognition** (charcode) - Nhận diện ký tự
2. **Token Definition** (token) - Định nghĩa token
3. **Lexical Analysis** (scanner) - Phân tích từ vựng
4. **Instruction Set** (instructions) - Tập lệnh máy ảo
5. **Code Generation** (codegen) - Tạo mã
6. **Syntax Analysis** (parser) - Phân tích cú pháp
7. **Virtual Machine** (interpreter) - Máy ảo thực thi

---

## 🔧 Chi Tiết Từng Bước

### Bước 1: Nhận Diện Ký Tự (Character Code)

**Mục đích:** Cho phép scanner nhận biết ký tự mới trong mã nguồn.

#### 1.1. Cập nhật `charcode.h`

Thêm định nghĩa ký tự vào enum `CharCode`:

```c
typedef enum {
  CHAR_SPACE,
  CHAR_LETTER,
  CHAR_DIGIT,
  CHAR_PLUS,
  CHAR_MINUS,
  // ... các ký tự khác
  CHAR_YOUR_NEW_CHAR,  // ← Thêm ký tự mới của bạn
  CHAR_UNKNOWN
} CharCode;
```

**Lưu ý:** Thêm trước `CHAR_UNKNOWN` để duy trì cấu trúc enum.

#### 1.2. Cập nhật `charcode.c`

Map mã ASCII của ký tự với định nghĩa CharCode:

```c
CharCode charCodes[256] = {
  // ... mapping các ký tự
  // Ví dụ: Nếu ký tự là '%' (ASCII 37)
  CHAR_SPACE, CHAR_EXCLAIMATION, CHAR_UNKNOWN, CHAR_UNKNOWN, 
  CHAR_UNKNOWN, CHAR_YOUR_NEW_CHAR, CHAR_UNKNOWN, CHAR_SINGLEQUOTE,
  // ...
};
```

**Cách xác định vị trí:**
- Tìm mã ASCII của ký tự (ví dụ: '%' = 37)
- Thay thế `CHAR_UNKNOWN` tại vị trí 37 thành `CHAR_YOUR_NEW_CHAR`

---

### Bước 2: Định Nghĩa Token

**Mục đích:** Tạo đại diện trừu tượng cho ký tự/cụm ký tự trong quá trình phân tích.

#### 2.1. Cập nhật `token.h`

Thêm token type vào enum `TokenType`:

```c
typedef enum {
  TK_NONE, TK_IDENT, TK_NUMBER, TK_CHAR, TK_EOF,
  
  // Keywords
  KW_PROGRAM, KW_CONST, KW_TYPE, KW_VAR,
  // ...
  
  // Symbols
  SB_SEMICOLON, SB_COLON, SB_PERIOD, SB_COMMA,
  SB_ASSIGN, SB_EQ, SB_NEQ,
  SB_PLUS, SB_MINUS, SB_TIMES, SB_SLASH,
  SB_YOUR_NEW_TOKEN,  // ← Thêm token mới
  SB_LPAR, SB_RPAR, SB_LSEL, SB_RSEL
} TokenType;
```

**Quy tắc đặt tên:**
- `KW_` cho từ khóa (keyword)
- `SB_` cho ký hiệu (symbol)
- `TK_` cho token đặc biệt

#### 2.2. Cập nhật `token.c`

Thêm biểu diễn chuỗi cho token trong hàm `tokenToString()`:

```c
char *tokenToString(TokenType tokenType) {
  switch (tokenType) {
    // ... các case khác
    case SB_PLUS: return "\'+\'";
    case SB_MINUS: return "\'-\'";
    case SB_YOUR_NEW_TOKEN: return "\'your_symbol\'";  // ← Thêm case mới
    case SB_LPAR: return "\'(\'";
    // ...
    default: return "";
  }
}
```

---

### Bước 3: Phân Tích Từ Vựng (Scanner)

**Mục đích:** Chuyển đổi ký tự thành token trong quá trình đọc mã nguồn.

#### 3.1. Cập nhật `scanner.c` - Hàm `getToken()`

Thêm xử lý nhận diện ký tự mới:

```c
Token *getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF)
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
    // ... các case khác
    
    case CHAR_YOUR_NEW_CHAR:
      token = makeToken(SB_YOUR_NEW_TOKEN, lineNo, colNo);
      readChar();
      return token;
    
    // Hoặc nếu là toán tử kép (ví dụ: **)
    case CHAR_TIMES:
      ln = lineNo;
      cn = colNo;
      readChar();
      if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_TIMES)) {
        readChar();
        return makeToken(SB_POWER, ln, cn);  // **
      }
      else
        return makeToken(SB_TIMES, ln, cn);  // *
    
    // ...
  }
}
```

**Lưu ý:**
- Dùng `readChar()` để đọc ký tự tiếp theo
- Với toán tử kép, cần kiểm tra ký tự thứ hai

#### 3.2. Cập nhật `scanner.c` - Hàm `printToken()`

Thêm case in token mới:

```c
void printToken(Token *token) {
  printf("%d-%d:", token->lineNo, token->colNo);
  
  switch (token->tokenType) {
    // ... các case khác
    case SB_YOUR_NEW_TOKEN:
      printf("SB_YOUR_NEW_TOKEN\n");
      break;
    // ...
  }
}
```

---

### Bước 4: Định Nghĩa Tập Lệnh (Instructions)

**Mục đích:** Tạo opcode cho máy ảo để thực thi phép toán/thao tác mới.

#### 4.1. Cập nhật `completed/instructions.h`

Thêm opcode vào enum:

```c
enum OpCode {
  OP_LA, OP_LV, OP_LC, OP_LI,
  // ... các opcode khác
  OP_AD,   // Add
  OP_SB,   // Subtract
  OP_ML,   // Multiply
  OP_DV,   // Divide
  OP_YOUR_NEW_OP,  // ← Thêm opcode mới với comment mô tả
  OP_NEG,
  // ...
};
```

Thêm prototype hàm emit:

```c
int emitAD(CodeBlock* codeBlock);
int emitSB(CodeBlock* codeBlock);
int emitYourNewOp(CodeBlock* codeBlock);  // ← Thêm prototype
int emitNEG(CodeBlock* codeBlock);
```

#### 4.2. Cập nhật `completed/instructions.c`

Implement hàm emit:

```c
int emitAD(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_AD, DC_VALUE, DC_VALUE); }
int emitSB(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_SB, DC_VALUE, DC_VALUE); }
int emitYourNewOp(CodeBlock* codeBlock) { 
  return emitCode(codeBlock, OP_YOUR_NEW_OP, DC_VALUE, DC_VALUE); 
}
```

Thêm vào hàm `printInstruction()`:

```c
void printInstruction(Instruction* inst) {
  switch (inst->op) {
    // ... các case khác
    case OP_AD: printf("AD"); break;
    case OP_YOUR_NEW_OP: printf("YOUR_NEW_OP"); break;  // ← Thêm
    case OP_NEG: printf("NEG"); break;
    // ...
  }
}
```

#### 4.3. Cập nhật `interpreter/instructions.h` và `interpreter/instructions.c`

**QUAN TRỌNG:** Lặp lại tất cả các bước 4.1 và 4.2 cho thư mục `interpreter/`.

---

### Bước 5: Tạo Mã (Code Generation)

**Mục đích:** Cung cấp hàm để parser gọi khi sinh mã cho cú pháp mới.

#### 5.1. Cập nhật `codegen.h`

Thêm prototype:

```c
void genAD(void);
void genSB(void);
void genYourNewOp(void);  // ← Thêm prototype
void genNEG(void);
```

#### 5.2. Cập nhật `codegen.c`

Implement hàm:

```c
void genAD(void) {
  emitAD(codeBlock);
}

void genSB(void) {
  emitSB(codeBlock);
}

void genYourNewOp(void) {
  emitYourNewOp(codeBlock);  // ← Implement mới
}

void genNEG(void) {
  emitNEG(codeBlock);
}
```

---

### Bước 6: Phân Tích Cú Pháp (Parser)

**Mục đích:** Định nghĩa ngữ pháp và thứ tự ưu tiên của cú pháp mới.

#### 6.1. Hiểu Cấu Trúc Biểu Thức

Thứ tự ưu tiên (từ thấp đến cao):
1. **Expression**: `+`, `-` (cộng, trừ)
2. **Term**: `*`, `/`, `%` (nhân, chia, modulo)
3. **Factor**: `**` (lũy thừa), literal, biến, `(expr)`

Cấu trúc parsing:
```
compileExpression()
  └─> compileExpression2()
       └─> compileTerm()
            ├─> compileFactor()
            │    └─> compileFactor2()  // Xử lý **
            └─> compileTerm2()         // Xử lý *, /, %
       └─> compileExpression3()        // Xử lý +, -
```

#### 6.2. Quyết Định Vị Trí

**Nếu toán tử có mức ưu tiên giống `*`, `/`, `%`:**
- Thêm vào `compileTerm2()`

**Nếu toán tử có mức ưu tiên cao hơn `*`, `/`:**
- Thêm vào `compileFactor2()` hoặc tạo hàm mới

**Nếu toán tử có mức ưu tiên thấp hơn `+`, `-`:**
- Thêm vào `compileExpression3()` hoặc tạo hàm mới

#### 6.3. Ví Dụ: Thêm Toán Tử Modulo `%` (cùng cấp với `*`, `/`)

Cập nhật `compileTerm2()` trong `parser.c`:

```c
Type *compileTerm2(Type *argType1) {
  Type *argType2;
  Type *resultType;

  switch (lookAhead->tokenType) {
    case SB_TIMES:
      eat(SB_TIMES);
      checkIntType(argType1);
      argType2 = compileFactor();
      argType2 = compileFactor2(argType2);
      checkIntType(argType2);
      genML();
      resultType = compileTerm2(argType1);
      break;
    
    case SB_SLASH:
      eat(SB_SLASH);
      checkIntType(argType1);
      argType2 = compileFactor();
      argType2 = compileFactor2(argType2);
      checkIntType(argType2);
      genDV();
      resultType = compileTerm2(argType1);
      break;
    
    // ← Thêm case mới cho toán tử của bạn
    case SB_YOUR_NEW_TOKEN:
      eat(SB_YOUR_NEW_TOKEN);
      checkIntType(argType1);          // Kiểm tra kiểu
      argType2 = compileFactor();      // Parse toán hạng thứ 2
      argType2 = compileFactor2(argType2);
      checkIntType(argType2);
      genYourNewOp();                  // Sinh mã
      resultType = compileTerm2(argType1);  // Đệ quy cho tính kết hợp trái
      break;
    
    // FOLLOW set
    case SB_PLUS:
    case SB_MINUS:
    // ... các token khác
      resultType = argType1;
      break;
    
    default:
      error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
  return resultType;
}
```

#### 6.4. Cập Nhật FOLLOW Set

Thêm token mới vào tất cả FOLLOW set liên quan:

```c
// Trong compileExpression3()
case SB_TIMES:
case SB_SLASH:
case SB_YOUR_NEW_TOKEN:  // ← Thêm vào FOLLOW
case KW_TO:
case KW_DO:
// ...
```

```c
// Trong compileArguments()
case SB_TIMES:
case SB_SLASH:
case SB_YOUR_NEW_TOKEN:  // ← Thêm vào FOLLOW
case SB_PLUS:
// ...
```

#### 6.5. Thêm Prototype vào `parser.h` (nếu cần hàm mới)

```c
Type* compileExpression(void);
Type* compileExpression2(void);
Type* compileExpression3(Type* argType1);
Type* compileTerm(void);
Type* compileTerm2(Type* argType2);
Type* compileFactor(void);
Type* compileFactor2(Type* argType1);  // ← Nếu thêm hàm mới
Type* compileIndexes(Type* arrayType);
```

---

### Bước 7: Máy Ảo Thực Thi (Virtual Machine)

**Mục đích:** Implement logic thực thi phép toán/thao tác trên máy ảo.

#### 7.1. Cập nhật `interpreter/vm.c`

Thêm case xử lý opcode mới trong hàm `run()`:

```c
int run(void) {
  Instruction* code = codeBlock->code;
  // ...
  
  while (ps == PS_ACTIVE) {
    switch (code[pc].op) {
      // ... các case khác
      
      case OP_AD:
        t --;
        if (checkStack()) 
          stack[t] += stack[t+1];
        break;
      
      case OP_SB:
        t --;
        if (checkStack()) 
          stack[t] -= stack[t+1];
        break;
      
      // ← Thêm case mới
      case OP_YOUR_NEW_OP:
        t --;
        if (checkStack()) {
          // Implement logic thực thi
          // Ví dụ: Modulo
          if (stack[t+1] == 0)
            ps = PS_DIVIDE_BY_ZERO;  // Xử lý lỗi nếu cần
          else
            stack[t] %= stack[t+1];
        }
        break;
      
      // ← Ví dụ: Power (lũy thừa)
      case OP_POW:
        t --;
        if (checkStack()) {
          int base_val = stack[t];
          int exp_val = stack[t+1];
          int result = 1;
          int i;
          
          if (exp_val < 0) {
            ps = PS_DIVIDE_BY_ZERO;  // Lỗi số mũ âm
          } else {
            for (i = 0; i < exp_val; i++) {
              result *= base_val;
            }
            stack[t] = result;
          }
        }
        break;
      
      // ...
    }
    pc ++;
  }
  return ps;
}
```

**Lưu ý về Stack:**
- `stack[t]`: Toán hạng thứ nhất (bên trái)
- `stack[t+1]`: Toán hạng thứ hai (bên phải)
- Kết quả lưu vào `stack[t]`
- Giảm `t--` vì đã "tiêu thụ" 2 toán hạng và tạo 1 kết quả

---

## 📝 Checklist Hoàn Chỉnh

Khi thêm cú pháp mới, đảm bảo đã hoàn thành:

### Compiler (completed/)
- [ ] `charcode.h` - Thêm CHAR_*
- [ ] `charcode.c` - Map mã ASCII
- [ ] `token.h` - Thêm SB_* hoặc KW_*
- [ ] `token.c` - Thêm tokenToString()
- [ ] `scanner.c` - Thêm getToken() case
- [ ] `scanner.c` - Thêm printToken() case
- [ ] `instructions.h` - Thêm OP_* và prototype
- [ ] `instructions.c` - Implement emit* và printInstruction()
- [ ] `codegen.h` - Thêm gen* prototype
- [ ] `codegen.c` - Implement gen*()
- [ ] `parser.h` - Thêm compile* prototype (nếu cần)
- [ ] `parser.c` - Thêm xử lý parsing
- [ ] `parser.c` - Cập nhật FOLLOW sets

### Interpreter (interpreter/)
- [ ] `instructions.h` - Thêm OP_* và prototype
- [ ] `instructions.c` - Implement emit*, printInstruction(), sprintInstruction()
- [ ] `vm.c` - Thêm case xử lý opcode trong run()

### Testing
- [ ] Viết file test `.kpl` để kiểm tra
- [ ] Compile với `make` ở cả 2 thư mục
- [ ] Chạy compiler: `./kplc test.kpl output`
- [ ] Chạy interpreter: `../interpreter/kplrun output`

---

## 🧪 Quy Trình Testing

### 1. Tạo File Test

Ví dụ `test_newop.kpl`:

```pascal
PROGRAM TestNewOp;
VAR a, b, c: INTEGER;
BEGIN
  a := 10;
  b := 3;
  c := a YOUR_OP b;  (* Sử dụng toán tử mới *)
  CALL WRITEI(c);
END.
```

### 2. Compile

```bash
cd completed/
make clean
make
./kplc test_newop.kpl output
```

### 3. Run

```bash
cd ../interpreter/
make clean
make
./kplrun output
```

### 4. Debug

Nếu gặp lỗi:
1. Kiểm tra messages trong quá trình compile
2. Xem `result2.txt` để debug parser
3. Dùng `gdb` để debug C code nếu cần

---

## 💡 Mẹo & Lưu Ý

### Quy Tắc Đặt Tên
- **Opcode:** `OP_XXX` (viết hoa, ngắn gọn)
- **Token:** `SB_XXX` cho symbol, `KW_XXX` cho keyword
- **Char:** `CHAR_XXX` (mô tả rõ ràng)

### Thứ Tự Ưu Tiên
```
Cao nhất:  ()  []  .
           **                (lũy thừa)
           * / %             (nhân, chia, modulo)
           + -               (cộng, trừ)
           < <= > >= = !=    (so sánh)
Thấp nhất: AND OR NOT        (logic)
```

### Tính Kết Hợp (Associativity)
- **Trái → Phải:** `+`, `-`, `*`, `/`, `%` → Dùng đệ quy ở cuối hàm
- **Phải → Trái:** `**`, `=` → Đệ quy trước khi tính toán

### Kiểm Tra Kiểu Dữ Liệu
```c
checkIntType(type);        // Chỉ chấp nhận INTEGER
checkCharType(type);       // Chỉ chấp nhận CHAR
checkBasicType(type);      // INTEGER hoặc CHAR
checkTypeEquality(t1, t2); // Cùng kiểu
```

### Xử Lý Lỗi Runtime
```c
if (điều_kiện_lỗi)
  ps = PS_DIVIDE_BY_ZERO;  // Hoặc mã lỗi khác
```

Các mã lỗi có sẵn:
- `PS_NORMAL_EXIT`
- `PS_IO_ERROR`
- `PS_DIVIDE_BY_ZERO`
- `PS_STACK_OVERFLOW`

---

## 📚 Tài Liệu Tham Khảo

### File Quan Trọng
- `completed/parser.c`: Logic phân tích cú pháp
- `completed/semantics.c`: Kiểm tra ngữ nghĩa
- `interpreter/vm.c`: Thực thi máy ảo
- `OPERATORS_CHANGES.md`: Ví dụ thực tế (toán tử `**` và `%`)

### Thuật Ngữ
- **Token:** Đơn vị từ vựng cơ bản
- **Opcode:** Mã lệnh máy ảo
- **FOLLOW Set:** Tập token có thể xuất hiện sau một cấu trúc
- **Precedence:** Thứ tự ưu tiên của toán tử
- **Associativity:** Tính kết hợp (trái/phải)

---

## ✅ Ví Dụ Hoàn Chỉnh

Xem file `OPERATORS_CHANGES.md` để thấy ví dụ chi tiết về việc thêm toán tử `**` (lũy thừa) và `%` (modulo).

---

**Tác giả:** Documentation for KPL Compiler  
**Ngày cập nhật:** January 11, 2026  
**Phiên bản:** 1.0
