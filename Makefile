INC=-I /usr/local/include/GL
FRM=-l glut -l GLUI -framework OpenGL -framework GLUT


%.o : %.cpp
	g++ -g -w -c -o $@ $<

lab7 : entity.o main.o ply.o
	g++ -w -g  -Wno-deprecated-declarations  main.o entity.o ply.o $(INC) $(FRM) -o lab7
