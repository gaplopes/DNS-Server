#include "header.h"

struct tm *localtime(const time_t *timer); //estrutura para o time

/*-----------------------*/
/* FUNCOES CONFIGURACOES */
/*-----------------------*/


List_domains cria_lista_domains(){
    List_domains lista = (List_domains)malloc(sizeof(ListDomains));
    if(lista!=NULL){
        lista->next = NULL;
    }
    return lista;
}


void insereLista(char domain[50]){
    char aux[10];
    strcpy(aux,"NULL");
    int i=1;
    if(strcmp(mempart->domains[0],aux)==0){
        strcpy(mempart->domains[0],domain);
    }
    else{
        while(strcmp(mempart->domains[i],aux)!=0){
            i++;
        }
        strcpy(mempart->domains[i],domain);
    }

}


void insereListaDomains(List_domains lista,char name[],struct sockaddr_in dest,int id,int sockfd){
    List_domains aux;
    List_domains novo;
    aux = lista;
    novo = (List_domains)malloc(sizeof(ListDomains));
    
    novo->dados_pedido.dest = dest;
    strcpy(novo->dados_pedido.name,name);
    novo->dados_pedido.id = id;
    novo->dados_pedido.sockfd = sockfd;

    while(aux->next!=NULL){
        aux = aux->next;
    }

    novo->next = NULL;
    aux->next = novo;
}

Dados retiraPrimeiroLista(List_domains lista){
    List_domains aux = lista;
    List_domains aux2 = lista->next;
    Dados dados;
 
    if(aux2->next == NULL){ //para o caso de so ter 1 pedido
        dados = aux2->dados_pedido;
        aux->next = NULL;
        free(aux2);
        return dados;
    }
    
    dados = aux2->dados_pedido;
    aux = lista;
    aux2 = lista->next;
    aux->next = aux2->next;
    free(aux2);
    return dados;
 
}

void imprime_lista(){
    int i;
    for(i=0;i<MAXDOMAINS;i++){
        if(strcmp(mempart->domains[i],"NULL")!=0){
            printf(" Domino: %s\n",mempart->domains[i]);
        }
    }
}
void coloca_listanull(){
    int i=0;
    for(i=0;i<MAXDOMAINS;i++){
        //mempart->domains[i] = malloc (MAXLINHA * sizeof (char));
        strcpy(mempart->domains[i],"NULL");
    }
}

int le_configuracoes(){
    FILE *fp;
    char linha[MAXLINHA],threads[MAXLINHA],domains1[MAXLINHA],auxdomains1[MAXLINHA],local_domain[MAXLINHA],
    local_domain_aux[MAXLINHA],named_pipe_estatistics[MAXLINHA],aux[MAXLINHA],aux_pipe[MAXLINHA];

    int i=0,num;
    char *token;

    //printf("\nConfiguracoes do Servidor\n\n");
 
    fp = fopen("config.txt", "r");   //abir o ficheiro em modo leitura
    if( fp == NULL )
    {
        printf("ERRO: não consigo abrir o ficheiro dados.txt\n");       //verificação
        exit(1);
    }

    //printf("entrei no le configuracoes\n");

    while(fgets(linha,MAXLINHA,fp)!=NULL){
        
        if(i==0){
            strcpy(threads, strtok(linha, "="));
            strcpy(aux, strtok(NULL, " "));
            num = atoi(aux);
            mempart->num_threads = num;
            //printf("NUM THREADS = %d\n", mempart->num_threads);
                    }
        else if(i==1){
            strcpy(auxdomains1, strtok(linha, "="));
            while((token = strtok(NULL, "; "))){
                strcpy(domains1,token);
                insereLista(domains1);
            }
            //mempart->domains = list_domain;
        }
        else if(i==2){
            strcpy(local_domain_aux, strtok(linha, "="));
            strcpy(local_domain, strtok(NULL, " "));
            strcpy(mempart->local_domain,local_domain);
            //printf("LOCAL DOMAIN = %s\n", mempart->local_domain);
        }
        else{
            strcpy(named_pipe_estatistics, strtok(linha, "="));
            strcpy(aux_pipe, strtok(NULL, " "));
            strcpy(mempart->named_pipe,aux_pipe);
            //printf("NAMED PIPE = %s\n", mempart->named_pipe);
        }
    i++;
    }
 
    fclose(fp);
 
    sem_post(sem_config);
    return 0;
}

int le_configuracoes_verifica(){ 
    FILE *fp;
    char linha[MAXLINHA],domains1[MAXLINHA],auxdomains1[MAXLINHA],local_domain[MAXLINHA],
    local_domain_aux[MAXLINHA];

    int i=0;
    char *token;

    //printf("\nConfiguracoes do Servidor\n\n");
 
    fp = fopen("config.txt", "r");   //abir o ficheiro em modo leitura
    if( fp == NULL )
    {
        printf("ERRO: não consigo abrir o ficheiro dados.txt\n");       //verificação
        exit(1);
    }

    //printf("entrei no le configuracoes\n");

    while(fgets(linha,MAXLINHA,fp)!=NULL){
        
        if(i==0){
            continue;
        }
        else if(i==1){
            strcpy(auxdomains1, strtok(linha, "="));
            while((token = strtok(NULL, "; "))){
                strcpy(domains1,token);
                insereLista(domains1);
            }
            //mempart->domains = list_domain;
        }
        else if(i==2){
            strcpy(local_domain_aux, strtok(linha, "="));
            strcpy(local_domain, strtok(NULL, " "));
            strcpy(mempart->local_domain,local_domain);
            //printf("LOCAL DOMAIN = %s\n", mempart->local_domain);
        }
        else{
            continue;
        }
        i++;
    }
 
    fclose(fp);
 
    sem_post(sem_config);
    return 0;
}


/*----------------------*/
/* FUNCOES ESTATISTICAS */
/*----------------------*/

int inicializa_lista_estatisticas(){
    estatisticas->n_total_pedidos = 0;
    estatisticas->n_pedidos_recusados = 0;
    estatisticas->n_enderecos_l_resolvidos = 0;
    estatisticas->n_enderecos_e_resolvidos = 0;
    return 0;
}

void writeHourPresent(char* time_string)
{
    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);
    char c_time_string[20];
    int hours=aTime->tm_hour ;
    int minutes=aTime->tm_min;
    int seconds = aTime->tm_sec;
    //HORA////////

    /* Print to stdout. */
    sprintf(c_time_string, "%02d:%02d:%02d", hours, minutes, seconds);
    strcpy(time_string, c_time_string);
}

int escreve_data_ultima_informacao(){
        time_t data_hora_segundos;
        struct tm *timeinfo;

        time(&data_hora_segundos);
        timeinfo = localtime(&data_hora_segundos); 

        estatisticas->hora_ultima_informacao = asctime(timeinfo);

        return 0;
}

void escreve_estatisticas(){
    printf("\n\nHora de arranque do servidor: %s", estatisticas->hora_inicio);
    printf("\nNumero total de pedidos: %d", estatisticas->n_total_pedidos);
    printf("\nNumero de pedidos recursados: %d", estatisticas->n_pedidos_recusados);
    printf("\nNumeros de endereços do dominio local resolvidos: %d", estatisticas->n_enderecos_l_resolvidos);
    printf("\nNumeros de endereços do dominio externos resolvidos: %d", estatisticas->n_enderecos_e_resolvidos);
    printf("\nData e hora da ultima informaçao obtida: %s\n\n", estatisticas->hora_ultima_informacao);
}

void criar_named_pipe(){
    // Create the FIFO if it does not exist
    //char name[MAXLINHA];
    //strcpy(name, mempart->named_pipe);
    
    if ((mkfifo(mempart->named_pipe, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)){
        perror("Cannot create pipe: ");
        exit(0);
    }
}

void escreve_pipe(){

    if ((fd = open(mempart->named_pipe, O_WRONLY)) < 0) {
        perror("Cannot open pipe for writing!");
        exit(0);
    }

    write(fd, &estatisticas, sizeof(lista_estatistic));

    close(fd);
}

void le_pipe(int sign){
    lista_estatistic l;
    // Opens the pipe for reading
    if ((fd = open(mempart->named_pipe, O_RDONLY)) < 0){
        perror("Cannot open pipe for reading!");
        exit(0);
    }

    if(sign!=0){
        alarm(3);
        read(fd, &l, sizeof(lista_estatistic));
    }

    printf("\nHora de arranque do servidor: %s", l.hora_inicio);
    printf("\nNumero total de pedidos: %d", l.n_total_pedidos);
    printf("\nNumero de pedidos recusados: %d", l.n_pedidos_recusados);
    printf("\nNumeros de endereços do dominio local resolvidos: %d", l.n_enderecos_l_resolvidos);
    printf("\nNumeros de endereços do dominio externos resolvidos: %d", l.n_enderecos_e_resolvidos);
    printf("\nData e hora da ultima informaçao obtida: %s\n", l.hora_ultima_informacao);

    close(fd);
}


/*-----------------*/
/* FUNCOES PEDIDOS */
/*-----------------*/

void catch_toclose(int sign) // main process monitors the reception of Ctrl-C
{
    escreve_estatisticas();
    cleanup();
}

void catch_SIGUSR1(int sign){
    int erro;

    if(*flag_sinal==0){
        coloca_listanull();
        printf("Entrei no SIGUSR1, estou em manutencao!...\n");
        *flag_sinal=1;
    }
    else{
        erro = le_configuracoes();
        sem_wait(sem_config);
        if(erro == 1){
            printf("Erro na actualizacao com o SIGUSR1!\n");
        }
        else{
            printf("Configuracoes atualizadas com o SIGUSR1!\n");
        }
        *flag_sinal = 0;
    }
}


//FUNCOES DE LIMPEZA


void clean_listas(List_domains lista) // clean up resources by pressing Ctrl-C
{
        List_domains ant = lista;
        List_domains aux;
        while(ant!=NULL){
                aux = ant->next;
                close(ant->dados_pedido.sockfd);
                free(ant);
                ant = aux;
        }
 
}
 
void cleanup(int signo){//Funcao que vai apagar e limpar tudo o que foi utilizado, clean up resources by pressing Ctrl-C
    
    int i;

    printf("Cleanig all...!\n");
    //Liberta os semáforos e memoria
 
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_testa);
    sem_destroy(sem_config);
    shmctl(shmid_sem_config, IPC_RMID, NULL);
    shmctl(shmid_config, IPC_RMID, NULL);


    for(i=0;i<mempart->num_threads;i++){
        pthread_cancel(threads[i]);
    }
    for(i=0;i< (mempart->num_threads);i++){
        pthread_join(threads[i],NULL);
    }

    //Remove a fila de Mensagens
    clean_listas(lista_d_prio);
    clean_listas(lista_d_nor);
 
    //Fecha o socket
    close(sockfd_close);

    //Libertar os pipes
    //fclose(fd);
 
    //Libertar malloc's
    //free(mempart->domains);

    //Libertar o mmap
    munmap(line, size.st_size);
 
    printf("System clean! Closing...\n");
    wait(NULL);
    exit(0);
}

//Verifica dominio do IP recebido, se esta no ficheiro

void mmap_localdns(){
        int fp;
        line = (char*)malloc(10000*sizeof(char));
        printf("\nLOCALDNS.txt\n");
       
        fp=open("localdns.txt", O_RDONLY);
 
        stat("localdns.txt", &size);
        line = mmap((caddr_t)0, size.st_size, PROT_READ, MAP_SHARED, fp, 0);
        close(fp);
       
        //printf("%s", line);
       
        //munmap(line, size.st_size);
        printf("%s", line);
}
 
//Verifica dominio do IP recebido, se esta no ficheiro
 
char* retira_IP_local(char nome[]){
    char *IP = (char *) malloc (50 * sizeof (char));
    int i=0,j, k =0, iaux=0;
    char name[50], ipaux[50];
    
    while(line[i]!='\0'){
        j=0;
        k=0;
        memset(name,'\0',50);
        
        if(line[i]== '\n'){
            iaux = i+1;
        }
        if(line[i] == ' '){
            for(j=iaux; j< i; j++){
                name[j-iaux] = line[j];
            }
            if(strcmp(name, nome)==0){
                i++;
                while(1){
                    ipaux[k++] = line[i++];
                    if(line[i]=='\n'){
                        break;
                    }
                }
                break;  
            }
            else{
                i++;
            }
        }
        else{
            i++;
        }
    }
    strcpy(IP, ipaux);

    return IP;
}

int verifica_dominio_local(char pedido[]){
    //char * pedido2 = (char *) pedido; //converte de unsigned char para char
    char aux1[MAXLINHA], aux2[MAXLINHA],aux3[MAXLINHA];
    char aux[MAXLINHA];
    strcpy(aux,"");
    strcpy(aux2,"");
    strcpy(aux3,"");
 
    strcpy(aux3,pedido);
    strcpy(aux1, strtok(aux3, "."));
    strcpy(aux2, strtok(NULL,"\n\0 "));
    strcpy(aux,mempart->local_domain);
    if(strncmp(aux2, aux, strlen(aux2))==0){
        return 1;
    }
 
    return 0;
}

int verifica_dominio(char pedido[], List_config lista_domains){
    List_config aux;
    aux = lista_domains->next;

    int conta;
    char aux1[MAXLINHA], aux2[MAXLINHA], aux3[MAXLINHA];

    strcpy(aux1, strtok(pedido, "."));
    strcpy(aux2, strtok(NULL, "."));
    strcpy(aux3, strtok(NULL, " "));

    while(aux!=NULL){
        if(strcmp(aux3, aux->domain)==0){
            conta = 1;
            return conta;
        }
        else{
            conta = 0;
        }

        aux = aux->next;
    }

    return conta;
}

void create_thread_pool(int n_threads){
    int i;

    #ifdef DEBUG
        printf("Creating thread pool\n");
    #endif

    if((threads = malloc(n_threads * sizeof(pthread_t))) == NULL){
        perror("While allocating space for thread pool\n");
        return;
    }

    if((id_threads = malloc(n_threads * sizeof(int))) == NULL){
        perror("While allocating space for thread pool id\n");
        return;
    }

    for(i = 0; i < n_threads; i++){
        id_threads[i] = i;
        pthread_create(&threads[i], NULL, pedidos, &id_threads[i]);
    }

    /*for(i = 0; i < n_threads; i++){
        pthread_join(threads[i], NULL);
    }*/
}

int empty_lista(ListDomains *lista){ //funcao que verifica se a lista contem algo
    if(lista->next == NULL){
        return 1;// a lista esta vazia
    }
    return 0;  // a lista tem alguma coisa
}

char* execexterno(char *pedido){
    FILE *fp;
    char *final = (char*)malloc(100*sizeof(char));
    char aux1[1000];

    char dig[100];
    strcpy(dig, "");
    strcat(dig, "dig ");
    strcat(dig, pedido);
    strcat(dig, " +noall +answer +short");

    fp = popen(dig,"r");
    if(fgets(aux1,1000,fp)==NULL){
        strcpy(final, "0.0.0.0");
    }else{
        fgets(aux1,1000,fp);
        strcpy(final, aux1);
    }

    pclose(fp);

    return final;
}

int validateRequest(char *name){
    char aux[50], aux1[50], aux2[50],aux3[50], aux4[50];
    int i;
    strcpy(aux,"");
    strcpy(aux2,"");
    strcpy(aux3,"");
 
    strcpy(aux3,name);
    
    strcpy(aux1, strtok(aux3, "."));
    strcpy(aux2, strtok(NULL,"."));
    strcpy(aux4, strtok(NULL, "\n\0"));

    imprime_lista();

    for(i=0;i<MAXDOMAINS;i++){
        if(strcmp(mempart->domains[i],"NULL")!=0){
            strcpy(aux,mempart->domains[i]);
            if(strncmp(aux, aux4, strlen(aux4))==0){
                return 1;
            }
        }
    }
    return 0;
}

void gen_statistics(){
    int aux;
    lista_estatistic l;

    //como ta nos slides
    struct timeval x;
    x.tv_sec = 10;
    x.tv_usec = 0;
 
    while(1){
        if ((fd = open(mempart->named_pipe,O_RDONLY|O_NONBLOCK))<0){
            perror("Cannot open pipe for reading");
            exit(0);
        }
 
        fd_set read_set;
        //colocar como se nao tivesse mexido
        FD_ZERO(&read_set);
 
        //ASSOCIAR O NUMERO DE DESCRITOR DE FICHEIRO
        FD_SET(fd,&read_set);
 
        if ((aux = select(fd+1,&read_set,NULL,NULL,&x)) > 0){
            read(fd,&l,sizeof(lista_estatistic));
            close(fd);
        }
 
        else if((aux = select(fd+1,&read_set,NULL,NULL,&x)) == 0){
            //completar
            printf("\nHora de arranque do servidor: %s", l.hora_inicio);
            printf("\nNumero total de pedidos: %d", l.n_total_pedidos);
            printf("\nNumero de pedidos recusados: %d", l.n_pedidos_recusados);
            printf("\nNumeros de endereços do dominio local resolvidos: %d", l.n_enderecos_l_resolvidos);
            printf("\nNumeros de endereços do dominio externos resolvidos: %d", l.n_enderecos_e_resolvidos);
            printf("\nData e hora da ultima informaçao obtida: %s\n", l.hora_ultima_informacao);
            x.tv_sec = 10;
            x.tv_usec = 0;
        }

        if(aux == -1){
            perror("Cannot Select");
        }       
    }
}