Include = ~/LProjects/pdfium_projects/pdfium/include
LibInclude = ~/LProjects/pdfium_projects/pdfium/lib

main: main.o
	g++ -o main main.o -L ${LibInclude} -lpdfium
main.o: main.cpp
	g++ -c main.cpp -I ${Include}
clean:
	rm main main.o output.pdf