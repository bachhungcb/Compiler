# Thêm các toán tử ** (lũy thừa) và % (chia lấy dư) vào trình biên dịch KPL

## Tóm tắt các thay đổi

Đã thêm hai toán tử mới vào trình biên dịch KPL:
- **`**`**: Toán tử lũy thừa (power/exponentiation)
- **`%`**: Toán tử chia lấy dư (modulo)

## Chi tiết các thay đổi

### 1. Charcode (charcode.h, charcode.c)
- Thêm `CHAR_PERCENT` vào enum `CharCode` để nhận diện ký tự `%`
- Cập nhật bảng mapping trong `charcode.c` để gán mã ASCII của `%` (37) tới `CHAR_PERCENT`

### 2. Token (token.h, token.c)
- Thêm hai token type mới:
  - `SB_POWER`: Đại diện cho toán tử `**`
  - `SB_PERCENT`: Đại diện cho toán tử `%`
- Cập nhật hàm `tokenToString()` để in đúng biểu diễn của các token mới

### 3. Scanner (scanner.c)
- Sửa lại xử lý `CHAR_TIMES`:
  - Kiểm tra nếu ký tự tiếp theo cũng là `*` thì trả về token `SB_POWER`
  - Nếu không thì trả về `SB_TIMES` (phép nhân bình thường)
- Thêm xử lý `CHAR_PERCENT`:
  - Trả về token `SB_PERCENT` khi gặp ký tự `%`
- Cập nhật hàm `printToken()` để xuất các token mới

### 4. Instructions (instructions.h, instructions.c)
- Thêm hai opcode mới trong enum `OpCode`:
  - `OP_MD`: Modulo (chia lấy dư)
  - `OP_POW`: Power (lũy thừa)
- Thêm hàm emit tương ứng:
  - `emitMD()`: Phát ra lệnh OP_MD
  - `emitPOW()`: Phát ra lệnh OP_POW
- Cập nhật hàm `printInstruction()` để in đúng tên lệnh

### 5. Code Generation (codegen.h, codegen.c)
- Thêm hai hàm tạo mã:
  - `genMD()`: Tạo mã cho phép modulo
  - `genPOW()`: Tạo mã cho phép lũy thừa

### 6. Parser (parser.h, parser.c)
- **Cấp độ ưu tiên (Precedence)**:
  - `**` (power): Cao nhất (cao hơn `*`, `/`, `%`)
  - `*`, `/`, `%`: Trung bình (thấp hơn `**`, cao hơn `+`, `-`)
  - `+`, `-`: Thấp nhất

- **Thay đổi cấu trúc phân tích**:
  - Thêm hàm `compileFactor2()`: Xử lý toán tử `**` với tính chất phải liên kết (right-associative)
  - Sửa `compileTerm2()`: Thêm xử lý cho toán tử `%`
  - Sửa `compileTerm()`: Gọi `compileFactor2()` sau `compileFactor()`
  - Sửa `compileTerm2()`: Gọi `compileFactor2()` trên operand thứ hai
  - Cập nhật tất cả FOLLOW set để bao gồm `SB_POWER` và `SB_PERCENT`

### 7. Virtual Machine Interpreter (vm.c, instructions.c)
- Thêm xử lý cho `OP_MD`:
  - Thực hiện phép chia lấy dư (modulo) trên hai số hàng đầu của stack
  - Kiểm tra lỗi chia cho 0
- Thêm xử lý cho `OP_POW`:
  - Tính lũy thừa của hai số
  - Hỗ trợ số mũ không âm
  - Kiểm tra lỗi với số mũ âm
- Cập nhật hàm `sprintInstruction()` trong `instructions.c` để in đúng tên lệnh

## Ví dụ sử dụng

### Toán tử **
```kpl
c := 2 ** 3;      (* c = 8 *)
d := 5 ** 2;      (* d = 25 *)
e := 10 ** 0;     (* e = 1 *)
```

### Toán tử %
```kpl
x := 10 % 3;      (* x = 1 *)
y := 20 % 7;      (* y = 6 *)
z := 15 % 5;      (* z = 0 *)
```

### Kết hợp cả hai
```kpl
result := 2 ** 3 % 5;  (* = 8 % 5 = 3 *)
a := (10 + 5) % 3;     (* = 15 % 3 = 0 *)
```

## File test
Tệp `test_operators.kpl` đã được tạo để kiểm tra các toán tử mới.

## Ghi chú
- Toán tử `**` có tính chất phải liên kết: `2 ** 3 ** 2 = 2 ** (3 ** 2) = 2 ** 9 = 512`
- Toán tử `%` có tính chất trái liên kết: `10 % 3 % 2 = (10 % 3) % 2 = 1 % 2 = 1`
- Cả hai toán tử chỉ hoạt động với kiểu INTEGER
- Chia cho 0 (hoặc modulo 0) sẽ tạo ra lỗi `PS_DIVIDE_BY_ZERO`
