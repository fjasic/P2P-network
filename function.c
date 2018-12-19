#include <header.h>

int ftp_get(int soc, char* file_name){
    FILE *fp;//fajl sa kojim manipulisem
    fp = fopen(file_name, "w+"); //otvaranje i pisanje 
    if(fp == NULL){ //ako je fajl prazan
        printf("The file %s is not existed\n", file_name);
		close(soc);
        return 0;
    }else{
		char buffer[FTP_BUF_SIZE];
		bzero(buffer, FTP_BUF_SIZE);
        int file_block_length = 0;
        while ((file_block_length = recv(soc, buffer, FTP_BUF_SIZE, 0)) > 0) 
        {
            if (file_block_length < 0) //ako je duzina fajla manja od nula onda je prazan,i postavljam gresku
            {
                printf("Recieve Data From Client Failed!\n");
                return 0;
            }
            int write_length = fwrite(buffer, sizeof(char), file_block_length, fp);//pisanje u fajl
            if (write_length < file_block_length){
                printf("File: %s Write Failed\n", file_name);
                break;
            }
            bzero(buffer, FTP_BUF_SIZE);
        }
        fclose(fp);//zatvaranje fajla
        printf("File: %s Transfer Finished\n", file_name);
    
    	close(soc);
	}
	return 1;
}

int ftp_put(int soc, char* file_name){

    FILE *file;
    file = fopen(file_name, "rb+");
    if(file == NULL){//fajl je prazan
        printf("The file %s is not existed\n", file_name);
		close(soc);
     	return 0;
    }

    int file_len;
    fseek(file, 0, SEEK_END);//stavljam na kraj fajla 0 bajtova
    file_len = ftell(file); //odredjujem duzinu fajla
    fseek(file, 0, SEEK_SET); 

    int read_len; //citanje duzine
    char read_buf[FTP_BUF_SIZE];
    do{
        read_len = fread(read_buf, sizeof(char), FTP_BUF_SIZE, file);

        if (read_len > 0){
            send(soc, read_buf, read_len, 0); //saljem poruku na server
            file_len -= read_len;
        }
		bzero(read_buf, FTP_BUF_SIZE);
    } while ((read_len > 0) && (file_len > 0));

    fclose(file);
	close(soc);
	return 1;
}

int get_port(char *buffer_read) { //obradjivanje hostname i porta
    char p1[10]; char p2[10]; 
    int count = 0;int i; int j = 0; int k = 0; 
    for(i = 0; buffer_read[i] != '.'; i++){
        if(buffer_read[i] == ',') count++; 
        if(count == 4){
            i++;
            while(buffer_read[i] != ','){
                p1[j++] = buffer_read[i++];
            } p1[j]= '\0';
			count++;
        }
        if(count == 5){
			i++;
            while(buffer_read[i] != ')'){
                p2[k++] = buffer_read[i++];
            } p2[k]= '\0';
        }
    }
    int new_port = atoi(p1)*256 + atoi(p2);  //dobijanje porta 
	
    return new_port;
}

int get_port2(char *buffer_read) {
    char p[10];
    int count = 0;int i; int j = 0;
    for(i = 0; buffer_read[i] != ' '; i++);i++;
    for(; buffer_read[i] != '\n'; i++){
        p[j++] = buffer_read[i];
    } p[j] = 0;
    
    int new_port = atoi(p);
    return new_port;
}
