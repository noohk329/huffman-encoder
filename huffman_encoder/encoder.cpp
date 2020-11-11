/*
* 학번: 201723303
* 이름: 노현경
*
* 허프만 코딩을 사용하여 아스키 코드로 표현되어있는 텍스트 문서를 입력받아 허프만 코딩으로 압축된 파일 저장하는 프로그램
* 입력: 아스키 코드로 표현되어 있는 텍스트 문서 input.txt
* 출력: 바이너리 코드로 압축한 파일 encoded.bin
*/


#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIT_SIZE 7
#define NUM_CHARS 256
#define MAX_CODE 100

/* 허프만 트리 노드 구조체 선언 */
typedef struct treeNode treenode;
struct treeNode {
	int freq;
	unsigned char ch;
	char code[MAX_CODE];
	treenode* left, * right;
};

/* 우선순위 큐로 힙 구조체 선언 */
typedef struct pq {
	int heap_size;
	treenode* A[NUM_CHARS];
}PQ;

typedef struct code_table {
	char code[NUM_CHARS];
};

code_table codebook[NUM_CHARS];

void create_pq(PQ* p) {
	p->heap_size = 0;
}

int parent(int i) {
	return (i - 1) / 2;
}

int left(int i) {
	return i * 2 + 1;
}

int right(int i) {
	return i * 2 + 2;
}

void insertPQ(PQ* p, treenode* element) {
	int i;

	p->heap_size++;
	i = p->heap_size - 1;

	while ((i > 0 ) && (p->A[parent(i)]->freq > element->freq))
	{
		p->A[i] = p->A[parent(i)];
		i = parent(i);
	}
	p->A[i] = element;
}


void heapify(PQ* p, int i) {
	int		l, r, smallest;
	treenode* t;

	l = left(i);
	r = right(i);

	if (l < p->heap_size && p->A[l]->freq < p->A[i]->freq)
		smallest = l;
	else
		smallest = i;
	if (r < p->heap_size && p->A[r]->freq < p->A[smallest]->freq)
		smallest = r;

	if (smallest != i) {
		t = p->A[i];
		p->A[i] = p->A[smallest];
		p->A[smallest] = t;
		heapify(p, smallest);
	}
}


treenode *extractPQ(PQ* p) {
	treenode *item;
	int parent, child;

	if (p->heap_size == 0) {
		printf("Heap underflow!!\n");
		exit(1);
	}

	item = p->A[0];
	p->A[0] = p->A[(p->heap_size)-1];
	p->heap_size--;
	
	heapify(p, 0);
	return item;
}

void destroy_tree(treenode* root) {
	if (root == NULL) return;
	destroy_tree(root->left);
	destroy_tree(root->right);
	free(root);
}

unsigned int get_freq(FILE* f, unsigned int v[]) {
	int r, n;
	
	for (n = 0;; n++) {
		r = fgetc(f);
		
		//text[n] = (char)r;
		
		if (feof(f)) break;

		v[r]++; // 아스키 코드 값의 빈도수 저장 
	}

	fclose(f);
	printf("%s\n", text);
	return n;
}

treenode* build_huffman(unsigned int freq[]) {
	int n;
	treenode* x, * y, * z;
	PQ p;

	create_pq(&p);

	for (int i = 0; i < NUM_CHARS; i++) {
		if (freq[i] > 0) {
			x = (treenode*)malloc(sizeof(treenode));

			x->left = NULL;
			x->right = NULL;
			x->freq = freq[i];
			x->ch = (char)i;
			memset(x->code, NULL, sizeof(x->code));

			insertPQ(&p, x);
		}
	}

	n = p.heap_size - 1;

	for (int i = 0; i < n; i++) {
		z = (treenode*)malloc(sizeof(treenode));

		x = extractPQ(&p);
		y = extractPQ(&p);

		z->left = x;
		z->right = y;
		z->freq = x->freq + y->freq;
		memset(z->code, NULL, sizeof(z->code));

		insertPQ(&p, z);
	}

	return extractPQ(&p);
}


/* ----------------------------------- 압축 파트 ----------------------------------------------*/

unsigned char pack(char* str) {
	unsigned char result = 0;

	for (int i = 7; i >= 0; i--) {
		if (*str == '1') {
			result = result | (1 << i);  
		}
		str++;
	}

	return result;
}

void compress_file(FILE* infile, FILE* outfile) {
	unsigned char byte;
	char buf[MAX_CODE];

	int locTotalNumOfBit;
	locTotalNumOfBit = ftell(outfile);
	if (fseek(outfile, 4, SEEK_CUR) != 0)
	{
		fclose(infile);
		fclose(outfile);
		return;
	}

	char bitBuf[MAX_CODE]; 
	int bitBufIdx = 0;
	int bitShiftCnt = 7;
	int totalBitNum = 0; 
	char flag = 0; 
	memset(bitBuf, 0, MAX_CODE);

	printf("\nEncoding Result: \n");

	while (fgets(buf, MAX_CODE, infile) != 0)
	{
		int len = strlen(buf);//읽어들인거의 길이
		for (int i = 0; i < len; i++)
		{
			char* huffmanCode = codebook[(int)buf[i]].code;
			printf("%s", huffmanCode);
			for (int j = 0; j < strlen(huffmanCode); j++)
			{
				char val = 0;
				if (huffmanCode[j] == '0')	{val = 0;}
				else if (huffmanCode[j] == '1') {val = 1;}

				val = val << bitShiftCnt;
				bitShiftCnt--;
				bitBuf[bitBufIdx] |= val;
				flag = 1;
				totalBitNum++;
				
				if (bitShiftCnt < 0)
				{
					bitShiftCnt = 7;
					bitBufIdx++;
					if (bitBufIdx >= MAX_CODE)
					{
						fwrite(bitBuf, 1, MAX_CODE, outfile);
						flag = 0;
						bitBufIdx = 0;
						memset(bitBuf, 0, MAX_CODE);
					}
				}
			}
		}
	}
		
	if (flag == 1)
	{
		fwrite(bitBuf, 1, bitBufIdx + 1, outfile);
	}

	if (fseek(outfile, locTotalNumOfBit, SEEK_SET) == 0)
	{
		fwrite(&totalBitNum, sizeof(totalBitNum), 1, outfile);
	}
	
	fclose(infile);
}

	//int codelen = strlen(*codes);
	
	//buf_ptr = *codes; // 첫글자
	
	//for (int i = 0; i * 8 < strlen(*codes); i++) {
		//byte = pack(buf_ptr);
		//fputc(byte, outfile);
		//buf_ptr += 8;
	//}


void makeCode(treenode* root, int level, FILE *out) {
	static char code[NUM_CHARS];
	char writeBuf[100];

	if (root) {
		code[level] = '0';
		makeCode(root->left, level+1, out);

		code[level] = '1';
		makeCode(root->right, level+1, out);
		
		if ((root->left == NULL) && (root->right == NULL)) {
			code[level] = 0;
			printf("%c(%d), %d, %s, %d\n", root->ch, (int)root->ch, root->freq, _strdup(code), strlen(_strdup(code)));
			strcpy(root->code , _strdup(code));
			strcpy(codebook[root->ch].code, _strdup(code));
			
			writeBuf[0] = root->ch;
			strcpy(&writeBuf[1], _strdup(code));
			fwrite(writeBuf, sizeof(char), 1 + strlen(_strdup(code)), out);

		}
	}
}

int main(int argc, char* argv[]) {
	FILE* input_file, * output_file;
	treenode* root;

	unsigned int n, freq[NUM_CHARS];

	char* codes[NUM_CHARS];
	char fname[] = "encoded.dat";

	char string[NUM_CHARS];

	memset(freq, 0, sizeof(freq));

	input_file = fopen("input.txt", "rt");
	if (!input_file) {
		perror("input.txt");
		exit(1);
	}
	

	n = get_freq(input_file, freq);
	root = build_huffman(freq);
	fclose(input_file);

	output_file = fopen(fname, "wb");
	if (!output_file) {
		perror(fname);
		exit(1);
	}

	// huffman 트리 정보 입력
	fwrite(&n, sizeof(n),1,  output_file);
	makeCode(root, 0, output_file); // code 정보 입력
	
	input_file = fopen("input.txt", "rt");
	if (!input_file) {
		perror("input.txt");
		exit(1);
	}

	compress_file(input_file, output_file);
	fclose(output_file);
	destroy_tree(root);

	exit(0);
}


