all:
	@echo Building PiOS
	@$(MAKE) -C kernel --no-print-directory
	@$(MAKE) -C user/dummy1 --no-print-directory
	@$(MAKE) -C user/dummy2 --no-print-directory

.PHONY: clean

clean:
	@$(MAKE) -C kernel clean --no-print-directory
		