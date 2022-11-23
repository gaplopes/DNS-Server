#include "header.h"

//PID'S E MEMORIA PARTILHADA

pid_t pid_statistic;
pid_t pid_pedidos;
pid_t pid_configuracoes;
int shmid_config;
int *shmat_config;
int shmid_sem_config;
int shmid_flag;

// VARIAVEIS GLOBAIS
int fd;
int sockfd_close;
int *flag_sinal;

pthread_mutex_t th_mutex = PTHREAD_MUTEX_INITIALIZER; //usada para ler as configuraçoes


int main(int argc , char *argv[]){
	pid_pedidos = getpid();
	printf("PID PROCESSO PEDIDOS = %d\n", getpid());
	
	init();

	signal(SIGINT, catch_toclose);

	server(argc ,argv);

	return 0;
}


int server(int argc , char *argv[]){
	
	lista_d_nor = cria_lista_domains();
    lista_d_prio = cria_lista_domains();
    
	unsigned char buf[65536], *reader;
	int sockfd, stop;
	struct DNS_HEADER *dns = NULL;
	
	struct sockaddr_in servaddr, dest;
	socklen_t len;
	// Check arguments
	char aux[40];
	int i;
	if(argc <= 1) {
		printf("Usage: dnsserver <port>\n");
		exit(1);
	}
	
	// Get server UDP port number
	int port = atoi(argv[1]);
	
	// Get server UDP port number
	
	if(port <= 0) { //verifica se a porta é valida, ou seja maior ou igual a 0
		printf("Usage: dnsserver <port>\n");
		exit(1);
	}
	
	
	// ****************************************
	// Create socket & bind
	// ****************************************
	
	// Create UDP socket
    sockfd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
    sockfd_close = sockfd;
 
	if (sockfd < 0) { //Erro ao criar a socket se o inteiro for menor que 0
         printf("ERROR opening socket.\n");
		 exit(1);
	}

	// Prepare UDP to bind port
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(port);
	
	// Bind application to UDP port
	int res = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	if(res < 0) {
         printf("Error binding to port %d.\n", servaddr.sin_port); //erro ao ligar a aplicaçao ao UDP
		 
		 if(servaddr.sin_port <= 1024) {
			 printf("To use ports below 1024 you may need additional permitions. Try to use a port higher than 1024.\n");
		 } else {
			 printf("Please make sure this UDP port is not being used.\n");
		 }
		 exit(1);
	}
	
	// ****************************************
	// Receive questions
	// ****************************************
	
	while(1) {
		
		//escreve_estatisticas(); // imprime estatisticas
		
		// Receive questions
		len = sizeof(dest);
		printf("\n\n-- Wating for DNS message --\n\n");
		if(recvfrom (sockfd,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , &len) < 0) {
			//The recvfrom() function receives a message from a connection-mode or connectionless-mode socket
			printf("Error while waiting for DNS message. Exiting...\n");
			exit(1);
		}
		
		(estatisticas->n_total_pedidos)++; //incrementa o numero total de pedidos

		
		printf("DNS message received\n");
	 
		// Process received message
		dns = (struct DNS_HEADER*) buf;
		//qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
		reader = &buf[sizeof(struct DNS_HEADER)];
	 
		printf("\n The query %d contains: ", ntohs(dns->id));
		printf("\n %d Questions.",ntohs(dns->q_count));
		printf("\n %d Answers.",ntohs(dns->ans_count));
		printf("\n %d Authoritative Servers.",ntohs(dns->auth_count));
		printf("\n %d Additional records.\n\n",ntohs(dns->add_count));
		// We only need to process the questions
		// We only process DNS messages with one question
		// Get the query fields according to the RFC specification
		struct QUERY query;
		if(ntohs(dns->q_count) == 1) {
			// Get NAME
			query.name = convertRFC2Name(reader,buf,&stop);
			//printf("ESTE é o QUERY.NAME = %s\n", query.name);
			
			for(i=0; i<strlen((char*)query.name); i++){
        		aux[i]=(char)(query.name[i]);
    		}

    		aux[i++] = '\0';
    		//aux = (const char*)getip((char*)query.name);
			/*--------CODIGO NOVO---------*/
			//--------Verificacao se o pedido corresponde ao dominio local e respetiva insercao da lista pertendida--------

			if(verifica_dominio_local(aux)==1){
				//insere na lista de dominios locais com prioridade
				insereListaDomains(lista_d_prio,aux,dest,dns->id,sockfd);
				sem_post(&sem_mutex);
				//tratar o pedido interno

			}
			else{
				//insere na lista de dominios normais
				insereListaDomains(lista_d_nor,aux,dest,dns->id,sockfd);
				sem_post(&sem_mutex);
				// tratar o pedido externo
			}
			/*---------ACABA AQUI O CODIGO NOVO-------*/
			
			reader = reader + stop;
			
			// Get QUESTION structure
			query.ques = (struct QUESTION*)(reader);
			reader = reader + sizeof(struct QUESTION);
			// Check question type. We only need to process A records.
			if(ntohs(query.ques->qtype) == 1) {
				printf("A record request.\n\n");
			} else {
				printf("NOT A record request!! Ignoring DNS message!\n");
				continue;
			}
			
		} else {
			printf("\n\nDNS message must contain one question!! Ignoring DNS message!\n\n");
			continue;
		}
		
		// Received DNS message fulfills all requirements.
		
		
		// ****************************************
		// Print received DNS message QUERY
		// ****************************************
		printf(">> QUERY: %s\n", query.name);
		printf(">> Type (A): %d\n", ntohs(query.ques->qtype));
		printf(">> Class (IN): %d\n\n", ntohs(query.ques->qclass));
			
		// ****************************************
		// Example reply to the received QUERY
		// (Currently replying 10.0.0.2 to all QUERY names)
		// ****************************************
		
		escreve_pipe(estatisticas);
		//sendReply(dns->id, query.name, inet_addr("10.0.0.2"), sockfd, dest);
	}
    return 0;
}

/**
	sendReply: this method sends a DNS query reply to the client
	* id: DNS message id (required in the reply)
	* query: the requested query name (required in the reply)
	* ip_addr: the DNS lookup reply (the actual value to reply to the request)
	* sockfd: the socket to use for the reply
	* dest: the UDP package structure with the information of the DNS query requestor (includes it's IP and port to send the reply)
**/
void sendReply(unsigned short id, unsigned char* query, int ip_addr, int sockfd, struct sockaddr_in dest) {
		unsigned char bufReply[65536], *rname;
		char *rip;
		struct R_DATA *rinfo = NULL;
		
		//Set the DNS structure to reply (according to the RFC)
		struct DNS_HEADER *rdns = NULL;
		rdns = (struct DNS_HEADER *)&bufReply;
		rdns->id = id;
		rdns->qr = 1;
		rdns->opcode = 0;
		rdns->aa = 1;
		rdns->tc = 0;
		rdns->rd = 0;
		rdns->ra = 0;
		rdns->z = 0;
		rdns->ad = 0;
		rdns->cd = 0;
		rdns->rcode = 0;
		rdns->q_count = 0;
		rdns->ans_count = htons(1);
		rdns->auth_count = 0;
		rdns->add_count = 0;
		
		// Add the QUERY name (the same as the query received)
		rname = (unsigned char*)&bufReply[sizeof(struct DNS_HEADER)];
		convertName2RFC(rname , query);
		
		// Add the reply structure (according to the RFC)
		rinfo = (struct R_DATA*)&bufReply[sizeof(struct DNS_HEADER) + (strlen((const char*)rname)+1)];
		rinfo->type = htons(1);
		rinfo->_class = htons(1);
		rinfo->ttl = htonl(3600);
		rinfo->data_len = htons(sizeof(ip_addr)); // Size of the reply IP address

		// Add the reply IP address for the query name 
		rip = (char *)&bufReply[sizeof(struct DNS_HEADER) + (strlen((const char*)rname)+1) + sizeof(struct R_DATA)];
		memcpy(rip, (struct in_addr *) &ip_addr, sizeof(ip_addr));
		
		// Send DNS reply
		printf("\nSending Answer... ");
		if( sendto(sockfd, (char*)bufReply, sizeof(struct DNS_HEADER) + (strlen((const char*)rname) + 1) + sizeof(struct R_DATA) + sizeof(ip_addr),0,(struct sockaddr*)&dest,sizeof(dest)) < 0) {
			printf("FAILED!!\n");
		} else {
			printf("SENT!!!\n");
		}
}

/**
	convertRFC2Name: converts DNS RFC name to name
**/
u_char* convertRFC2Name(unsigned char* reader,unsigned char* buffer,int* count) {
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
    name[0]='\0';
 
    while(*reader!=0) {
        if(*reader>=192) {
            offset = (*reader)*256 + *(reader+1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1;
        } else {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0) {
            *count = *count + 1;
        }
    }
 
    name[p]='\0';
    if(jumped==1) {
        *count = *count + 1;
    }
 
    for(i=0;i<(int)strlen((const char*)name);i++) {
        p=name[i];
        for(j=0;j<(int)p;j++) {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0';
    return name;
}

/**
	convertName2RFC: converts name to DNS RFC name
**/
void convertName2RFC(unsigned char* dns,unsigned char* host) {
    int lock = 0 , i;
    strcat((char*)host,".");
     
    for(i = 0 ; i < strlen((char*)host) ; i++) {
        if(host[i]=='.') {
            *dns++ = i-lock;
            for(;lock<i;lock++) {
                *dns++=host[lock];
            }
            lock++;
        }
    }
    *dns++='\0';
}

int init(){ //Funcao que vai inicializar tudo o que é necessario

	//Declaraçoes
	int processo_config;
	int processo_estatistic;

	//Bloquear sinais

	sigset_t sinais;

	if (sigfillset(&sinais)!=0){
		printf("Não pode inciar alguns sinais...!\n");
		exit(1);
	}

	//Estes sinais vao ser usados
    sigdelset(&sinais,SIGINT);
    sigdelset(&sinais,SIGUSR1);
    sigdelset(&sinais,SIGALRM);

    if (sigprocmask(SIG_BLOCK,&sinais,0)!=0){
        fprintf(stderr,"Não pode bloquear mais sinais\n");
        exit(1);
    }

	//Cria memoria partilhada das configuraçoes

	if ((shmid_sem_config = shmget(IPC_PRIVATE, sizeof (sem_t), IPC_CREAT | 0766)) < 0) {
        perror("Erro memoria partilhada!\n");
        exit(1);
    }

    if ((sem_config = (sem_t*) shmat(shmid_sem_config, NULL, 0)) == (sem_t*)-1) { //memoria mapeada
        perror("Erro memoria partilhada!\n");
        exit(1);
    }
	printf("Memória Partilhada (sem_config)!\n");

	sem_init(sem_config, 1, 0);

    if ((shmid_config = shmget(IPC_PRIVATE, sizeof (lista_config), IPC_CREAT | 0766)) < 0) {
        perror("Erro memoria partilhada!\n");
        exit(1);
    }

    if ((mempart = (lista_config*) shmat(shmid_config, NULL, 0)) == (lista_config*)-1) { //memoria mapeada
        perror("Erro memoria partilhada!\n");
        exit(1);
    }
    printf("Memória Partilhada (lista_config)!\n");

    //Cria memoria partilhada da flag

	if ((shmid_flag = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0766)) < 0) {
        perror("Erro memoria partilhada!\n");
        exit(1);
    }

    if ((flag_sinal = (int*) shmat(shmid_flag, NULL, 0)) == (int*)-1) { //memoria mapeada
        perror("Erro memoria partilhada!\n");
        exit(1);
    }
	printf("Memória Partilhada (flag_sinal)!\n");

	*flag_sinal = 0;


    //mempart->domains = (char**)malloc(MAXDOMAINS*sizeof(char*));

	coloca_listanull();

    //Mapear ficheiro localdns.txt em memoria
    mmap_localdns();

    //Inicializa a estrutura de estatisticas e adiciona a hora de arranque
	estatisticas = malloc(sizeof(lista_estatistic));
	estatisticas->hora_ultima_informacao = (char *) malloc(sizeof(char));
	writeHourPresent(estatisticas->hora_inicio);
	inicializa_lista_estatisticas();

	
	//Cria processo de configuraçoes
	processo_config = fork();
	if(processo_config == -1){
		//verifica se o fork foi bem feito
		printf("Erro ao criar o processo de configuraçoes!\n");
	}
	else if(processo_config == 0){
		pid_configuracoes = getpid();
		le_configuracoes();
		printf("PID CONFIGURACOES = %d \n", pid_configuracoes);
    	while(1){
    		signal(SIGUSR1, catch_SIGUSR1); //kill -10 pid
    	}
		exit(0);
	}

	sem_wait(sem_config); //espera que as configs estejam todas carregadas e prontas a ser utilizadas

	//Cria named_pipe
	criar_named_pipe();

	//Cria processo de estatisticas
	processo_estatistic = fork();

	if(processo_estatistic == -1){
		//verifica se o fork foi bem feito
		printf("Erro ao criar o processo de estatisticas!\n");
	}
	else if(processo_estatistic == 0){
		pid_statistic = getpid();
		printf("PID ESTATISTICAS = %d \n", pid_statistic);
		//gen_statistics();
		exit(0);
	}

	//Cria recursos de sincronizacao (semaforos)
	init_semaphore();

	//Cria threads de acordo com as configuracoes

	create_thread_pool(mempart->num_threads);

	//Envia para os named pipes das estatisticas a hora de arranque

	escreve_estatisticas();
	
	return 0;
}


void* pedidos(void *a){
	//provavelmente vou ter de ter um mutex
	char *IPdevolvido = (char*)malloc(20*sizeof(char));
	Dados dados;
  	unsigned char*cenas;
  	cenas = malloc(100*sizeof(unsigned char));
  	while(1){
  		sem_wait(&sem_mutex);
	  	if(empty_lista(lista_d_prio)==0){
	  		//printf("ENTROU NA PRIO\n");
	  		//trata do primeiro pedido
	  		pthread_mutex_lock(&th_mutex);
	  		dados = retiraPrimeiroLista(lista_d_prio);
	  		IPdevolvido = retira_IP_local(dados.name);
			printf("IP (interno) para o %s sera %s !", dados.name, IPdevolvido);
	  		strncpy((char *)cenas, dados.name, strlen(dados.name));
	  		sendReply(dados.id, cenas, inet_addr(IPdevolvido), dados.sockfd, dados.dest);
	  		
	  		(estatisticas->n_enderecos_l_resolvidos)++; //aumenta o numero de externos resolvidos
	  		//writeHourPresent(estatisticas->hora_ultima_informacao); //escreve a data da ultima informaçao obtida
	  		escreve_data_ultima_informacao();

	  		pthread_mutex_unlock(&th_mutex);
	  	}
	  	else if(empty_lista(lista_d_nor)==0){
	  		//trata primeiro pedido
	  		//printf("ENTROU NA NOR\n");
	  		if(*flag_sinal!=1){
	  			pthread_mutex_lock(&th_mutex);
		  		dados = retiraPrimeiroLista(lista_d_nor);

		  		if(validateRequest(dados.name)==1){
		  			IPdevolvido = execexterno(dados.name);
					printf("IP (externo) para o %s sera %s !", dados.name, IPdevolvido);
			  		strncpy((char *)cenas, dados.name, strlen(dados.name));
			  		sendReply(dados.id, cenas, inet_addr(IPdevolvido), dados.sockfd, dados.dest);

			  		(estatisticas->n_enderecos_e_resolvidos)++; //aumenta o numero de externos resolvidos
			  		//writeHourPresent(estatisticas->hora_ultima_informacao); //escreve a data da ultima informaçao obtida
			  		escreve_data_ultima_informacao();

			  		pthread_mutex_unlock(&th_mutex);
		  		}else{
		  			strcpy(IPdevolvido, "0.0.0.0");
		  			printf("Pedido recusado para %s !", dados.name);
		  			strncpy((char *)cenas, dados.name, strlen(dados.name));
			  		sendReply(dados.id, cenas, inet_addr(IPdevolvido), dados.sockfd, dados.dest);

			  		(estatisticas->n_pedidos_recusados)++; //aumenta o numero de externos resolvidos
			  		//writeHourPresent(estatisticas->hora_ultima_informacao); //escreve a data da ultima informaçao obtida
			  		escreve_data_ultima_informacao();

			  		pthread_mutex_unlock(&th_mutex);
		  		}
	  		}
	  		else{
	  			pthread_mutex_lock(&th_mutex);
		  		dados = retiraPrimeiroLista(lista_d_nor);

	  			strcpy(IPdevolvido, "0.0.0.0");
		  		printf("Pedido recusado para %s !", dados.name);
		  		strncpy((char *)cenas, dados.name, strlen(dados.name));
			  	sendReply(dados.id, cenas, inet_addr(IPdevolvido), dados.sockfd, dados.dest);

			  	(estatisticas->n_pedidos_recusados)++; //aumenta o numero de externos resolvidos
			  	//writeHourPresent(estatisticas->hora_ultima_informacao); //escreve a data da ultima informaçao obtida
			  	escreve_data_ultima_informacao();

			  	pthread_mutex_unlock(&th_mutex);
	  		}
	 	}
	}
}


void init_semaphore() {
	if((sem_init(&sem_mutex,0,0) == -1)){ //cria dois semáforos inicializados a 1
		perror("Failed to initialize semaphore");
		exit(1);
	}
	if((sem_init(&sem_testa,1,1) == -1)){ //cria dois semáforos inicializados a 1
		perror("Failed to initialize semaphore");
		exit(1);
	}
}


