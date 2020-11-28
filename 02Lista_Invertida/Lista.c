#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<dirent.h>
#include<locale.h>
#include<ctype.h>

#define TOT_COLECAO 220 // docs 
#define TAM_VOC_CONS 3  //Hashing
#define TAM_CONS 9     //Qtd elementos da consulta
#define TAM_vocabulario_colecao 1000000// qtd vocabulos
#define TAM_DOC 2000000

char dados[TOT_COLECAO][TAM_DOC];//matriz de documentos
char consulta[TAM_CONS+1];
int lista_invertida[TAM_vocabulario_colecao][TOT_COLECAO];
int tab_hash[TAM_vocabulario_colecao];

typedef struct // Struct para vocabulario
{ char   termo[200];
  float  idf;
  int    num_docs;
}vocabulo;

vocabulo vocabulario_colecao[TAM_vocabulario_colecao];

typedef struct //Struct para termo
{ char  termo[200];
  int   tf;
  float peso;
}voc_cons;


struct dirent *lsdir;//diretorio para usar a funcao lsdir

   //FUNCOES DOS HASHS
//Metodo de Horner. 
//preenche vetor tab_hash com -1
void preenche_tab_hash(){
	int i;
	for(i=0;i<TAM_vocabulario_colecao;i++){
		tab_hash[i]=-1;
	}
}
//verificacao da chave
int verifica_chave(char cadeia[], int h, int tam){
	int i;
	for(i=0;i<TAM_vocabulario_colecao;i++){

	if(strcmp(cadeia,vocabulario_colecao[i].termo)==0){//se já existe o termo
	return (i&0xFFFFFFF)%tam;///para resultados positivos sempre ---tratamento overflow
	break;
	}
   }
	    if(tab_hash[h]==-1){//se nao existe termo no endereco
		tab_hash[h]=tab_hash[h]+1;
		return (h&0xFFFFFFF)%tam;///para resultados positivos sempre ---tramento overflow
	    }  
	    else{//caso já esteja ocupado e seja um termo diferente 
	    tab_hash[h]=tab_hash[h]+1;
		return hash2(cadeia, h, tam, tab_hash[h]);
		}
	}  
//Transforma uma string em um indice no intervalo 0..TAM_vocabulario_colecao-1.
int hash(char cadeia[],int tam) 
{ int i, j, h=cadeia[0];
for(i=1;cadeia[i]!='\0';i++){
	h=(h*251+cadeia[i])%tam; //0 a 255 são os caracteres disponiveis ---251 é o melhor pois é primo
    }
    return verifica_chave(cadeia, h, tam);
}
//sondagem Duplo Hash
int hash2(char cadeia[],int h1, int tam, int t)//int t é a quantidade de tentativas 
{ int j=0,i,h2=h1%(tam-1)+1;
	h1=(h1+h2*t)%tam; 
	if(h1<TAM_vocabulario_colecao)
	return (h1&0xFFFFFFF)%tam;///para resultados positivos sempre ---tratamento overflow	
}		

//Retorna a posicao de um termo, se não existe retorna -1
int proc_hash_colecao(char termo[]) 
{ int i=0,end,pos=hash(termo,TAM_vocabulario_colecao);
  end=(pos+i)%TAM_vocabulario_colecao;
  while( (vocabulario_colecao[end].num_docs!=0) && (i<TAM_vocabulario_colecao) )
      { if(vocabulario_colecao[end].termo==termo)
          break;
        i++;
        end=(pos+i)%TAM_vocabulario_colecao;
      }
  if(vocabulario_colecao[end].termo==termo)
          return end;
     else return -1;
}
//FUNCOES DA LISTA_INVERTIDA
//INICIALIZA A LISTA
void inicializa_lista_invertida(){
	int i=0;///termo 0 existe 
	while(i<TAM_vocabulario_colecao){
		int j=0;//enquanto doc inicia do 1
		while(j<TOT_COLECAO){
			lista_invertida[i][j]=0;
			j++;
			}
		i++;	
	}
}
//CHECK O TERMO PARA LISTA
void check_termo(int doc, int end){///serve para gerar a frequencia do termo y no doc x
	if(end!=-1){
	lista_invertida[end][doc]++;
	vocabulario_colecao[end].num_docs++;//somando total de aparições do termo nos docs
   }
}
//IMPRIME LISTA INVERTIDA - ESSA LISTA FICA NO FORMATO: (ENDERECO-HASH TF1  TF2  TF3 ...TFN)
void imprime_lista_invertida(){
	int i=0, total=0;
	FILE *arq;
	arq= fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\02Lista Invertida 001\\Lista\\lista_invertida.txt", "wt");
	
	if(!arq){
		printf("\nArquivo lista_invertida não pode ser aberto\n");
		exit(3);
	}
	else{
		while(i<TAM_vocabulario_colecao){
	//	if(vocabulario_colecao[i].num_docs!=0){ //somente em casos onde nao seja necessario imprimi posicoes do vertor hash vazios
			fprintf(arq,"%d",i);//imprimindo endereco do termo seguida da lista invertida
			imprime_lista_termo(arq,i);
			fprintf(arq,"\n");
			total++;
	    //	}
	    	i++;
		}
		fprintf(arq,"\nTotal de chaves:%d\n", total);
	}
	fclose(arq);
}
void imprime_lista_termo(FILE *file, int i){
	int j=1;
	while(j<TOT_COLECAO){
		fprintf(file," %d",lista_invertida[i][j]);//imprimindo lista invertida do endereco do termo i, sendo que linha é termo e sua freq tf e coluna é doc
		j++;
	}
}
/*//IMPRIME LISTA INVERTIDA - ESSA LISTA FICA NO FORMATO: (ENDERECO-HASH TERMO: {(DOC1;TF1);...(DOCN;TFN)}
void imprime_lista_invertida_comp(){
	int i=0, total=0;
	FILE *arq;
	arq= fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\02Lista Invertida 001\\Lista\\lista_invertida_total.txt", "wt");
	
	if(!arq){
		printf("\nArquivo lista_invertida não pode ser aberto\n");
		exit(3);
	}
	else{
		while(i<TAM_vocabulario_colecao){
	//	if(vocabulario_colecao[i].num_docs!=0){ //somente em casos onde nao seja necessario imprimi posicoes do vertor hash vazios
			fprintf(arq,"%d -%s :{",i, vocabulario_colecao[i].termo);//imprimindo endereco do termo e termo seguida da lista invertida
			imprime_lista_termo_comp(arq,i);
			fprintf(arq,"}\n");
			total++;
	    //	}
	    	i++;
		}
		fprintf(arq,"\nTotal de chaves:%d\n", total);
	}
	fclose(arq);
}
void imprime_lista_termo_comp(FILE *file, int i){
	int j=1;
	while(j<TOT_COLECAO){
		fprintf(file,"(%d;%d) ",j,lista_invertida[i][j]);//imprimindo lista invertida do endereco do termo i, j é o doc.
		j++;
	}
}*/
void iniciar_colecao()
{ int i;
  for(i=0;i<TAM_vocabulario_colecao;i++)
   { vocabulario_colecao[i].num_docs=0;
   }
}


void gera_vocabulario_colecao()//mudar para abrir o proprio arquivo
{ 
  int doc,t,tam;
  int i=0;
  int end;
  for(doc=0;doc<TOT_COLECAO;doc++) ///percorre coleção MUDAR PARA O PROPRIO DOC TXT
   { t=0; tam=strlen(dados[doc]);
     while(t<tam){
     	char termo[62];
     if(dados[doc][t]!=' '){ ///caracter por caracter
     	   termo[i]=dados[doc][t];
		   i++;
		   t++;
		   if(t>tam-1){//termina doc
		   	    termo[i]='\0';
                i=0;
				t=0;
				end=hash(termo,TAM_vocabulario_colecao);
				check_termo(doc+1,end); 
				break;
			}
		   }
		   if(dados[doc][t]==' '||dados[doc][t]=='\n'){///quando encontra um espaço
            termo[i]='\0';
            i=0;
            t++;
            end=hash(termo, TAM_vocabulario_colecao);
			check_termo(doc+1,end); 
			}
						        
		 }
       }
   }
                          
////FUNÇOES PARA PERCORRER ARQUIVOS E DIRETORIO
 percorre_arquivo(FILE *file, int num){///converter txt em caracteres de um vetor ---recebe arquivo e o end do documento
	 int i=0, j, r=0;
	 char ch;
	 char vetor[TAM_DOC];
	 if(file==NULL){ // verificacao se existe
	 	printf("\nArquivo não existe---\n");
	 }
	 else{
	  while((ch=fgetc(file))!= EOF){
		 if(ch!=' '||ch!='\n'){///sendo caracter
		  if(isalpha(ch)){///sendo o caracter letra
		     ch=tolower(ch); // torna todos os caracteres minusculos
		  	 vetor[i]=ch;
		  	 i++;
	  }
			if(!isalpha(ch)){
				vetor[i]=' ';
				i++;
			}  
		 }
		 if(ch==' '||ch=='\n'){///sendo o ch um espaco ou ainda uma quebra de linha
			 vetor[i]=' ';
		      i++;
		     }
	   	   }
	   }
   	 for(j=0;j<i;j++){
     dados[num][j]=vetor[j];//ocupa a string que contem todos os documentos na ordem 0..TOT_COLECAO
     }
}
 ///DIRETORIO
abrir_diretorio(DIR *dir){
   int i=0, j;
  
    while ( ( lsdir = readdir(dir) ) != NULL ) ///escrevendo em arquivo txt o diretorio
     {
     	char diretorio[1000]="C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Arquivos\\";
     	if(strcmp(lsdir->d_name,".")!=0&&strcmp(lsdir->d_name,"..")!=0){
     	strcat(diretorio,lsdir->d_name);
     	
       	FILE *arq;
        arq =fopen(diretorio, "r");
        
        printf("\nlsddir:%s \n arq:%s\n  strcat:%s\n",lsdir->d_name, arq,diretorio);
  
       if(arq==NULL){ //verificaçao se existe arquivo
	 	 printf("\nArquivo Diretorio não existe---\n");
		 }
	   else{
	   	percorre_arquivo(arq, i);
        i++;
	    fclose(arq);
	   }
      }
     }
   }
void vocabulario(FILE *file){///arquivo vocabulario gerado pelo programa passado
char termo[62];
int i=0;
if(!file){
	printf("Arquivo Vocabulario.txt ERRO");
	exit(3);
}
else{
	while((fscanf(file,"%d %s\n",&i,&termo))!=EOF){
		int j=0;
		while(j<(strlen(termo)+1)){
		vocabulario_colecao[i].termo[j]=termo[j];
		if(j==strlen(termo)){
			vocabulario_colecao[i].termo[j]='\0';
		 }
		j++;
	    }
	}
 }
}

/*
OBS:

1.PODE-SE MELHORAR COLOCANDO A FUNÇAO DE GERAR VOCABULARIO DIRETAMENTE COM OS ARQUIVOS TXT
MAS ISSO DEMANDARÁ TEMPO MAIOR DE EXECUÇÃO; ATÉ O MOMENTO OS ARQUIVOS SÃO PASSADOS A UMA 
FUNÇÃO QUE OS CONVERTE EM VETOR E AÍ SIM SÃO GERADOS VOCÁBULOS;

2.TESTE COM DOCUMENTOS TXT SUPERIORES A 1 MB NÃO FORAM REALIZADOS

*/
int main()
{
  system("clear");
  setlocale(LC_ALL, "Portuguese");
  
  printf("Palavra a ser consultada \n");
  scanf("%s", consulta);
  printf("%s Palavra consultada \n", consulta);
  
inicializa_lista_invertida();

  FILE *file;  // Vocabulario
  file = fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Vocabulario\\Vocabulário.txt", "rt");
  
  DIR *dir;         //diretorio
  dir = opendir("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Arquivos");
  
  abrir_diretorio(dir);

  iniciar_colecao();
  vocabulario(file);
  gera_vocabulario_colecao();
 
  imprime_lista_invertida(); 
    
  fclose(file);
  closedir(dir);
  printf("\nSucesso\n");
  return 0;
}
