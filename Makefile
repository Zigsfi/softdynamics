
all:
	clang++ -w -g -Wall -Wextra *.cpp -L ~/Downloads/COMP175/glui-2.36/src/lib -I ~/Downloads/COMP175/glui-2.36/src/include -framework OpenGL -lglui -framework GLUT -o a5
