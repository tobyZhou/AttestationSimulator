/*
 * Attestation Simulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include "Utils.h"
#include "openssl_hmac/hmac.h"

#define BUFSIZE 1024

void log(char * color, char * content){
    printf("%s", color);
    printf("%s", content);
    printf("%s\n",KNRM);
}

unsigned int charToNum (char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    return 0;
}

unsigned long stringToHex(char * value) {
    if(strlen(value) != 8) {
        log(KRED, "The Register values are invalid! Please check! Eg: 0x40000074\n");
        exit(0);
    }
    
    unsigned long result = 0;
    for (int i = 7; i >= 0; i--) {
        result += pow(16, i) * charToNum(*(value + (7-i)));
    }
    return result;
}

void getConfig(char* memoryLog, char* packetIdLog, char* rtuIP, int* rtuPort,
                int* attestTimeThreshold) {
    FILE *fp = fopen("config", "rt");
    if(!fp) {
        log(KRED, "The config file is missing. It should be in the same directory as the program.\n");
        exit(0);
    }
    
    char line[BUFSIZE];
    while(fgets(line, sizeof(line), fp)) {
        
        // Get line, then split into name and value. Remove new line char if any
        char* firstColon  = strchr(line, ':');
        *firstColon = '\0';
        char* name = line;
        char* value = firstColon + 1;        
        char* newLineChar = strchr(value, '\n');
        if(newLineChar!=NULL) {
            *newLineChar = '\0';
        }
        
        //printf("%s   %s\n", name, value);
        //printf("%d   %d\n", strlen(name), strlen(value));
        
        if(strcmp(name, "Memory_Log_File") == 0) {
            strcpy(memoryLog, value);
        } else if(strcmp(name, "Packet_ID_File") == 0) {
            strcpy(packetIdLog, value);
        } else if(strcmp(name, "RTU_IP") == 0) {
            strcpy(rtuIP, value);
        } else if(strcmp(name, "RTU_Port") == 0) {
            *rtuPort = atoi(value);
        } else if(strcmp(name, "Attest_Time_Threshold") == 0) {
            *attestTimeThreshold = atoi(value);
        }
    }
    //printf ("\nFinish getting config. Returning to main now.\n\n");
}

int createSocket() {
    /* Create a socket connection to talk to the RTU */
    /* Try a maximum of 4 times before exiting */
    int sockfd = -1, tries = 0;
    while (tries < 4 && sockfd < 0) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        tries++;
        if (sockfd < 0) {
            printf("ERROR in opening  a socket for communication . Retrying attempt %d \n", tries);
        } else {
            printf("Successfully create socket.\n");
            break;
        }
        if (tries == 4) {
            log(KRED, "Unable to open socket for communication after 4 attempts . Aborting \n");
            exit(0);
        }
    } 
    return sockfd;
}


struct hostent * findServer(char * rtuIP) {
    struct hostent *server;
    server = gethostbyname(rtuIP);
    if (server == NULL) {
        log(KRED, "ERROR Unable to resolve the IP address of the RTU . Aborting \n");
        exit(0);
    } else {
        printf("Successfully find the RTU's IP address.\n");
    }
    return server;
}


int connectServer(int sockfd, struct sockaddr_in * serveraddr, int serveraddr_size) {
    int confd = -1, tries = 0;
    while (tries < 4 && confd < 0) {
        tries++;
        confd = connect(sockfd, serveraddr, serveraddr_size);
        if (confd < 0) {
            printf("ERROR connecting to the RTU . Retrying attempt %d \n", tries);
        } else {
            printf("Successfully connect to server.\n");
            break;
        }
        if (tries == 4) {
            log(KRED, "Unable to connect to the RTU for communication after 4 attempts . Aborting \n");
            exit(0);
        }
    }
    return confd;
}


void testHash() {
    // Test for HMAC
    //unsigned char* key = (unsigned char *) "Jefe";
    unsigned char key[5] = "Jefe";
    unsigned char* data = (unsigned char *) "what do ya want for nothing?";
    unsigned char digest[16];
    int keySize = strlen(key);
    int dataSize = strlen(data);
    
    unsigned char* combine = "content:words;encrypted:integrityCheck";
    int semicolon=0, firstColon=0, secondColon=0;
    char* tmp = combine;
    int startNum = tmp;
    int combineSize = strlen(combine);
    while(*tmp) {
        if (*tmp == ':') {
            if (firstColon==0) {
                firstColon = tmp - startNum;
            } else if (secondColon==0) {
                secondColon = tmp - startNum;
            } else {
                printf("This is WRONG!!!");
                return 0;
            }
        } else if (*tmp == ';') {
            if (semicolon==0) {
                semicolon = tmp - startNum;
            } else {
                printf("This is WRONG!!!");
                return 0;
            }
        }
        tmp++;
    }
    char con[(semicolon - firstColon)];
    char en[(combineSize - secondColon)];
    strncpy(con, combine+firstColon+1, (semicolon - firstColon)-1);
    con[(semicolon - firstColon)-1] = '\0';
    strncpy(en, combine+secondColon+1, (combineSize - secondColon)-1);
    en[(combineSize - secondColon)-1] = '\0';
    puts(con);
    puts(en);
    
    lrad_hmac_md5(data, dataSize, key, keySize, digest);
    for (int i = 0; i < 16; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
    return 0;
}


/* 
 * This function will return a ID for reboot packet.
 *  - ID information is stored in a local file
 *  - ID will only increase, not decrease.
 */
unsigned long int getPacketID (unsigned char* buffer, char* file) {
    FILE *fp;
    int size=20;    // 256^4 is 10 digits in decimal, 1 for end char.
    char readID[size];  
    unsigned long int MAX_ID = 4294967295;  // 256^4 - 1
    
    // Open file for reading
    fp = fopen(file, "rt");
    if(!fp) {
        log(KRED, "The packet ID file cannot be found!\n");
        exit(0);
    }
    
    // Read contents from ID file.
    while(fgets(readID, size, fp));
    // Remove newline char
    char *pos;
    if((pos=strchr(readID, '\n')) != NULL)
        *pos = '\0';
    //printf("%s\n", readID);
    //printf("%d\n", strlen(readID));
    if(strlen(readID) > 10) {  // 256^4 is 10 digits in decimal
        log(KRED, "The ID from packetID file is larger than limit! Please check!\n");
        exit(0);
    }
    unsigned long long int ID = atoll(readID);
    if(ID >= MAX_ID) {
        log(KRED, "The ID from packetID file is equal/larger than limit! Please check!\n");
        exit(0);
    }
    fclose(fp);
    
    // Set new ID and Convert unsigned long int to hexadecimal
    unsigned long int newID = (unsigned long int)ID + 1;
    sprintf(buffer, "%08X", newID & 0xFFFFFFFF);
    buffer[8] = '\0';
    
    return newID;
}


void setNewPacketID (int id, char* file) {
    FILE *fp;
    fp = fopen(file, "w+");
    fprintf(fp, "%lu\n", id);
    fclose(fp);
}


int main(int argc, char** argv) {

    // Declare Variables
    int sockfd = -1, confd = -1, timeExceedLimit = 0, connectionCreated = 0;
    int readResult = -1, writeResult = -1, optionSelected = -1;

    char rtuIP[BUFSIZE];
    int rtuPort = 2404;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    
    struct timeval start, end;
    float diff_time;
    unsigned char buf_recv[BUFSIZE]; // for read socket
    
    // Get configurations from config file
    char memoryLog[BUFSIZE], packetIdLog[BUFSIZE];
    char rebootSuccessMsg[BUFSIZE] = "Packet Received! Rebooting RTU to Attestation mode soon!";
    char warningWrongMode[BUFSIZE] = "Wrong Mode! Please change to Attestation mode!";
    char wrongCmdHash[BUFSIZE] = "Invalid command or hash value.\0";
    int attestTimeThreshold=50;
    getConfig(memoryLog, packetIdLog, rtuIP, &rtuPort, &attestTimeThreshold);
    /*printf("\n%s\n", memoryLog);
    printf("%s\n", packetIdLog);
    printf("%x\n", r0);
    printf("%x\n", r13);
    printf("%x\n", r15);
    printf("%x\n", cpsr);
    printf("%s\n", rtuIP);
    printf("%d\n", rtuPort);
    printf("%d\n", attestTimeThreshold);
    printf("%s\n", rebootSuccessMsg);
    printf("%s\n", warningWrongMode);
    return 0;*/

startconsole:
    // Reset Variables (not all)
    readResult = -1;
    writeResult = -1;
    optionSelected = -1;
    bzero(buf_recv, BUFSIZE);
    
    /* 
     * Show mode options and take input from user.
     */
    log(KYEL, "\n\n***** Welcome to the Substation Automation System Attestation Console *****");    
    printf("1. Reboot RTU to Attestation Mode.  \n");
    printf("2. Initiate Attestation of Substation Automation Systems devices  \n");
    printf("3. Reboot RTU back to normal Mode.  \n");
    //printf("4. Run exploits on Substation Automation Systems devices \n");
    printf("4. Exit application \n");
    printf("Enter your choice of command to execute on the RTU (1, 2 , 3 or 4 ) \n");
    scanf("%d", &optionSelected);

    if ((optionSelected != 1) && (optionSelected != 2) && (optionSelected != 3)) exit(0);

    /*
     * Create socket and server connection
     */
    if(connectionCreated == 0) {
        printf("\n\n----- Create socket and connection ----- \n");
        
        //Create socket.
        sockfd = createSocket();
    
        // Find the server via IP address.
        server = findServer(rtuIP);
    
        // build the server's Internet address.
        bzero((char *) &serveraddr, sizeof (serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *) server->h_addr,
                (char *) &serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(rtuPort);
    
        // Connect to server.
        confd = connectServer(sockfd, &serveraddr, sizeof(serveraddr));
        
        connectionCreated = 1;
    }


    /*
     * Run
     */
    if (optionSelected == 1) {
        printf("\n\n----- Selected Option 1. Sending the command to reboot RTU to attestation mode -----\n");

        //Generate and send reboot command.
        printf("(1). Creating 'Reboot to Atteation' Packet\n");
        unsigned char* cmd = (unsigned char *) "1111";
        unsigned char* key = (unsigned char *) "F2b9";
        unsigned char digest[16];
        unsigned char msg[53];  // "content:<cmd><key>;encrypted:<hash value>\0"
        unsigned char id[9];
        unsigned long int newID;
        
        // Hash ID & cmd together
        newID = getPacketID(id, packetIdLog);
        unsigned char combine[13];
        strcpy(combine, cmd);
        strcat(combine, id);
        strcat(combine, "\0");

        // Compute hash value
        lrad_hmac_md5(combine, strlen(combine), key, strlen(key), digest);
        
        // Make packet message
        strcpy(msg, "contents:");
        //printf("%d\n", strlen(msg));
        strcat(msg, cmd);
        //printf("%d\n", strlen(msg));
        strcat(msg, key);
        //printf("%d\n", strlen(msg));
        strcat(msg, id);
        //printf("%d\n", strlen(msg));
        strcat(msg, ";hash:");
        //printf("%d\n", strlen(msg));
        strcat(msg, digest);
        //printf("%d\n", strlen(msg));
        strcat(msg, "\0");
        
        // Write to RTU
        printf("\n(2). Sending Packet to RTU\n");
        //return 0 ;
        writeResult = write(sockfd, msg, strlen(msg));
        if (writeResult < 0) {
            log(KRED, "Write API : Timeout while writing to the RTU. Please try again. \n");
            goto startconsole;
        } else {
            printf("Successfully sending reboot command (to Attestation mode).\n");
        }
        
        // Read from RTU
        printf("\n(3). Receiving packet from RTU\n");
        readResult = read(sockfd, buf_recv, BUFSIZE);
        if(readResult < 0) {
            log(KRED, "READ API : Timeout while reading from the RTU. Please try again...  \n");
            goto startconsole;
        } else {
            printf("Received Message from RTU.\n");
            log(KYEL, buf_recv);
        }
        
        if(strcmp(buf_recv, rebootSuccessMsg)==0) {
            setNewPacketID(newID, packetIdLog);
        }

        sleep(3);
        close(sockfd);
        
    } else if (optionSelected == 2) {
        printf("\n\n----- Selected Option 2. Doing attestation for main board -----\n");

        /*
         *  Send attestation command.
         */
        printf("(1). Creating 'Atteation' Packet\n");
        unsigned char* cmd = (unsigned char *) "1000";
        unsigned char* key = (unsigned char *) "ggK0";
        unsigned char digest[16];
        unsigned char msg[45];  // "content:<cmd><key>;encrypted:<hash value>\0"
        unsigned char* id = (unsigned char *) "12345678";   //dummy id; currently only reboot to attest needs ID
        
        // Hash ID & cmd together
        unsigned char combine[13];
        strcpy(combine, cmd);
        strcat(combine, id);
        strcat(combine, "\0");

        // Compute hash value
        lrad_hmac_md5(combine, strlen(combine), key, strlen(key), digest);
        
        // Make packet message
        strcpy(msg, "contents:");
        //printf("%d\n", strlen(msg));
        strcat(msg, cmd);
        //printf("%d\n", strlen(msg));
        strcat(msg, key);
        //printf("%d\n", strlen(msg));
        strcat(msg, id);
        //printf("%d\n", strlen(msg));
        strcat(msg, ";hash:");
        //printf("%d\n", strlen(msg));
        strcat(msg, digest);
        //printf("%d\n", strlen(msg));
        strcat(msg, "\0");
        //printf("\nAttestation message is: %s\nHash value:", msg);
        //display(digest);
        
        gettimeofday(&start, NULL);
        
        printf("\n(2). Sending Packet to RTU\n");
        writeResult = write(sockfd, msg, strlen(msg));
        if (writeResult < 0) {
            log(KRED, "WRITE API : Timeout while writing to the RTU. Please try again. \n");
            goto startconsole;
        } else {
            printf("Successfully send attestation command.\n");
        }

        /* 
         * Receive the server's reply 
         */
        printf("\n(3). Receiving packet from RTU\n");
        readResult = read(sockfd, buf_recv, BUFSIZE);
        //printf("%d\n", readResult);
        gettimeofday(&end, NULL);
        if (readResult < 0) {
            log(KRED, "READ API : Timeout while reading from the RTU. Please try again.. \n");
            goto startconsole;
        } else {
            printf("Successfully received response from the RTU  \n");
            if(strcmp(buf_recv, warningWrongMode)==0) {
                log(KRED, "Wrong Mode! Please change to Attestation mode!\n");
                return 0;
            } else if (strcmp(buf_recv, wrongCmdHash)==0) {
                log(KRED, "Wrong cmd or hash value. Please switch RTU mode and try again.\n");
                return 0;
            } else if (strlen(buf_recv) != 48) {
                log(KRED, buf_recv);
            }
        }

        /*
         *  Check for response and timing.
         */
        //if(choice==1)  // main board
        //    printf("Expected time for checksum computation is < 20 milliseconds \n");
        //else  // IO board
        //    printf("Expected time for checksum computation is < 235 milliseconds \n");
        printf("\n(3). Check result and timing\n");
        printf("Size of the response: ");
        printf("%d bytes\n", (int)strlen(buf_recv));
        printf("%s",KYEL);
        display(buf_recv);
        printf("%s",KNRM);
        
        diff_time = timedifference_msec(start, end);
        printf("Total execution time for checksum computation in milliseconds : ");
        printf("%s",KYEL);
        printf("%3.3f ", diff_time);
        printf("%s\n",KNRM);
        if ((int)diff_time > attestTimeThreshold ) {
            timeExceedLimit = 1;
            log(KRED, "Execution time falls outside the expected range \n");
        } else
            printf("Execution time falls within the expected range!\n");

        /*
         * Run simulator
         */
        printf("\n(4). Running the attestation simulator\n");
        unsigned char simulatedMainRes[48] = {0};
        SimulatorIOBoard_MainBoard(simulatedMainRes, memoryLog);
        printf("Checksum computed by simulator(in main): \n");
        printf("%s",KYEL);
        display(simulatedMainRes);
        printf("%s",KNRM);
        
        /*
         * Compare result and make judgment.
         */
        printf("\n(5). Compare results from RTU and Simulator\n");
        int s1 = strlen(buf_recv);
        int s2 = strlen(simulatedMainRes);
        if (s1 != s2) {
            printf("Size not the same.\n");
            printf("Size from RTU is: %d\n", s1);
            printf("Size from Simulator is: %d\n", s2);
            for (int i=0; i<s1; i++)
                printf("%u ", (unsigned char)buf_recv[i]);
            printf("\n");
            for (int i=0; i<s2; i++)
                printf("%u ", (unsigned char)simulatedMainRes[i]);
            printf("\n");
        } else {
            printf("Same length.\n");
            for (int i=0; i<s1; i++)
                printf("%u ", (unsigned char)buf_recv[i]);
            printf("\n");
            for (int i=0; i<s2; i++)
                printf("%u ", (unsigned char)simulatedMainRes[i]);
            printf("\n");
        }
        
        // Conclude
        printf("\n\n");
        int sameResult = 0;
        if(strcmp(buf_recv, simulatedMainRes) == 0) {
            sameResult = 1;
            log(KYEL, "The simulator and RTU have SAME values!");
        } else {
            log(KRED, "The simulator and RTU have Different values!");
        }
        if ((timeExceedLimit == 0) && (strlen(buf_recv) == 48) && sameResult)
            log(KYEL, "The attested device is free from malware !");
        else
            log(KRED, "The attested device may be infected with malware !");  
        
        sleep(3);
        close(sockfd);
        
    } else if(optionSelected == 3) { 
        printf("\n\n----- Selected Option 3. Sending the command to reboot RTU back to normal mode -----\n");

        // Send command to reboot
        printf("(1). Creating 'Reboot back to normal' Packet\n");
        unsigned char* cmd = (unsigned char *) "9999";
        unsigned char* key = (unsigned char *) "F2b9";
        unsigned char digest[16];
        unsigned char msg[45];  // "content:<cmd><key>;encrypted:<hash value>\0"
        unsigned char* id = (unsigned char *) "12345678";   //dummy id; currently only reboot to attest needs ID
        
        // Hash ID & cmd together
        unsigned char combine[13];
        strcpy(combine, cmd);
        strcat(combine, id);
        strcat(combine, "\0");

        // Compute hash value
        lrad_hmac_md5(combine, strlen(combine), key, strlen(key), digest);
        
        // Make packet message
        strcpy(msg, "contents:");
        strcat(msg, cmd);
        strcat(msg, key);
        strcat(msg, id);
        strcat(msg, ";hash:");
        strcat(msg, digest);
        strcat(msg, "\0");
        
        printf("\n(2). Sending Packet to RTU\n");
        writeResult = write(sockfd, msg, strlen(msg));
        if (writeResult < 0) {
            log(KRED, "Write API : Timeout while writing to the RTU. Please try again. \n");
            goto startconsole;
        } else {
            printf("Successfully sending reboot command (back to normal).\n");
        }
        
        printf("\n(3). Receiving packet from RTU\n");
        readResult = read(sockfd, buf_recv, BUFSIZE);
        if (readResult < 0) {
            log(KRED, "READ API : Timeout while reading from the RTU. Please try again. \n");
            goto startconsole;
        } else {
            printf("Received Message from RTU.\n");
            log(KYEL, buf_recv);
        }
        
        sleep(3);
        close(sockfd);
        
    } else if (optionSelected == 4) {
        exit(0);
        // This option is currently no in use.
        /*printf("\nSelected Option 4. Sending an exploit to compromise the Main Board. \n");

        // Send exploit to RTU
        unsigned char* cmd = (unsigned char *) "6000";
        unsigned char* key = (unsigned char *) "F2b9";
        unsigned char digest[16];
        unsigned char msg[45];  // "content:<cmd><key>;encrypted:<hash value>\0"
        unsigned char* id = (unsigned char *) "00000000";   //dummy id; currently only reboot to attest needs ID
        
        lrad_hmac_md5(cmd, strlen(cmd), key, strlen(key), digest);
        
        strcpy(msg, "contents:");
        strcat(msg, cmd);
        strcat(msg, key);
        strcat(msg, id);
        strcat(msg, ";hash:");
        strcat(msg, digest);
        strcat(msg, "\0");
        
        writeResult = write(sockfd, msg, strlen(msg));
        if (writeResult < 0) {
            log(KRED, "WRITE API : Timeout while writing to the RTU. Please try again. \n");
        }else {
            log(KBLU, "Successfully compromised the RTU.\n");
        }
        
        sleep(3);
        close(sockfd);*/
        
    } else {
        // Input other than 1 ,2 ,3, 4
        exit(0);
    }
    
    exit(0);

}

