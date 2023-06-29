CC = gcc   #compiler

sourceFiles=$(addprefix ./src/, pfmain.c pfgame.c pfai.c) #all source files
includeDirs=-I./src/include# -I./Allegro/allegro-5.2.8.0/build/include
libraryDirs=#-L./Allegro/allegro-5.2.8.0/build/lib
libraryFiles=-lpthread# -lallegro -lallegro_main -lallegro_primitives



all: $(sourceFiles) # $(includeFiles)
	gcc -v -g -o ./main.out $(sourceFiles) $(includeDirs) $(libraryDirs) $(libraryFiles)



allegro: 
	echo "making allegro"


       
