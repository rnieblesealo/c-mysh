mysh : mysh.c
	@gcc mysh.c -o mysh

leaks:
	@echo "Running valgrind..."
	@valgrind \
				--leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
				--trace-children=yes \
        ./mysh

clean:
	@echo "Cleaning up..."
	@rm mysh 2>/dev/null || true 
	@rm *.out 2>/dev/null || true 
