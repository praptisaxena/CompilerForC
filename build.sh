# #!/bin/bash
# gcc lexer.c -o main -Wall -Wextra
#!/bin/bash
gcc main.c lexer.c parser.c semantic.c codegen.c -o main -Wall -Wextra
