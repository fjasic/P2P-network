#include "function.h"

char buffer_read[BUFFERSIZE];  //bafer odakle citamo
char buffer_write[BUFFERSIZE];	//bafer u koji pisemo 
char large_buff[BUFFERSIZE]; 
char hostname[100];	//ovde smestam hostname
int sockfd; 
int data_sock; 
int new_port = -1;
struct sockaddr_in serv_addr; // socket info

void error(const char *msg, int i){  //poruka o gresci
    perror(msg);
    exit(i);
}

void change_type(char t){		
	bzero(buffer_write, BUFFERSIZE); //bafer iz kog citam je prazan,postavlja sve na 0
	sprintf(buffer_write ,"TYPE %c\r\n", t);//formatiranje bafera
	send( sockfd, buffer_write, (int)strlen(buffer_write), 0);//inicijalizacija soketa 
	parse();//razclanjivanje
	bzero(buffer_write, BUFFERSIZE);//bafer u koji pisem je prazan,postavlja sve na 0
}

int parse() { 
	int flag = 0; 
	while(flag == 0) {
		bzero(large_buff, BUFFERSIZE);
		int result = recv( sockfd, large_buff, BUFFERSIZE, 0);
		if (result < 0)  error("Error read", 5);
		char *p = strtok(large_buff, "\n");//deli string na seriju tokena
		while (p != 0 ) {
			char *command;
			command = p;
			printf("%s\n",command);
			if(command[3] == ' '){
				char type[4];
				strncpy(type, command, 4);//kopiram komandu
				type[4] = 0;

				flag = -1;
				return atoi(type);
			}
			p = strtok(NULL,"\n");
		}
	}
	return 0;
}

void pasv(){
	int type;
	bzero(buffer_read, BUFFERSIZE);
	recv( sockfd, buffer_read, BUFFERSIZE, 0);//citanje sa soketa
	printf("%s\n",buffer_read);
	if(buffer_read[3] == ' '){//ako je 4 karakter prazan,tj komanda dolsa do kraja
		char ctype[4];
		strncpy(ctype, buffer_read, 4);
		ctype[4] = 0;
		type = atoi(ctype);//pretvara sting u integer
	} else type = 0;

	if( type == 227){
		if(strcmp(hostname, "localhost") != 0){
			new_port = get_port(buffer_read);
		}else
			new_port = get_port2(buffer_read);

		data_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (data_sock < 0) error("ERROR opening data socket\n", 2);
		serv_addr.sin_port = htons(new_port);
		if(strcmp( hostname, "localhost") != 0) change_type('i');
	}else new_port = -1;
}


int main(int argc, char const *argv[])
{
	if(argc != 3) error("argument error!", 0);

	struct hostent *host;		  // info hosta
	int i;

	//kreiranje socketa
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket\n", 2);

	//dobijanje informacija o hostu 
	serv_addr.sin_family = AF_INET;
	host = gethostbyname(argv[1]);
	strcpy(hostname, argv[1]);
	if (host == NULL) error("ERROR, no such host\n", 0);
	memcpy(&serv_addr.sin_addr.s_addr, host->h_addr, host->h_length); 

	//dobijanje informacija o portu
	serv_addr.sin_port = htons(atoi(argv[2]));
	bzero(&(serv_addr.sin_zero), 8);

	//uspostavljanje konekcije
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting! \n", 3);

	parse();

	while(strcmp(buffer_write, "QUIT\r\n") != 0){ //sve dok mu nisam prosledio komandu QUIT,program nastavlja da radi
		printf("ftp:$");
		bzero(buffer_write, BUFFERSIZE);
		fgets(buffer_write, BUFFERSIZE, stdin);

		for(i = 0; buffer_write[i] != '\n'; i++); buffer_write[i] = '\0'; 
		strcat(buffer_write, "\r\n");//citam sa ulaza

		int result = send( sockfd, buffer_write, (int)strlen(buffer_write), 0); 
		if (result < 0) error("Error write to serVer!\n", 4);

		if(strncmp(buffer_write, "STOR ", 5) == 0){ //slanje podataka na server 
			if(new_port != -1){
				sleep(1);
				if(connect(data_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)//ako se konekcija nije uspostavla salji gresku
					printf("Error connecting data socket! \n");
			}
			int type = parse();
		    if(type == 150){
				// primanje fajlova
				char filename[100]; int j = 0;
				for(i = 0; buffer_write[i] != ' '; i++); i++;
				for(; buffer_write[i] != '\r'; i++){
					filename[j++] = buffer_write[i];
				}filename[j] = 0;

				sleep(1);
				ftp_put(data_sock, filename);
				type = parse();
				close(data_sock);
			}else if(type == 500){
					parse();
			}
			new_port = -1;

		}else if(strcmp(buffer_write, "PASV\r\n") == 0){ //pasivni rezim rezim
			pasv();

		}else if(strcmp(buffer_write, "NLST\r\n") == 0){//komanda za listanje datoteka na serveru
			if(strcmp(hostname, "localhost") != 0){
				if(new_port != -1){
					sleep(1);
					if(connect(data_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
						printf("Error connecting data socket! \n");
				}
				int type = parse();
				if(type == 150){
					int readCount;
					do{
						readCount = read(data_sock, buffer_read, BUFFERSIZE);
						buffer_read[readCount] = 0; // !!!
						printf("%s",buffer_read);
					} while(readCount != 0);
					close(data_sock);
					parse();
				}else if(type == 500){
					parse(); 
				}
				new_port = -1;

			}else {
				result = recv( sockfd, buffer_read, BUFFERSIZE, 0);
				if (result < 0)  error("Error read", 5);
				printf("%s\n",buffer_read);
			}
		}else if(strncmp(buffer_write, "RETR ", 5) == 0){//skidanje podataka sa servera

			if(new_port != -1){
				sleep(1);
				if(connect(data_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
					printf("Error connecting data socket! \n");
			}
			int type = parse();
		    if(type == 150){
				// primanje fajlova
				char filename[100]; int j = 0;
				for(i = 0; buffer_write[i] != ' '; i++); i++;
				for(; buffer_write[i] != '\r'; i++){
					filename[j++] = buffer_write[i];
				}filename[j] = 0;
				sleep(1);
				ftp_get(data_sock, filename);
				type = parse();
				if(type == 550){
					unlink(filename);
					close(data_sock);
					
				}
				else if(type == 226)
					close(data_sock);
			}else {
				continue;
			}
			new_port = -1;

		}else if(strncmp(buffer_write, "PASS", 4) == 0){ //sifra za korisnika

		    if(parse() == 230)
		    	continue;

		}else if(strncmp(buffer_write, "QUIT\r\n", 4) == 0){ //odjavljivanje sa servera i kraj rada
			parse();
			break;

		}else{
			parse();
		}

	}

	close(sockfd);
	return 0;
}
