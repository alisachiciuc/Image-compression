#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct matrix {
	unsigned char red, green, blue;  
} MATRIX;

typedef struct tree {
	struct tree* top_left;
	struct tree* top_right;
	struct tree* bottom_left;
	struct tree* bottom_right;
	unsigned char red, green, blue;
	uint32_t area;
	int32_t t_l,t_r,b_l,b_r;
} *TREE;

typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;


TREE initTree(unsigned char red, unsigned char green, unsigned char blue, uint32_t area) {
	TREE tree = (TREE)malloc(sizeof(struct tree));
	tree->area = area;
	tree->red = red;
	tree->green = green;
	tree->blue = blue;
	tree->t_l = -1;
	tree->t_r = -1;
	tree->b_l = -1;
	tree->b_r = -1;
	tree->top_left = NULL;
	tree->top_right = NULL;
	tree->bottom_left = NULL;
	tree->bottom_right = NULL;
	return tree;
}

void delete(TREE tree) {
	if(tree != NULL) {
		delete(tree->top_left);
		delete(tree->top_right);
		delete(tree->bottom_left);
		delete(tree->bottom_right);
		free(tree);
	}
}

void writeFile(char* file,int size, MATRIX** matrix) {
	FILE* output;
	int i, j;
	output = fopen(file, "wb");
	fprintf(output, "P6\n");
	fprintf(output, "%d %d\n", size, size);
	fprintf(output, "255\n");
	for(i = 0; i < size ; i++) 
		for( j = 0; j < size ; j++ ) {
			fwrite(&matrix[i][j].red, sizeof(unsigned char), 1, output);
			fwrite(&matrix[i][j].green, sizeof(unsigned char), 1, output);
			fwrite(&matrix[i][j].blue, sizeof(unsigned char), 1, output);
		}
	fclose(output);
}

MATRIX**  readFile(char* file, int* size) {
	FILE* input; 
	MATRIX** matrix;
	int i, j, n, width, height;
	char a, b, c;
	input = fopen(file, "rb");
	fscanf(input, "%c%c %d %d %d%c", &a, &b, &width,&height, &n, &c);
	matrix = (MATRIX**)malloc(width*sizeof(MATRIX*));
	for(i = 0; i < width; i++) 
		matrix[i] = (MATRIX*)malloc(height*sizeof(MATRIX));
	for(i = 0; i < height; i++) 
		for(j = 0; j < width; j++) 
			fread(&matrix[i][j], sizeof(struct matrix), 1, input);   
	fclose(input);
	*size = width;
	return matrix;
}

void printFile(char* file, int leaf, int node, TREE* vector) {
	FILE* output;
	int i;
	output = fopen( file, "wb" );
	fwrite(&leaf, sizeof(int), 1, output);
	fwrite(&node, sizeof(int), 1, output);
	for(i = 0; i < node; i++) {
		fwrite(&vector[i]->blue, sizeof(unsigned char), 1, output);
		fwrite(&vector[i]->green, sizeof(unsigned char), 1, output);
		fwrite(&vector[i]->red, sizeof(unsigned char), 1, output);
		fwrite(&vector[i]->area, sizeof(uint32_t), 1, output);
		fwrite(&vector[i]->t_l, sizeof(int32_t), 1, output);
		fwrite(&vector[i]->t_r, sizeof(int32_t), 1, output);
		fwrite(&vector[i]->b_l, sizeof(int32_t), 1, output);
		fwrite(&vector[i]->b_r, sizeof(int32_t), 1, output);
	}
	fclose( output );
}

QuadtreeNode* scanFile(char *file, int* leaf, int* node) {
	FILE* input;
	QuadtreeNode* vector;
	input = fopen(file, "rb");
	fread(leaf, sizeof(int), 1, input);
	fread(node, sizeof(int), 1, input);
	vector = (QuadtreeNode*)malloc(*node*sizeof(QuadtreeNode));
	fread(vector, sizeof(QuadtreeNode), *node, input);
	fclose(input);
	return vector;
}

TREE createTree(int left_x, int left_y, int right_x, int right_y, MATRIX** matrix, int factor) {
	long long red = 0, green = 0 , blue = 0, suma = 0;  
	int i, j, mean;

	for(i = left_x; i < right_x; i++) 
		for(j = left_y; j < right_y; j++) {
			red = red + matrix[i][j].red;
			green = green + matrix[i][j].green;
			blue = blue + matrix[i][j].blue;
		}

	red = red / ((right_x - left_x) * (right_x - left_x));
	green = green / ((right_x - left_x) * (right_x - left_x));
	blue = blue / ((right_x - left_x) * (right_x - left_x));

	TREE tree = initTree(red, green, blue, (right_x - left_x) * (right_x - left_x));

	for(i = left_x; i < right_x; i++) 
		for(j = left_y; j < right_y; j++) 
			suma = suma + (red - matrix[i][j].red)*(red - matrix[i][j].red) + (green - matrix[i][j].green)*(green - matrix[i][j].green) + (blue - matrix[i][j].blue)*(blue - matrix[i][j].blue);

	mean = suma / (3 * (right_x - left_x) * (right_x - left_x));
    
	if(mean > factor) {
		int middle_x, middle_y;
		middle_x = (left_x + right_x) / 2;
		middle_y = (left_y + right_y) / 2;
		tree->top_left = createTree(left_x, left_y, middle_x, middle_y, matrix, factor);
		tree->top_right = createTree(left_x, middle_y,  middle_x, right_y, matrix, factor);
		tree->bottom_right = createTree(middle_x, middle_y, right_x, right_y, matrix, factor);
		tree->bottom_left = createTree(middle_x, left_y,  right_x, middle_y, matrix, factor);
	}
	return tree;
}

void count(TREE tree, int* leaf, int* node) {
	*node = *node + 1;
	if(tree != NULL) { 
		if(tree->top_left == NULL) 
			*leaf = *leaf + 1;
		else {
			count(tree->top_left, leaf, node);
			count(tree->top_right, leaf, node);
			count(tree->bottom_right, leaf, node);
			count(tree->bottom_left, leaf, node);
		}
	}
}

void createVector(TREE* vector, TREE tree, int node) {
	int n = 1, i, first = 0, last = 1;
	vector[0] = tree;
     
	while(n != node) {
		for(i = first; i < last; i++) 
			if( vector[i]->top_left != NULL ) {
				vector[n] = vector[i]->top_left;
				vector[i]->t_l = n;
				vector[n+1] = vector[i]->top_right;
				vector[i]->t_r = n+1;
				vector[n+2] = vector[i]->bottom_right;
				vector[i]->b_r = n+2;
				vector[n+3] = vector[i]->bottom_left;
				vector[i]->b_l = n+3;
				n = n + 4;
			}
		first = last;
		last = n;
	}
}

void compress(int factor, char* input_file, char* output_file) {
	int i, size, leaf = 0, node = 0;
	MATRIX** matrix;
	TREE tree;
	TREE* vector;
  
	matrix = readFile(input_file, &size);
	tree = createTree(0, 0, size, size, matrix, factor); 
	count(tree, &leaf, &node);
	vector = (TREE*)malloc(node*sizeof(TREE)); 
	createVector(vector, tree, node);
	printFile(output_file, leaf, node, vector);
	
	for(i = 0; i < size; i++) 
		free(matrix[i]);   
	free(matrix);
	delete(tree);
	free(vector);
}

void createTree_D( QuadtreeNode* vector, TREE tree, int i ) {
	if( vector[i].top_left != -1 ) {
		tree->top_left = initTree(vector[vector[i].top_left].red, vector[vector[i].top_left].green, vector[vector[i].top_left].blue, vector[vector[i].top_left].area);
		createTree_D(vector, tree->top_left, vector[i].top_left);
    
		tree->top_right = initTree(vector[vector[i].top_right].red, vector[vector[i].top_right].green, vector[vector[i].top_right].blue, vector[vector[i].top_right].area);
		createTree_D(vector, tree->top_right, vector[i].top_right);
   
		tree->bottom_right = initTree(vector[vector[i].bottom_right].red, vector[vector[i].bottom_right].green, vector[vector[i].bottom_right].blue, vector[vector[i].top_right].area);
		createTree_D(vector, tree->bottom_right, vector[i].bottom_right ;
    
		tree->bottom_left = initTree(vector[vector[i].bottom_left].red, vector[vector[i].bottom_left].green, vector[vector[i].bottom_left].blue, vector[vector[i].bottom_left].area);
		createTree_D(vector, tree->bottom_left, vector[i].bottom_left);
	}
}

void createMatrix(int left_x, int left_y, int right_x, int right_y, TREE tree, MATRIX** matrix) {
	int i, j;
	if(tree->top_left == NULL) {
		for(i = left_x; i<right_x; i++)
			for(j = left_y; j < right_y; j++) {
				matrix[i][j].red = tree->red;
				matrix[i][j].green = tree->green;
				matrix[i][j].blue = tree->blue;
			}
	}
	else {
		int middle_x, middle_y;
		middle_x = (left_x + right_x)/2;
		middle_y = (left_y + right_y)/2;
		createMatrix(left_x, left_y, middle_x, middle_y, tree->top_left, matrix);
		createMatrix(left_x, middle_y,  middle_x, right_y, tree->top_right, matrix);
		createMatrix(middle_x, middle_y, right_x, right_y, tree->bottom_right, matrix);
		createMatrix(middle_x, left_y,  right_x, middle_y, tree->bottom_left, matrix);
	}
}

void decompress(char* input_file, char* output_file) {
	int i, size, leaf, node;
	TREE tree;
	QuadtreeNode* vector;
	MATRIX** matrix;

	vector = scanFile(input_file, &leaf, &node);
	tree = initTree(vector[0].red, vector[0].green, vector[0].blue, vector[0].area);  
	createTree_D(vector, tree, 0);
	size = sqrt(vector[0].area);
	matrix = (MATRIX**)malloc(size*sizeof(MATRIX*));
	for(i = 0; i < size; i++)
		matrix[i] = (MATRIX*)malloc(size*sizeof(MATRIX));
	createMatrix(0, 0, size, size, tree, matrix);
	writeFile(output_file, size, matrix);

	for(i = 0; i < size; i++) 
		free(matrix[i]);   
	free(matrix);
	delete(tree);
	free(vector);
}

void invers(TREE tree, char type) {
	if(tree != NULL) {
		TREE aux;
		if(type == 'h') {
			aux = tree->top_left;
			tree->top_left = tree->top_right;
			tree->top_right = aux;
			aux = tree->bottom_left;
			tree->bottom_left = tree->bottom_right;
			tree->bottom_right = aux;
		}
		else {
			aux = tree->top_left;
			tree->top_left = tree->bottom_left;
			tree->bottom_left = aux;
			aux = tree->bottom_right;
			tree->bottom_right = tree->top_right;
			tree->top_right = aux;
		}
		invers(tree->top_left, type);
		invers(tree->top_right, type);
		invers(tree->bottom_left, type);
		invers(tree->bottom_right, type);
	}
}

void mirror(char type, int factor, char* input_file, char* output_file) {
	int i, size;
	MATRIX** matrix;
	TREE tree; 
 
	matrix = readFile(input_file, &size);
	tree = createTree(0, 0, size, size, matrix, factor); 
	invers(tree, type);  
	createMatrix(0, 0, size, size, tree, matrix);
	writeFile(output_file, size, matrix);

	for(i = 0; i < size; i++) 
		free(matrix[i]);   
	free(matrix);
	delete(tree);
}

TREE middle(TREE tree1, TREE tree2) {
	TREE tree3 = initTree(tree1->red, tree1->green, tree1->blue, tree1->area);
	tree3->red = ( ree1->red + tree2->red)/2;
	tree3->green = (tree1->green + tree2->green)/2;
	tree3->blue = (tree1->blue + tree2->blue)/2;

	if(!(tree1->top_left == NULL && tree2->top_left == NULL)) {
		if(tree1->top_left == NULL && tree2->top_left != NULL) {
			tree3->top_left = middle(tree1, tree2->top_left);
			tree3->top_right = middle(tree1, tree2->top_right);
			tree3->bottom_left = middle(tree1, tree2->bottom_left);
			tree3->bottom_right = middle(tree1, tree2->bottom_right);
		}
		else if(tree2->top_left == NULL)  {
			tree3->top_left = middle(tree1->top_left, tree2);
			tree3->top_right = middle(tree1->top_right, tree2);
			tree3->bottom_left = middle(tree1->bottom_left, tree2);
			tree3->bottom_right = middle(tree1->bottom_right, tree2);
		}
		else {
			tree3->top_left = middle(tree1->top_left, tree2->top_left);
			tree3->top_right = middle(tree1->top_right, tree2->top_right);
			tree3->bottom_left = middle(tree1->bottom_left, tree2->bottom_left);
			tree3->bottom_right = middle(tree1->bottom_right, tree2->bottom_right);
		}
	}
	return tree3;
}

void bonus(int factor, char* file1, char* file2, char* file3) {
	int size,i;
	MATRIX** matrix1;
	MATRIX** matrix2;
	TREE tree1, tree2, tree3;  

	matrix1 = readFile(file1, &size);
	tree1 = createTree(0, 0, size, size, matrix1, factor);
	matrix2 = readFile(file2, &size);
	tree2 = createTree(0, 0, size, size, matrix2, factor);
	tree3 = middle(tree1, tree2);
	createMatrix(0, 0, size, size, tree3, matrix1);
	writeFile(file3, size, matrix1);

	for(i = 0; i < size; i++) 
		free(matrix1[i]);   
	free(matrix1);
	for(i = 0; i < size; i++) 
		free(matrix2[i]);   
	free(matrix2);
	delete(tree1);
	delete(tree2);
	delete(tree3);
}

int main(int argc, char **argv) {
	if (argv[1][1] == 'c') 
		compress(atoi(argv[2]), argv[3], argv[4]); 
	else if(argv[1][1] == 'd') 
		decompress(argv[2], argv[3]);
	else if(argv[1][1] == 'm') 
		mirror(argv[2][0], atoi(argv[3]), argv[4], argv[5]);
	else if(argv[1][1] == 'o')
		bonus(atoi(argv[2]), argv[3], argv[4], argv[5]);
	return 0; 
}
