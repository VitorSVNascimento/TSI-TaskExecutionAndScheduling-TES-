#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include "tes.h"

int exibirTes(const char *tes){
    const char *COMANDO_SAIDA = "exit"; 
    int numArq,numTarefas;
    pid_t pid1,pid2,pp;
    char linhaComandoTes[TAMANHO_PROMPT];
    Tarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS];
    while (1){
        pid2=-1;
        printf("\n%s",tes);
        if(lerString(linhaComandoTes,TAMANHO_PROMPT)==0)
            continue;
        if(!strcmp(linhaComandoTes,COMANDO_SAIDA))
            break;
            //loop que passa passa por cada arquivo informado carregando ele para a memória    
        gerarTarefas(&numArq,tarefas,linhaComandoTes);    
        if(numArq>0){
                            // cria os processos do fork
            pp = criarProcesso();
            pid1 = getpid();
            //cria o primeiro processo e verifica se é necessario criar o segundo
            if(pp!=0 && numArq > NUMERO_MAXIMO_DE_EXECUCOES){
                pp = criarProcesso();
                pid2 = getpid();
            }

            if(pp!=0){
                wait(&pp);
                if(pid2!=-1)
                wait(&pp);
            }else{
                pp = getpid();
                if(pp == pid1){
                    if(numArq >= NUMERO_MAXIMO_DE_EXECUCOES)
                        numTarefas = NUMERO_MAXIMO_DE_EXECUCOES;
                    else
                        numTarefas = 1;
                    executarTarefas(tarefas,numTarefas);
                }

                if(pp == pid2){
                    numTarefas = numArq - NUMERO_MAXIMO_DE_EXECUCOES;
                    executarTarefas(tarefas+NUMERO_MAXIMO_DE_EXECUCOES,numTarefas);
                }
            }
        }

        
        
    }

    return EXIT_SUCCESS;
}

int inicializaTes(){
    char *tes = "tes >";
    exibirTes(tes);
    return EXIT_SUCCESS;
}
int main(){
    return inicializaTes();
}

int lerString(char *str,int tamStr){

    setbuf(stdin,NULL);
    fgets(str,tamStr,stdin);
    setbuf(stdin,NULL);
    retiraN(str);
    return strlen(str);
}

void gerarTarefas(int *numArq,Tarefa tarefas[], char *linhaComandoTes){
    char *tokens;
    int sucesso;
    unsigned int num;
    tokens = strtok(linhaComandoTes," ");    
    num=0;
    while (tokens != NULL){
        if(num >= NUMERO_MAXIMO_DE_PROGRAMAS){
            printf("\nNumero maximo de programas carregados\n");
            break;
        }
        sucesso = montarPrograma(&tarefas[num].programa, tokens);
        if(sucesso){
            tarefas[num].identificador = num+1;
            num++;
        }
        
        tokens = strtok(NULL," ");
    }
    *numArq=num;
}

void retiraN(char *str){

    int tam = strlen(str);
    if(str[tam-1]=='\n')
        str[tam-1]='\0';
    return ;
}

int abrirArquivo(char *nomeArq,FILE **arq){

    *arq=fopen(nomeArq,"r");
    if(*arq==NULL)
        return 0;
    return 1;
}

int montarPrograma(Programa *programa,char *nomePrograma){
    const char *ENSTENSAO = ".lpas";
    char aux[TAMANHO_NOME_ARQUIVO+10];
    strcpy(aux,nomePrograma);
    strcat(aux,ENSTENSAO);
    FILE *arquivo;
    
    if(!abrirArquivo(aux,&arquivo)){
        printf("\nPrograma %s não encontrado",nomePrograma);
        return 0;
    }
    strcpy(programa->nome,nomePrograma);
    programa->numeroDeInstrucoes = lerArquivoLPAS(programa->instrucoes,arquivo);

    fclose(arquivo);
    return 1;
}

unsigned short lerArquivoLPAS(Instrucao instrucoes[NUMERO_MAXIMO_DE_INSTRUCOES],FILE *arquivo){
    int i=0,k=0;
    char c;

    while((c = getc(arquivo))!=EOF){
        if(c == COMENTARIO){
            while (getc(arquivo)!=FIM_DE_LINHA);
            if(k>0)
                i++;
            k=0;
            
        }else if(c!=FIM_DE_LINHA && c!='\r'){
            instrucoes[i][k] = c;
            k++;
        }else if(c==FIM_DE_LINHA || k >= 10){
            instrucoes[i][k] = '\0';
            i++;
            k=0;
        }
        
    }
    k = strlen(instrucoes[i]);
    if(!k)
        i--;
    else if(k>TAMANHO_INSTRUCAO-1)
        instrucoes[i][TAMANHO_INSTRUCAO-1] = '\0';

    return i+1;
}

pid_t criarProcesso() {
	// Cria o processo filho
	pid_t pid = fork();
			
	// Verifica se ocorreu um erro na criação do processo filho.
	if (pid < 0) {
		printf("\nERRO: o processo filho não foi criado.\n\n");
		exit(EXIT_FAILURE);
	}
	return pid;
}

int executarTarefas(Tarefa tarefas[],int numTarefas){
    MaquinaExecucao me;
    for(int i = 0 ; i < numTarefas ; i++)
        inicializaDescritorDeTarefa(&me.df[i],&tarefas[i]);
    
    for(int i = 0 ; i<numTarefas; i++){
        printf("\nValor da tarefa = %d",me.df[i].tarefa.identificador);
        printf("\nEstado = %d",me.df[i].estado);
    }
    exit(EXIT_SUCCESS);
}

void inicializaDescritorDeTarefa(DescritorTarefa *df,Tarefa *tarefa){

    df->tarefa = *tarefa;
    df->pc = 0;
    df->tempoCPU = 0;
    df->tempoES = 0;
    df->estado = PRONTA;

}