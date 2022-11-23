#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <ctype.h>

//includes de mapped files
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

//includes de named pipes
#include <linux/stat.h>

//include semaforos e threads
#include <pthread.h>
#include <semaphore.h>

#define FIFO_FILE "statistics"
#define MAXLINHA 50
#define MAXDOMAINS 5
#define HEADER_H_INCLUDED

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAP_FILE    0

//PID'S E MEMORIA PARTILHADA

int shmid_config;
int *shmat_config;
int shmid_sem_config;

//CHAMADA FUNCOES
 
void convertName2RFC (unsigned char*,unsigned char*);
unsigned char* convertRFC2Name (unsigned char*,unsigned char*,int*);
void sendReply(unsigned short, unsigned char*, int, int, struct sockaddr_in);
int init();
int server(int argc , char *argv[]);
int config();
void init_semaphore();

// VARIAVEIS GLOBAIS
int fd;
int sockfd_close;
int *flag_sinal;
struct stat size;
char *line;

//SEMAFOROS E THREADS

//sem_t *sem_mutex; //usado para ler as configuraçoes
sem_t sem_mutex;
sem_t sem_testa;
sem_t *sem_config;

/************/
/*Estruturas*/
/************/

//DNS header structure

struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
 
//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
//Structure of a Query
struct QUERY
{
    unsigned char *name;
    struct QUESTION *ques;
};

//ESTRUTURA TIRADA DA NET DA SOCKADDR_IN
/*
struct sockaddr_in { 
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*/ 


/*Lista e estrutura de configuracao*/

typedef struct lnode *List_config;
typedef struct lnode{
    char domain[50];
    List_config next;
}Domains;

typedef struct{
    int num_threads;
    char domains[MAXDOMAINS][MAXDOMAINS];
    char local_domain[MAXLINHA];
    char named_pipe[20];
}lista_config;


/*Estrutura de estatistica*/

typedef struct{
    //int hora_inicio, mins_inicio;
    char hora_inicio[MAXLINHA];
    int n_total_pedidos;
    int n_pedidos_recusados;
    int n_enderecos_l_resolvidos;
    int n_enderecos_e_resolvidos;
    char *hora_ultima_informacao;
}lista_estatistic;


/*Lista e estrutura dos pedidos*/

typedef struct{
    //dados do pedido
    struct sockaddr_in dest; 
    char name[100];
    int sockfd;
    unsigned short id;
}Dados;

typedef struct pnode *List_domains;
typedef struct pnode{
    Dados dados_pedido;
    List_domains next;
}ListDomains;
/*    Id das threads*/

pthread_t* threads;
int* id_threads;


/*Semaforos*/

sem_t* config_mutex, *estat_mutex;

/*Funcoes config*/
lista_config * mempart;
lista_estatistic * estatisticas;
List_domains lista_d_prio;//lista dos pedidos locais
List_domains lista_d_nor; // lista dos pedidos normais (externos)

List_domains cria_lista_domains();
void insereLista(char domain[50]);
void insereListaDomains(List_domains lista,char name[],struct sockaddr_in dest,int id,int sockfd);
Dados retiraPrimeiroLista(List_domains lista);
void imprime_lista();
void coloca_listanull();
int le_configuracoes();
int le_config_verifica();
void alterar_config();
void alterar_configs();

/*Funcoes estatistic*/

void estatistic();
int inicializa_lista_estatisticas();
void writeHourPresent(char* time_string);
int escreve_data_ultima_informacao();
void escreve_estatisticas();
void criar_named_pipe();
void escreve_pipe();
void le_pipe(int sign);

/*Funcoes pedidos*/


int acrescenta_localdns();
void catch_toclose(int sign);
void catch_SIGUSR1(int sign);
//void clean_sem_mem();
void cleanup();
//void cleanup(int signo);

//int get_stat(int fdin);
void mmap_localdns();
char* retira_IP_local(char nome[]);
int verifica_dominio_local(char pedido[]);
int verifica_dominio(char pedido[], List_config lista_domains);

void create_thread_pool(int n_threads);
int empty_lista(ListDomains *lista);
char* execexterno(char *pedido);
void *pedidos(void*);
int validateRequest(char *name);
void gen_statistics();