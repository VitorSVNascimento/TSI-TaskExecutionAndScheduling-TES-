#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include "tes.h"

int exibirTes(const char *tes){
    const char *COMANDO_SAIDA = "exit"; 
    int numArq,numTarefas=0;
    pid_t pid1,pid2,pidAtual,teste;
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
            pidAtual = criarProcesso();
            pid1 = getpid();
            //cria o primeiro processo e verifica se é necessario criar o segundo
            if(pidAtual!=0 && numArq > NUMERO_MAXIMO_DE_EXECUCOES){
                pidAtual = criarProcesso();
                pid2 = getpid();
            }

            if(pidAtual!=0){
                wait(&teste);
                if(pid2!=-1)
                    wait(&teste);
            }else{
                pidAtual = getpid();
                if(pidAtual == pid1){
                    if(numArq >= NUMERO_MAXIMO_DE_EXECUCOES)
                        numTarefas = NUMERO_MAXIMO_DE_EXECUCOES;
                    else
                    numTarefas = 1;
                    executarTarefas(tarefas,numTarefas);
                }

                if(pidAtual == pid2){
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
    char c,cc;

    while((c = getc(arquivo))!=EOF){
        if(c == COMENTARIO){
            while ((cc = getc(arquivo)))
                if(cc==FIM_DE_LINHA || cc == EOF)
                    break;
            if(k>0){
                instrucoes[i][k] = '\0';
                i++;
            }
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
    int cod,erro;
    char nomeVar[TAMANHO_INSTRUCAO];
    me.numeroDeProgramas = numTarefas;
    for(int i = 0 ; i < me.numeroDeProgramas ; i++)
        inicializaDescritorDeTarefa(&me.df[i],&tarefas[i]);

    while (me.numeroDeProgramas > 0){
        for (me.df->pc = 0 ; me.df->pc < me.df[0].tarefa.programa.numeroDeInstrucoes; me.df->pc++){
            cod = decodificaInstrucao(me.df[0].tarefa.programa.instrucoes[me.df[0].pc],nomeVar);
            if(cod==HALT)
                break;
            erro = executarInstrucao(cod,nomeVar,&me);
            if(erro)
                printaErro(me.df[0].tarefa.programa.nome,erro);
        }
        break;
    }
    
    exit(EXIT_SUCCESS);
}

void inicializaDescritorDeTarefa(DescritorTarefa *df,Tarefa *tarefa){
    df->tarefa = *tarefa;
    df->pc = 0;
    df->tempoCPU = 0;
    df->tempoES = 0;
    df->valorRegistrador = 0;
    df->numVariavel = 0;
    df->estado = PRONTA;
}

int decodificaInstrucao(Instrucao instrucao,char nomeVar[TAMANHO_INSTRUCAO]){
    const int TAM_VETOR = 9;
    char nomeComando[][TAMANHO_INSTRUCAO] = {"READ","WRITE","LOAD","STORE","ADD","SUB","MUL","DIV","HALT"};
    int codComando[] = {READ,WRITE,LOAD,STORE,ADD,SUB,MUL,DIV,HALT};
    char *tokens,nomeInstrucao[TAMANHO_INSTRUCAO],aux[TAMANHO_INSTRUCAO];
    strcpy(aux,instrucao);
    tokens = strtok(aux," ");  
    strcpy(nomeInstrucao,tokens);
    tokens = strtok(NULL," "); 
    strcpy(nomeVar,tokens); 
    tokens = strtok(NULL," "); 
    if(tokens!=NULL)
        if(strlen(tokens))
            return MUITAS_INSTRUCOES;

    for(int i = 0 ; i < TAM_VETOR ; i++)
        if(!strcmp(nomeInstrucao,nomeComando[i]))
            return codComando[i];
    return INSTRUCAO_LPAS_INVALIDA;
}

int executarInstrucao(int cod,char nomeVar[TAMANHO_INSTRUCAO],MaquinaExecucao *me){
    int expressao,existe;
    expressao = atoi(nomeVar);
    switch (cod){
    case READ:
        if(expressao)
            return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
        if(existe==-1){
            existe = criarVariavel(nomeVar,&me->df[0]);
            if(existe==-1)
                return -1;
        }
        printf("\n%s->",me->df[0].tarefa.programa.nome);
        me->variaveis[existe] = funcaoReadLpas();
        break;
    case WRITE:
        if(expressao){
            printf("\n%s->",me->df[0].tarefa.programa.nome);
            funcaoWriteLpas(expressao);    
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1){
                printf("\n%s->",me->df[0].tarefa.programa.nome);
                funcaoWriteLpas(me->variaveis[existe]);
            }
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;

        }
        break;
    case LOAD:
        if(expressao){
            funcaoLoadLpas(&me->registrador,expressao);
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1)
                funcaoLoadLpas(&me->registrador,me->variaveis[existe]);
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        } 
        break;
    case STORE:
        if(expressao)
            return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
        if(existe==-1){
            existe = criarVariavel(nomeVar,&me->df[0]);
            if(existe==-1)
                return -1;
        }
        funcaoStoreLpas(&me->variaveis[existe],me->registrador);
        me->variaveis[existe] = me->registrador;        
        break;
    case ADD:
        if(expressao){
            funcaoAddLpas(&me->registrador,expressao);
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1)
                funcaoAddLpas(&me->registrador,me->variaveis[existe]);
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        } 
        break;
    case SUB:
        if(expressao){
            funcaoSubLpas(&me->registrador,expressao);
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1)
                funcaoSubLpas(&me->registrador,me->variaveis[existe]);
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        } 
        break;
    case MUL:
        if(expressao){
            funcaoMulLpas(&me->registrador,expressao);
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1)
                funcaoMulLpas(&me->registrador,me->variaveis[existe]);
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        } 
        break;
    case DIV:
        if(expressao){
            funcaoDivLpas(&me->registrador,expressao);
        }else{
            existe = verificaVariavel(nomeVar,me->df[0].variaveisTarefa,me->df[0].numVariavel);
            if(existe!=-1)
                funcaoDivLpas(&me->registrador,me->variaveis[existe]);
            else
                return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
        } 
        break;
    default:
        return INSTRUCAO_LPAS_INVALIDA;
        break;
    }

    return 0;
}

void printaErro(char *nomePrograma,int erro){
    printf("\n%s:",nomePrograma);
    switch(erro){
        case INSTRUCAO_LPAS_INVALIDA:
            printf(" instrução LPAS inválida\n");
            break;
        case ARGUMENTO_INSTRUCAO_LPAS_INVALIDO:
            printf(" argumento instrução LPAS inválido\n");
            break;
        case ARGUMENTO_INSTRUCAO_LPAS_AUSENTE:
            printf(" argumento instrução LPAS ausente\n");
            break;
        case MUITAS_INSTRUCOES:
            printf(" muitas instruções LPAS\n");
            break;
        case SIMBOLO_INVALIDO:
            printf(" simbolo inválido\n");
            break;
            
    }
}

int verificaVariavel(char nomeVar[TAMANHO_INSTRUCAO],Variavel variavies[NUMERO_MAXIMO_DE_VARIAVEIS],int tam){

    for(int i = 0; i < tam ; i++)
        if(!strcmp(nomeVar,variavies[i].nomeVariavel))
            return i;
    return -1;
}

int criarVariavel(char nomeVar[TAMANHO_INSTRUCAO],DescritorTarefa *df){
    if(df->numVariavel < NUMERO_MAXIMO_DE_VARIAVEIS){
        strcpy(df->variaveisTarefa[df->numVariavel].nomeVariavel,nomeVar); 
        df->numVariavel++;
        return df->numVariavel-1;
    }
    return -1;
}

int funcaoReadLpas(){
    int valor;
    printf("READ: ");
    scanf("%d",&valor);
    return valor;
}

void funcaoWriteLpas(int valor){
    printf("WRITE: %d\n",valor);
    return;
}

void funcaoLoadLpas(int *registrador,int valor){
    *registrador = valor;
    return;
}

void funcaoStoreLpas(int *variavel, int registrador){
    *variavel = registrador;
    return;
}
void funcaoAddLpas(int *registrador,int valor){
    *registrador += valor;
    return;
}
void funcaoSubLpas(int *registrador,int valor){
    *registrador -= valor;
    return;
}
void funcaoMulLpas(int *registrador,int valor){
    *registrador *= valor;
    return;
}
void funcaoDivLpas(int *registrador,int valor){
    *registrador /= valor;
    return;
}