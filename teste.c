#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include "grafo.h"

int main(int argc, char const *argv[]){
	FILE *input, *output;
	grafo compras, recomend;

	if(argc < 2){
		printf("Erro: especifique um arquivo de entrada no formato .dot\n");
		exit(1);
	}
	else{
		input = fopen(argv[1], "r");
	}
	output = fopen("recomendacoes.dot", "w");
	compras = le_grafo(input);
	recomend = recomendacoes(compras);
	escreve_grafo(output, recomend);
	destroi_grafo(compras);
	destroi_grafo(recomend);
	return 0;
}
