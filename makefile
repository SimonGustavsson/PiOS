all:
	@$(MAKE) -C kernel --no-print-directory
	@$(MAKE) -C user/dummy1 --no-print-directory
	@$(MAKE) -C user/dummy2 --no-print-directory

.PHONY: clean help fire

fire:
	@$(MAKE) -C kernel fire --no-print-directory

clean:
	@$(MAKE) -C kernel clean --no-print-directory
	@$(MAKE) -C user/dummy1 clean --no-print-directory
	@$(MAKE) -C user/dummy2 clean --no-print-directory

help:
	@$(MAKE) -C kernel help --no-print-directory


		