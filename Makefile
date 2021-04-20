start: lang.o

lang.o: objectsFiles/main.o objectsFiles/Language.o objectsFiles/ASM.o definitions/proc.cpp
	g++ objectsFiles/main.o objectsFiles/Language.o objectsFiles/ASM.o definitions/proc.cpp -fsanitize=address -g -o language.out

objectsFiles/main.o: main.cpp
	g++ -c main.cpp -o objectsFiles/main.o -fsanitize=address -g -std=c++17

objectsFiles/proc.o: definitions/proc.cpp
	g++ -c definitions/proc.cpp -o objectsFiles/proc.o -fsanitize=address -g -std=c++17

objectsFiles/Language.o: definitions/Language.cpp
	g++ -c definitions/Language.cpp -o objectsFiles/Language.o -std=c++17 -fsanitize=address -g

objectsFiles/ASM.o: definitions/cleanAsm.cpp
	g++ -c definitions/cleanAsm.cpp -o objectsFiles/ASM.o -std=c++17 -fsanitize=address -g

clear: 
	rm objectsFiles/Language.o
	rm objectsFiles/ASM.o
	rm objectsFiles/main.o
