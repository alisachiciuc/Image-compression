build: quadtree.c
	        gcc -Wall quadtree.c -o quadtree -lm

run: build
	        ./quadtree 

clean: 
	        rm quadtree 

