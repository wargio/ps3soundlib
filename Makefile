all:
	@$(MAKE) install -C libs --no-print-directory
#	@$(MAKE) pkg -C samples/fireworks3D/ --no-print-directory

clean:
	@$(MAKE) clean -C libs --no-print-directory
#	@$(MAKE) clean -C samples/fireworks3D/ --no-print-directory

