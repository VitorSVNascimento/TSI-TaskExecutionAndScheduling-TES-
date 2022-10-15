#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tes.h"

int exibirTes(const char *tes){
    const char *COMANDO_SAIDA = "exit"; 
    int sair = 0,erro,numArq;
    char linhaComandoTes[TAMANHO_PROMPT],*tokens;
    Programa programa;
    while (!sair){
        
        printf("\n%s",tes);
        erro = lerString(linhaComandoTes,TAMANHO_PROMPT);
        if(erro==0)
            printf("\nnada");
        if(!strcmp(linhaComandoTes,COMANDO_SAIDA))
            sair = 1;
        else{
            tokens = strtok(linhaComandoTes," ");    
            //loop que passa passa por cada arquivo informado carregando ele para a memória        
            while (tokens != NULL){
                erro = montarPrograma(&programa, tokens);
                if(!erro)
                    numArq++;
                tokens = strtok(NULL," ");
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
        printf("\nPrograma não encontrado");
        return 0;
    }
    strcpy(programa->nome,nomePrograma);

    printf("\nPrograma %s aberto com sucesso",programa->nome);

     programa->numeroDeInstrucoes = lerArquivoLPAS(programa->instrucoes,arquivo);
    for(int k=0;k<programa->numeroDeInstrucoes;k++)
    printf("\nInstrução %d = %s",k,programa->instrucoes[k]);
    fclose(arquivo);
    return 1;
}

unsigned short lerArquivoLPAS(Instrucao instrucoes[NUMERO_MAXIMO_DE_INSTRUCOES],FILE *arquivo){
    int i=0,k=0;
    char c;
    printf("\nAntes do primeiro while");
    while((c = getc(arquivo))!=EOF){
        if(c == ';'){
            while (getc(arquivo)!='\n');
            if(k>0)
                i++;
            k=0;
            
        }else if(c!='\n'){
            instrucoes[i][k] = c;
            k++;
        }else if(c=='\n'){
            instrucoes[i][k] = '\0';
            i++;
            k=0;
        }
        
    }
    return i;
}
