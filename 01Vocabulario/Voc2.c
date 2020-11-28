#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<dirent.h>
#include <locale.h>

#define TOT_COLECAO 230 // docs 
#define TAM_VOC_CONS 3  //Hashing
#define TAM_CONS 9     //Qtd elementos da consulta
#define TAM_vocabulario_colecao 900000// qtd vocabulos
#define TAM_DOC 2000000

char dados[TOT_COLECAO][TAM_DOC];//matriz de documentos
char consulta[TAM_CONS+1];
int tab_hash[TAM_vocabulario_colecao];

typedef struct cel 
{ int    id_doc;
  int    tf;
  struct cel *prox;
}celula;     

typedef struct
{ char   termo[200];
  float  idf;
  int    num_docs;
  celula *lista_inver;
}vocabulo;

vocabulo vocabulario_colecao[TAM_vocabulario_colecao];

typedef struct
{ char  termo[200];
  int   tf;
  float peso;
}voc_cons;

voc_cons voc_consulta[TAM_VOC_CONS];

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
// numero de termos da colecao nao e maior do que TAM_vocabulario_colecao
void ins_hash_colecao(int doc,char termo[]) 
{ int i=0, j,end,pos=hash(termo,TAM_vocabulario_colecao);
  end=(pos+i)%TAM_vocabulario_colecao;  
      while( (vocabulario_colecao[end].num_docs!=0) && (i<TAM_vocabulario_colecao) )
      { 
	  if(strcmp(vocabulario_colecao[end].termo,termo)==0){//compara termos já existentes
		  break;
       }
        i++;
        end=(pos+i)%TAM_vocabulario_colecao;
      }
  if(i>TAM_vocabulario_colecao-1)
       printf("TABELA HASH CHEIA- VOCABULOS\n");
    else { if(vocabulario_colecao[end].termo!=termo){
             vocabulario_colecao[end].num_docs++;
         }
         for(j=0;termo[j]!='\0';j++){
         	if(termo[j]!=' '){
         vocabulario_colecao[end].termo[j]=termo[j]; 
           }
		 }
		 printf("\nTermo{%d} Vocábulo{%s}\n", end, vocabulario_colecao[end].termo);
         }
	}
		
// numero de termos nao e maior do que TAM_VOC_CONS
void ins_hash_consulta(char termo[]) 
{ int i=0,end,h=hash(termo,TAM_VOC_CONS);
  end=(h+i)%TAM_VOC_CONS;
  while( (voc_consulta[i].tf!=0) && (i<TAM_VOC_CONS) )
      { if(voc_consulta[end].termo==termo)
          break;
        i++;
        end=(h+i)%TAM_VOC_CONS;
      }
  if(i>TAM_VOC_CONS-1)
       printf("TABELA HASH CHEIA - CONSULTA\n");
    else {
	      /// voc_consulta[end].termo=termo;//resolve com for
           voc_consulta[end].tf++;
         }
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

void iniciar_colecao()
{ int i;
  for(i=0;i<TAM_vocabulario_colecao;i++)
   { vocabulario_colecao[i].num_docs=0;
     vocabulario_colecao[i].lista_inver=NULL;
   }
}

void inicia_consulta()
{ int i;
  for(i=0;i<TAM_VOC_CONS;i++)
   { voc_consulta[i].tf=0;
     voc_consulta[i].peso=0;
   }
}

void ver_hash_colecao() 
{ int i,total=0;
  FILE *arq;
  arq=fopen("./ver_hash_colecao.txt","wt");   
  if(!arq)
   { printf("\nnao foi possivel abrir o arquivo: ./ver_hash_colecao.txt\n\n");
     exit(3);
   }
  for(i=0;i<TAM_vocabulario_colecao;i++)
   if(vocabulario_colecao[i].num_docs!=0)
     { fprintf(arq,"Pos:%d Termo:%c NDocs:%d  { ",i,vocabulario_colecao[i].termo,vocabulario_colecao[i].num_docs);
        //imprime_lista(arq,vocabulario_colecao[i].lista_inver);
        fprintf(arq,"}\n");
       total++;
     }
  fprintf(arq,"\nTotal de chaves:%d\n",total);
  fclose(arq);
}

void gera_vocabulario_colecao()//mudar para abrir o proprio arquivo
{ 
  int doc,t,tam;
  int i=0;
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
				ins_hash_colecao(doc+1,termo); 
				break;
			}
		   }
		   if(dados[doc][t]==' '||dados[doc][t]=='\n'){///quando encontra um espaço
            termo[i]='\0';
            i=0;
            t++;
            ins_hash_colecao(doc+1,termo);
		/*	for(r=1;dados[doc][t]=' ';r++){
				t++;
		    	} */
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
   int i=1, j;
  
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
//FUNÇÃO PARA COMPARAR VOCABULARIO
int comparar_voc(char termo[]){	
	char voc[TAM_DOC];
	FILE *arq;
	arq=fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Vocabulario\\Vocabulário.txt", "rt");
	if(arq==NULL){ // verificacao se existe
	 	printf("\nArquivo não existe---\n");
	 }
	 else{
	    while( fgets(voc, sizeof(voc), (arq))!=NULL ){
			if(termo==voc){ //sendo termo encontrado
				return 1;
				break;
			  }
			}
	} 
    fclose(arq);
}

//FUNÇÃO PARA IMPRIMIR NO ARQUIVO VOCABULARIO
void imprimir_voc(){
		int i;
		FILE *arq;
		arq=fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Vocabulario\\Vocabulário.txt", "wt");
		if(arq==NULL){ // verificacao se existe
	 	printf("\nArquivo não existe aqui imprimir Vocabulario.txt---\n");
	    }
	    else{
		 for(i=0;i<TAM_vocabulario_colecao;i++){
			if(strcmp(vocabulario_colecao[i].termo, "")!=0){
				if(comparar_voc(vocabulario_colecao[i].termo)!=1){
		    	fprintf(arq,"%d %s\n", i, vocabulario_colecao[i].termo);
		     	printf("termo impresso: %s\n", vocabulario_colecao[i].termo);
		       }
	         }
         	}
		fclose(arq);
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
  
  preenche_tab_hash();
  //executa_consulta();
  
  DIR *dir;         //diretorio
  dir = opendir("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Arquivos");
  
  abrir_diretorio(dir);

  iniciar_colecao();
  gera_vocabulario_colecao();
  imprimir_voc();
  
  closedir(dir);
  printf("\nSucesso\n");
  return 0;
}
