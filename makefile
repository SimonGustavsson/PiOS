all:
	@$(MAKE) -C kernel --no-print-directory
	@$(MAKE) -C user/dummy1 --no-print-directory
	@$(MAKE) -C user/dummy2 --no-print-directory

.PHONY: clean
	clean:
		@rm -rf $(BUILD_DIR)
		@rmdir $(BUILD_DIR)
		@cd kernel; $(MAKE) clean --no-print-directory
		