# -------------------------------------------------------------------
# MAKEFILE CHO BAI THUC HANH SO 1 - INDEXER
# -------------------------------------------------------------------

# 1. Cấu hình trình biên dịch
CC = gcc

# 2. Các cờ (Flags) biên dịch
# -Wall     : Bật tất cả các cảnh báo (Warning) - Rất quan trọng để check lỗi logic.
# -Wextra   : Bật thêm các cảnh báo nâng cao.
# -g        : Thêm thông tin Debug (để bạn có thể dùng gdb sửa lỗi).
# -std=c99  : Sử dụng chuẩn C99 (cho phép khai báo biến trong vòng for).
CFLAGS = -Wall -Wextra -g -std=c99

# Tùy chọn bảo mật (Optional): Bật AddressSanitizer để phát hiện tràn bộ đệm/rò rỉ bộ nhớ ngay khi chạy
# Bỏ comment dòng dưới nếu bạn muốn kiểm tra lỗi bộ nhớ (cần cài libasan)
# CFLAGS += -fsanitize=address

# 3. Tên file thực thi đầu ra (Output executable)
TARGET = indexer

# 4. Tên file mã nguồn (Source code)
# Thay 'main.c' bằng tên file code thực tế của bạn (ví dụ: bai1.c)
SRC = main.c

# -------------------------------------------------------------------
# CÁC RULES (QUY TẮC)
# -------------------------------------------------------------------

# Rule mặc định (khi bạn chỉ gõ 'make')
all: $(TARGET)

# Quy tắc build: Tạo file thực thi từ mã nguồn
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Rule chạy chương trình (tiện lợi: gõ 'make run')
run: $(TARGET)
	@echo "--- Dang chay chuong trinh ---"
	./$(TARGET)

# Rule dọn dẹp: Xóa file thực thi để build lại từ đầu
clean:
	rm -f $(TARGET)