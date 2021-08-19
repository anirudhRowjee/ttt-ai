main:
	clang -o ttt main.c pvp.c pvc.c -O3

# Address Sanitizer to check for Memory Leaks, because Valgrind
# Damn near nuked my PC xD
asan:
	clang -fsanitize=address -O1 -fno-omit-frame-pointer -g -o ttt main.c pvp.c pvc.c

