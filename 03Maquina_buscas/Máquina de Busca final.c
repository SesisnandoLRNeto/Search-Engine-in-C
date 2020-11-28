#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<dirent.h>
#include<locale.h>
#include<ctype.h>

#define TOT_COLECAO 220 // docs 
#define TAM_VOC_CONS 14  //Hashing
#define TAM_CONS 64     //Qtd elementos da consulta
#define TAM_vocabulario_colecao 900000// qtd vocabulos
 
char consulta[TAM_CONS+1];
int lista_invertida[TAM_vocabulario_colecao][TOT_COLECAO];

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

voc_cons voc_consulta[TAM_vocabulario_colecao];

float peso_termo_colecao[TAM_vocabulario_colecao][TOT_COLECAO];

float norma_doc[TOT_COLECAO];

float norma_cons;

struct dirent *lsdir;//diretorio para usar a funcao lsdir

 //FUNCOES DOS HASHS
//Metodo de Horner. 
//verificacao da chave
int verifica_chave(char cadeia[], int h, int tam){
	int i;
	for(i=0;i<TAM_vocabulario_colecao;i++){

	if(strcmp(cadeia,vocabulario_colecao[i].termo)==0){//se já existe o termo
	return (i&0xFFFFFFF)%tam;///para resultados positivos sempre ---tratamento overflow
	break;
	}
   }
   if(strcmp(cadeia,vocabulario_colecao[i].termo)!=0){
	return -1;
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
{ int i=0,end=hash(termo,TAM_vocabulario_colecao);
  while( (vocabulario_colecao[end].num_docs!=0) && (i<TAM_vocabulario_colecao) )
      { if(vocabulario_colecao[end].termo==termo)
          break;
        i++;
      }
  if(vocabulario_colecao[end].termo==termo)
          return end;
     else return -1;
}
// numero de termos nao e maior do que TAM_VOC_CONS
void ins_hash_consulta(char termo[]) 
{ int i=0, j,end=hash(termo,TAM_vocabulario_colecao);
  while( (voc_consulta[i].tf!=0) && (i<TAM_VOC_CONS) )
      { if(strcmp(voc_consulta[end].termo,termo)==0){//compara termos já existentes
		  break;
       }
        i++;
      }
  if(i>TAM_VOC_CONS-1)
       printf("TABELA HASH CHEIA - CONSULTA\n");
    else {
    	  	
	      for(j=0;termo[j]!='\0';j++){
         	if(termo[j]!=' '){    	
	       voc_consulta[end].termo[j]=termo[j];
           } 
		}
		 voc_consulta[end].tf++;
		 if(vocabulario_colecao[end].num_docs==0){///armazenar em cache o termo já consultado
		 ler_lista_invertida(end);
	   }
	  printf("TERMO:%s  END:%d  TF:%d\n", voc_consulta[end].termo,end, voc_consulta[end].tf); ///mudar para o vetor hash já existente
      }
} 
void gera_vocabulario_consulta(char consulta[])//mudar para abrir o proprio arquivo
{ 
  int t,tam;
  int i=0;
  int end;
  t=0; tam=strlen(consulta);
     while(t<tam){
     	char termo[62];
     if(consulta[t]!=' '){ ///caracter por caracter
     	   consulta[t]=tolower(consulta[t]);//evitar que o usuario digite Maiusculas
     	   termo[i]=consulta[t];
		   i++;
		   t++;
		   if(t>tam-1){//termina consulta
		   	    termo[i]='\0';
                i=0;
				t=0;
				ins_hash_consulta(termo);
				break;
			}
		   }
		   if(consulta[t]==' '||consulta[t]=='\n'){///quando encontra um espaço
            termo[i]='\0';
            i=0;
            t++;
            ins_hash_consulta(termo);
			}			        
	  }
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
//LER A LISTA INVERTIDA
void ler_lista_invertida(int pos){///arquivo lista_invertida gerado pelo programa passado
char linha[TOT_COLECAO*100];
FILE *arq; // Lista invertida
arq = fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\02Lista Invertida 001\\Lista\\lista_invertida.txt", "rt");

if(!arq){
	printf("Arquivo lista_invertida.txt ERRO");
	exit(3);
}
else{
	while(!feof(arq)){
		fgets(linha,sizeof(linha),arq);
		int i[TOT_COLECAO+1];
		int cont=0, doc=0, line=0;
		char *prt = strtok(linha, " ");
		line=atoi(prt);
	
		if(line==pos){
		   while(prt!=NULL){
			prt=strtok(NULL, " ");
			i[cont]=atoi(prt);//converter a string para inteiro
			lista_invertida[line][doc]=i[cont];
			if(i[cont]!=0){//contando o IDF
				vocabulario_colecao[line].num_docs++;
			}
			cont++;
			doc++;
	    	}
	    	break;
	     }
	   }
	}
	fclose(arq);
}
//INICIA A COLECAO	
void iniciar_colecao()
{ int i;
  for(i=0;i<TAM_vocabulario_colecao;i++)
   { vocabulario_colecao[i].num_docs=0;
     vocabulario_colecao[i].lista_inver=NULL;
   }
}
//INICIA A CONSULTA
void inicia_consulta()
{ int i;
  for(i=0;i<TAM_VOC_CONS;i++)
   { voc_consulta[i].tf=0;
     voc_consulta[i].peso=0;
   }
} 
//FUNCOES PROCESSAMENTO DA CONSULTA 
void executa_consulta(void)
{ int i, j,pal,doc,tot,*resultado, cont=1;
  float acumulador[TOT_COLECAO], similaridade[TOT_COLECAO], acum;
// CALCULA PESO DA CONSULTA
  printf("Peso dos Termos - Consulta-----------------------------------------------------------------------------\n");
  
  for(j=0;j<TAM_vocabulario_colecao;j++){
   if(voc_consulta[j].tf!=0){
        voc_consulta[j].peso = voc_consulta[j].tf * vocabulario_colecao[j].idf;
        printf("{TERMO-CONSULTA:%s TF:%d PESO-CONSULTA:%5.2f}\n ",vocabulario_colecao[j].termo, voc_consulta[j].tf,voc_consulta[j].peso);
       }
   }
  printf("\n\n");
     // inicializa
  for(j=0;j<TOT_COLECAO;j++)     
     { acumulador[j]=0;
       similaridade[j]=0;
     }
     //CALCULA NORMA DA CONSULTA
  printf("\nNorma da Consulta------------------------------------------------------------------------------------\n");
      for(i=0;i<TAM_vocabulario_colecao;i++){ //termos
        if(voc_consulta[i].tf!=0){
          acum += powf(voc_consulta[i].peso,2); // ao quadrado
        }
	  }
	  norma_cons= sqrt(acum); // raiz quadrada
      printf("Norma Consulta=%5.2f \n",norma_cons);
  printf("\n\n");
  
    //CALCULA OS ACUMULADORES
  printf("Acumuladores-------------------------------------------------------------------------------------------\n");  ///verificar acumuladores
  for(i=0;i<TAM_vocabulario_colecao;i++){
    if(voc_consulta[i].tf!=0){
     for(j=0;j<TOT_COLECAO;j++){
     	printf("peso do termo %5.2f vocabulario peso%5.2f\n", peso_termo_colecao[i][j], voc_consulta[i].peso);
      acumulador[j]	 += peso_termo_colecao[i][j] * voc_consulta[i].peso;
        }
      }
     }
  for(doc=0;doc<TOT_COLECAO;doc++){
    if(acumulador[doc]!=0){
      printf("Acumuladores:{D%d=%5.2f} \n",doc+1,acumulador[doc]);
    }
   }
  printf("\n\n");

    //CALCULA SIMILARIDADE
 printf("Similaridade-------------------------------------------------------------------------------------------\n");
  tot=0;
  for(doc=0;doc<TOT_COLECAO;doc++){
    if(acumulador[doc]!=0){
	    tot++;
        if(norma_doc[doc]!=0){ 
		  similaridade[doc]= acumulador[doc] / (norma_doc[doc]*norma_cons); 
		  if(similaridade[doc]==1){ ///Resolver o problema de consultas com uma única palavra;
		  	similaridade[doc]=(acumulador[doc]/100);
		  }
		  printf("sim %5.5f , acumulador %5.5f, norma doc %5.5f, norma cons %5.5f, doc %d \n", similaridade[doc], acumulador[doc], norma_doc[doc] ,norma_cons, doc);
          printf("{D%d=%5.2f%%}\n",doc+1,similaridade[doc]*100);
          }
      }   
   }
printf("\n\n");

//RANKING SIMILARIDADE
float maior[TOT_COLECAO], aux;
int vet[TOT_COLECAO], c=0, rank;

maior[0]=similaridade[0];

for(i=0;i<TOT_COLECAO;i++){
	for(j=i+1;j<TOT_COLECAO;j++){
		 		if(similaridade[i]<similaridade[j]){
					aux=similaridade[i];
					vet[i]=j;
					similaridade[i]=similaridade[j];
					similaridade[j]=0;
	   }
      } 
}
printf("\n -------------------------R----E----S---U---L---T---A---D----O--------------------------------------- \n\n");
printf("_______________________________________________________________________________________________________\n");
for(i=0;i<10;i++){
	printf("RANK:%d - %5.5f%% \n",i+1, similaridade[i]*100);
	if(similaridade[i]!=0){
	 nome_doc(vet[i]+1);
     }printf("_______________________________________________________________________________________________________\n");
     
	}
/*	printf("\n\nCaso queira abrir o arquivo, digite a opção de acordo com o RANK do documento (1,2..10)\n\n");
	scanf("%d",&rank);
//ABRIR ARQUIVO AO USUARIO
printf("\nImprime rank: %d\n", rank);
system();
*/
 }

// CALCULA O IDF DA COLECAO
void calcula_idf()          
{ int i,qtd=0;
  float tmp;
  for(i=0;i<TAM_vocabulario_colecao;i++)
   if(vocabulario_colecao[i].num_docs!=0)
     qtd++;
   
printf("QtdReal:%d\n",qtd);   
      
  printf("\nIDF dos Termos - Vocabulario-------------------------------------------------------------------------\n");
  for(i=0;i<TAM_vocabulario_colecao;i++){
   if(voc_consulta[i].tf!=0){
        printf("Num_docs:%d\n", vocabulario_colecao[i].num_docs);
        vocabulario_colecao[i].idf=log((float)TOT_COLECAO/vocabulario_colecao[i].num_docs);
        printf("{%s=%5.4f}\n ",vocabulario_colecao[i].termo,vocabulario_colecao[i].idf);
       }
   else{
   vocabulario_colecao[i].idf=0;
   }
  }
   printf("\n");
}
// Calcula o peso dos termos na colecao
void calcula_peso_colecao()          
{ int i,j;
  for(i=0;i<TAM_vocabulario_colecao;i++){
    for(j=0;j<TOT_COLECAO;j++){
      peso_termo_colecao[i][j]=0;
  }
}
  printf("\nPeso dos Termos Vocabulario--------------------------------------------------------------------------\n");
  for(i=0;i<TAM_vocabulario_colecao;i++){
    if(vocabulario_colecao[i].num_docs!=0){
           for(j=0;j<TOT_COLECAO;j++){
            peso_termo_colecao[i][j]=lista_invertida[i][j]*vocabulario_colecao[i].idf; ///TF * IDF
           }    
    }
}
  for(j=0;j<TOT_COLECAO;j++)
   { printf("NumDoc:%d  ", j+1);
     for(i=0;i<TAM_vocabulario_colecao;i++)
       if(peso_termo_colecao[i][j]!=0){
       //  printf("{%s=%5.2f}\n",vocabulario_colecao[i].termo,peso_termo_colecao[i][j]);
       }
     printf("\n");
   }
}
//CALCULA NORMA DOS DOCS
void calcula_norma_colecao()
{ int i,j;
  float acum, acumula;
  printf("\nNorma dos Documentos---------------------------------------------------------------------------------\n");
  for(j=0;j<TOT_COLECAO;j++) //documentos
    { acum=0;
      for(i=0;i<TAM_vocabulario_colecao;i++){ //termos
        if(peso_termo_colecao[i][j]!=0){
          acum += peso_termo_colecao[i][j]; //acumulando pesos no documentos
        }
	  }
	  acumula=powf(acum, 2);//ao quadrado
      norma_doc[j]= sqrt(acumula); // raiz quadrada
      printf("Doc%d=%5.2f \n",j+1,norma_doc[j]);
    }
  printf("\n\n");
}

//FUNCAO PARA LER ARQUIVO VOCABULARIO.TXT
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
//GERA O NOME DO ARQUIVO
nome_doc(int doc){
char linha[1000];
int cont=1;
FILE *arq; // Lista dos documentos me ordem
arq = fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\03Máquina de Buscas (Sim, Acum, Peso, Norma e Ranking)\\documentos.txt", "rt");

if(!arq){
	printf("Arquivo documentos.txt ERRO");
	exit(3);
}
else{
	while(!feof(arq)){
		fgets(linha,sizeof(linha),arq);
		if(cont==doc){
		printf("%s\n",linha);
		break;
	     }
	     cont++;
	   }
     }
	fclose(arq);
}
/*
OBS:
*/
int main()
{
  char consulta[100];
  system("clear");
  setlocale(LC_ALL, "Portuguese");
  
  //PRE PROCESSOS
  FILE *file;  // Vocabulario
  file = fopen("C:\\Users\\Sesisnando\\Desktop\\Maquina de Buscas\\Máquina de Buscas\\01Vocabulario 002\\Vocabulario\\Vocabulário.txt", "rt");
  
  iniciar_colecao();
  inicializa_lista_invertida();
  vocabulario(file);

  //MENU----
  
    int continuar=1;

   /* do
    {
        
        printf("\n(Digite a sua opção)\n");
        printf("1. Fazer Consulta\n");
        printf("0. Sair\n");

        scanf("%d", &continuar);
        system("cls || clear");
      
           */
             //POS PROCESSOS
            fclose(file);
/*
       switch(continuar)
        {
    
            case 1:*/ 
            //CONSULTA
            consulta:
            system("cls || clear");
            printf("\n\t\t\t\t Máquina de Buscas - Documentos Históricos da Amazonia\n\n");
            printf("-------------------------------------------------------------------------------------------------------\n");
            printf("Digite o termo a ser consultado \n");
            scanf("%[^\n]", &consulta);
  			
            //PROCESSOS DA CONSULTA
           gera_vocabulario_consulta(consulta);
           calcula_idf(); 
           calcula_peso_colecao();
           calcula_norma_colecao();
           executa_consulta();
           
           printf("\n\t\t\t\t Fazer nova consulta? - Digite : 0 (Zero)\n");
           printf("\n\t\t\t\t Deseja sair do programa? - Digite Qualquer tecka\n");
           scanf("%d[^\n]",&continuar);
		   if(continuar==0)
           {
           	fflush(stdin);
           	goto consulta;
           	clear();
		   }
		   system("exit");
		    	
  /*          case 0:
                //sair();
                break;

            default:
                printf("Digite uma opcao valida\n");
        }
    } while(continuar);
  */
  //system("start "" /max "C:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\Reader\\AcroRd32.exe" A/ "C:\\Users\Sesisnando\\Desktop\\Maquina de Buscas\\Biblioteca Digital - TCC\\Pdf\\01 Cronograma de Itacoatiara.pdf"");
//  return 0;
}
