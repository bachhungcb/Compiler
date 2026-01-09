# Định nghĩa các thư mục con (Sub-directories)
# Đây là các module thành phần trong kiến trúc của bạn
COMPILER_DIR = completed
INTERPRETER_DIR = interpreter

# .PHONY đánh dấu các target này là giả (không phải tên file)
# Điều này tránh xung đột nếu vô tình có file trùng tên với thư mục
.PHONY: all clean $(COMPILER_DIR) $(INTERPRETER_DIR)

# Rule mặc định: Build cả Compiler và Interpreter
all: $(COMPILER_DIR) $(INTERPRETER_DIR)
	@echo "Build process completed successfully for both binaries."

# Rule để build Compiler (KPLC)
# Cờ -C chỉ thị cho make thay đổi context vào thư mục con trước khi thực thi
$(COMPILER_DIR):
	@echo "=== Building Compiler (KPLC) ==="
	$(MAKE) -C $(COMPILER_DIR)

# Rule để build Interpreter (KPLRUN)
$(INTERPRETER_DIR):
	@echo "=== Building Interpreter (VM) ==="
	$(MAKE) -C $(INTERPRETER_DIR)

BIN_FILE = $(FILE:.kpl=.bin)

run: $(COMPILER_DIR) $(INTERPRETER_DIR)
	@if [ -z "$(FILE)" ]; then \
		echo "❌ [ERROR] Missing input file. Usage: make run FILE=path/to/code.kpl"; \
		exit 1; \
	fi
	@echo "🛡️ [SEC] Starting Compilation Phase..."
	@# Chạy Compiler: Đầu vào là $(FILE), đầu ra tự động là $(BIN_FILE)
	./$(COMPILER_DIR)/kplc $(FILE) $(BIN_FILE) -dump
	
	@echo "🚀 [EXEC] Starting Execution Phase..."
	@# Chạy Interpreter với file binary vừa tạo
	./$(INTERPRETER_DIR)/kplrun $(BIN_FILE)
	
	@echo "🧹 [CLEANUP] Removing artifact..."
	rm -f $(BIN_FILE)

# Rule để dọn dẹp (Clean) các file object và binary ở cả 2 thư mục
clean:
	@echo "=== Cleaning up build artifacts ==="
	$(MAKE) -C $(COMPILER_DIR) clean
	$(MAKE) -C $(INTERPRETER_DIR) clean