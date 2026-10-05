CC ?= i686-w64-mingw32-gcc
CFLAGS ?= -O2 -s -shared -Wall -Wextra -Iinclude
WINE ?= wine
TCC ?= tools/tcc/tcc.exe

TARGET = scripts/NFSMWSavetricks.asi
INSTALL_DIR ?= $(HOME)/Games/Nfsmw2005/scripts

.PHONY: all clean install

all: $(TARGET)

$(TARGET): src/main.c include/patch.h
	@chmod +x build.sh
	@./build.sh

clean:
	rm -f $(TARGET)

install: all
	@if [ -d "$(INSTALL_DIR)" ]; then \
		echo "Installing to $(INSTALL_DIR)..."; \
		cp -v scripts/NFSMWSavetricks.asi "$(INSTALL_DIR)/"; \
		if [ ! -f "$(INSTALL_DIR)/NFSMWSavetricks.ini" ]; then \
			cp -v scripts/NFSMWSavetricks.ini "$(INSTALL_DIR)/"; \
		else \
			echo "Keeping existing $(INSTALL_DIR)/NFSMWSavetricks.ini"; \
		fi; \
		echo "Installation finished successfully."; \
	else \
		echo "Error: Directory $(INSTALL_DIR) not found. Specify INSTALL_DIR=/path/to/game/scripts" >&2; \
		exit 1; \
	fi
