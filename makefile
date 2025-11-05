# project
EXE=project
OBJDIR=build

# Main target
all: $(OBJDIR) $(EXE)

# Create build directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW
LIBS=-lfreeglut -lglew32 -lglu32 -lopengl32 -lm
CLEAN=rm -f *.exe *.o *.a && rm -rf $(OBJDIR)
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations
LIBS=-framework GLUT -framework OpenGL
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall
LIBS=-lglut -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.a && rm -rf $(OBJDIR)
endif

# Compile rules
.c.o:
	gcc -c $(CFLG)  $< -o $(OBJDIR)/$@
.cpp.o:
	g++ -c $(CFLG)  $< -o $(OBJDIR)/$@

#  Link
project: $(OBJDIR)/main.o $(OBJDIR)/bullseye.o $(OBJDIR)/ground.o $(OBJDIR)/lighting.o $(OBJDIR)/axes.o $(OBJDIR)/tree.o $(OBJDIR)/view.o $(OBJDIR)/utils.o
	gcc $(CFLG) -o $@ $^  $(LIBS)

# Compile objects directory
$(OBJDIR)/bullseye.o: objects/bullseye.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/ground.o: objects/ground.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/lighting.o: objects/lighting.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/axes.o: objects/axes.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/tree.o: objects/tree.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/view.o: view.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/utils.o: utils.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

$(OBJDIR)/main.o: main.c | $(OBJDIR)
	gcc -c $(CFLG) -o $@ $<

#  Clean
clean:
	$(CLEAN)
