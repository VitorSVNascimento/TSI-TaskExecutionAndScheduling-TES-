#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Requerido por fork, getpid e getppid.
#include <sys/wait.h> // Requerido por wait.
#include "tes.h"

int exibirTes(const char *tes){
    const char *COMANDO_SAIDA = "exit"; 
    int sair = 0,erro,tam,numArq;
    pid_t pid1,pid2,pp;
    char linhaComandoTes[TAMANHO_PROMPT],*tokens;
    Tarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS];
    while (!sair){
        pid2=-1;
        printf("\n%s",tes);
        tam = lerString(linhaComandoTes,TAMANHO_PROMPT);
        if(tam!=0){
            if(!strcmp(linhaComandoTes,COMANDO_SAIDA))
                sair = 1;
            else{
                tokens = strtok(linhaComandoTes," ");    
                numArq=0;
                //loop que passa passa por cada arquivo informado carregando ele para a memória        
                while (tokens != NULL){
                    if(numArq >= NUMERO_MAXIMO_DE_PROGRAMAS){
                        printf("\nNumero maximo de programas carregados");
                        break;
                    }
                    erro = montarPrograma(&tarefas[numArq].programa, tokens);
                    if(erro){
                        tarefas[numArq].identificador = numArq+1;
                        numArq++;
                    }
                    
                    tokens = strtok(NULL," ");
                }
                pp = criarProcesso();
                pid1 = getpid();
                if(pp!=0 && numArq > 2){
                    pp = criarProcesso();
                    pid2 = getpid();
                }

                if(pp==pid1 && pp!=0){
                
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

    for(int i=0;i<programa->numeroDeInstrucoes;i++)
        printf("INSTRUÇÃO %d = %s",i,programa->instrucoes[i]);

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

    printf("\nValor de I = %d",i);
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
