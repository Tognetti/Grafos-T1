#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"
#include <graphviz/cgraph.h>

//------------------------------------------------------------------------------
// (apontador para) estrutura de dados para representar um grafo
//
// o grafo pode ser direcionado ou não
//
// o grafo tem um nome, que é uma "string"

struct grafo {
  Agraph_t *g;
};

//------------------------------------------------------------------------------
// desaloca toda a memória usada em *g
//
// devolve 1 em caso de sucesso,
//         ou
//         0, caso contrário

int destroi_grafo(grafo g) {
  agclose(g->g);
  free(g);
  return !g;
}
//------------------------------------------------------------------------------
// lê um grafo no formato dot de input
//
// devolve o grafo lido,
//         ou
//         NULL, em caso de erro

grafo le_grafo(FILE *input) {
  grafo graph;

  graph = malloc(sizeof (struct grafo));
  if(!graph)
    return NULL;

  graph->g = agread(input, NULL);

  return graph;
}
//------------------------------------------------------------------------------
// escreve o grafo g em output usando o formato dot.
//
// devolve o grafo escrito,
//         ou
//         NULL, em caso de erro

grafo escreve_grafo(FILE *output, grafo graph) {
  agwrite(graph->g, output);
  return graph;
}
//------------------------------------------------------------------------------
// devolve o grafo de recomendações de g
//
// cada aresta {c,p} de H representa uma recomendação do produto p
// para o consumidor c, e tem um atributo "weight" que é um inteiro
// representando a intensidade da recomendação do produto p para o
// consumidor c.
//
// cada vértice de g tem um atributo "tipo" cujo valor é 'c' ou 'p',
// conforme o vértice seja consumidor ou produto, respectivamente

grafo recomendacoes(grafo compras){
  grafo recomend;
  grafo consumidores; //Grafo para registrar as recomendações de um consumidor para outro (usado para garantir que não haja recomendações duplicadas)
  Agnode_t *consumidor, *produto, *semelhante, *recomendado;
  Agnode_t *con_recomend, *rec_recomend, *con_consumidores, *sem_consumidores; //Vertices para serem usados no grafo de recomendacoes e de consumidores
  Agedge_t *aresta1, *aresta2, *aresta3, *aresta_recomend;
  char *type, nome_aresta[50], nome_aresta_con[50], *weight_str;
  int weight;

  //Cria grafo de recomendações vazio
  recomend = malloc(sizeof(struct grafo));
  recomend->g = agopen("recomendacoes", Agundirected, NULL);
  //Seta peso default como 0
  agattr(recomend->g, AGEDGE, "weight", "0");

  //Cria grafo de consumidores vazio
  consumidores = malloc(sizeof(struct grafo));
  consumidores->g = agopen("consumidores", Agundirected, NULL);

  //Percorre todos os vértices
  for(consumidor = agfstnode(compras->g); consumidor; consumidor = agnxtnode(compras->g, consumidor)){
    type = agget(consumidor, "tipo");
    //Se for consumidor
    if(!strcmp(type, "c")){
      //Percorre todas as arestas saindo dos consumidores
      for(aresta1 = agfstedge(compras->g, consumidor); aresta1; aresta1 = agnxtedge(compras->g, aresta1, consumidor)){
        //Ve em qual ponta da aresta está o produto comprado
        if(!strcmp(agget(agtail(aresta1), "tipo"), "p")){
          produto = agtail(aresta1);
        }
        else{
          produto = aghead(aresta1);
        }

        //Percorre todas as arestas saindo dos produtos comprados
        for(aresta2 = agfstedge(compras->g, produto); aresta2; aresta2 = agnxtedge(compras->g, aresta2, produto)){
          //Ve em qual ponta da aresta está o consumidor que também comprou o produto (o semelhante)
          if(!strcmp(agget(agtail(aresta2), "tipo"), "c")){
            semelhante = agtail(aresta2);
          }
          else{
            semelhante = aghead(aresta2);
          }

          //Percorre todas as arestas saindo dos semelhantes
          for(aresta3 = agfstedge(compras->g, semelhante); aresta3; aresta3 = agnxtedge(compras->g, aresta3, semelhante)){
            //Ve em qual ponta da aresta está o produto a ser recomendado
            if(!strcmp(agget(agtail(aresta3), "tipo"), "p")){
              recomendado = agtail(aresta3);
            }
            else{
              recomendado = aghead(aresta3);
            }

            //Se o produto recomendado já não foi comprado pelo consumidor
            if(!agedge(compras->g, consumidor, recomendado, NULL, 0)){
              //Procura (ou cria caso não exista) os respectivos vértices no grafo de consumidores
              con_consumidores = agnode(consumidores->g, agnameof(consumidor), 1);
              sem_consumidores = agnode(consumidores->g, agnameof(semelhante), 1);
              //Cria nome da aresta de recomendacao entre os consumidores. O nome da aresta é o produto recomendado
              snprintf(nome_aresta_con, 50, "%s", agnameof(recomendado));

              //Ve se o produto já não foi recomendado para o consumidor pelo mesmo semelhante
              if(!agedge(consumidores->g, con_consumidores, sem_consumidores, nome_aresta_con, 0)){
                //Caso a recomendação nao exista ainda, cria aresta no grafo consumidores
                agedge(consumidores->g, con_consumidores, sem_consumidores, nome_aresta_con, 1);

                //Procura (ou cria caso não exista) os respectivos vértices no grafo de recomendacoes
                con_recomend = agnode(recomend->g, agnameof(consumidor), 1);
                rec_recomend = agnode(recomend->g, agnameof(recomendado), 1);

                //cria aresta de consumidor para produto recomendado
                snprintf(nome_aresta, 50, "%s%s", agnameof(consumidor), agnameof(recomendado));
                aresta_recomend = agedge(recomend->g, con_recomend, rec_recomend, nome_aresta, 1);
                printf("%s recomendou %s para %s\n", agnameof(semelhante), agnameof(recomendado), agnameof(consumidor));

                //Incrementa peso da aresta
                weight_str = agget(aresta_recomend, "weight");
                weight = atoi(weight_str);
                weight++;
                weight_str = (char *) malloc(10 * sizeof(char));
                snprintf(weight_str, 10, "%d", weight);
                agset(aresta_recomend, "weight", weight_str);
                free(weight_str);
                agattr(recomend->g, AGEDGE, "weight", "0");
                printf("peso %s = %s\n\n",agnameof(aresta_recomend), agget(aresta_recomend, "weight"));
              }
            }
          }
        }
      }
    }
  }


  return recomend;
}

//------------------------------------------------------------------------------
