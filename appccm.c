//  Mobile and Pervasive Computing
//
//
//  Implementation of the algorithm:
//  "Adaptive per-user per-object cache consistency managment for mobile data access in Wireless Mesh Networks"
//  by Yiman Li and Ing-Ray Chen
//
//
//  Created by Spiros Chatzikotoulas AEM:300 sphatzik@inf.uth.gr
//  and Nick Goulias AEM:282 nigoulia@inf.uth.gr
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct hash *hashTable = NULL;
int eleCount = 0;

// struct that contains the basic data of server, mobile client and data proxie

struct node{
    int key;
    char objectID;
    int dpIds[4];
    int mcIds[4];//data proxies and mesh clients that have the item in their cache
    int flagCachingLocation; //CCM or DPM mode for the caching status CCM = 1 DPM =0
    int addressDP; // the address of the data proxy
    char timeStamp[20]; //time of the latest update of the object
    int validity; //flag if the cached object is valid or not
    struct node *next;
};



// Timestamp creator

char *time_stamp(){
    
    char *timestamp = (char *)malloc(sizeof(char) * 16);
    time_t ltime;
    ltime=time(NULL);
    struct tm *tm;
    tm=localtime(&ltime);
    
    sprintf(timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    return timestamp;
}


//struct hash for making table containing list of structs type node

struct hash {
    struct node *head;
    int count;
};


//create node and initialize its variables

struct node * createNode(int key, char objectID, int mcIds[], int dpIds[], int flagCachingLocation, int addressDP, char *timeStamp, int validity) {
    struct node *newnode;
    int i=0;
    
    newnode = (struct node *)malloc(sizeof(struct node));
    newnode->key = key;
    newnode->objectID = objectID;
    newnode->flagCachingLocation = flagCachingLocation;
    newnode->addressDP = addressDP;
    strcpy(newnode->timeStamp, time_stamp());
    newnode->validity = validity;
    
    if(mcIds!=NULL){
        
        //printf("Server Item MC\n");
        
        for(i=0;i<=3;i++){
            newnode->mcIds[i]=mcIds[i];
        }
    }
    if(dpIds!=NULL){
        
        //printf("Server Item DP\n");
        for(i=0;i<=3;i++){
            newnode->dpIds[i]=dpIds[i];
        }
    }
    newnode->next = NULL;
    return newnode;
}


//Insertion of an element in to the Hash table

void insertToHash(int key, char objectID, int mcIds[], int dpIds[], int flagCachingLocation, int addressDP, char *timeStamp, int validity) {
    int hashIndex = key % eleCount;
    
    struct node *newnode =  createNode(key, objectID, mcIds, dpIds, flagCachingLocation, addressDP, timeStamp, validity);
    /* head of list for the bucket with index "hashIndex" */
    
    if (!hashTable[hashIndex].head) {
        
        hashTable[hashIndex].head = newnode;
        hashTable[hashIndex].count = 1;
        
        return;
    }
    /* adding new node to the list */
    
    newnode->next = (hashTable[hashIndex].head);
    /*
     * update the head of the list and no of
     * nodes in the current bucket
     */
    hashTable[hashIndex].head = newnode;
    hashTable[hashIndex].count++;
    //printf("%d\n",hashTable[hashIndex].count);
    return;
}



// Validate element in Hash table

int validateElementInHash(int key, char element, char newElement, int numOfDps) {
    int hashIndex = key % eleCount, temp=0;
    struct node *myNode;
    myNode = hashTable[hashIndex].head;
    if (!myNode) {
        printf("Element unavailable in hash table\n");
    }
    
    while (myNode != NULL) {
        if ((myNode->key == key)&&(myNode->objectID == element)&&(newElement=='?')) {          // Find element in hash and validate it
            myNode->validity = 1;
            return 1;
        }
        myNode = myNode->next;
    }
    return 0;
}




// Invalidation Report

void invalidationReport(int key, char element, char newElement) {
    int hashIndex = key % eleCount, temp=0;
    struct node *myNode;
    myNode = hashTable[hashIndex].head;
    if (!myNode) {
        printf("Element unavailable in server\n");
    }

    while (myNode != NULL) {
        if (myNode->objectID == element) {          // Find the element in server and invalidate it
            myNode->validity = -1;
        }
        myNode = myNode->next;
    }
    

}



//Search function in hash table

int searchInHash(int key, char element, char newElement, int numOfDps) {
    int hashIndex = key % eleCount, flag = 0, temp=0, returnFromHash=1;
    struct node *myNode;
    myNode = hashTable[hashIndex].head;
    if (!myNode) {
        printf("Search element unavailable in hash table\n");
        returnFromHash = 0;
    }
    while (myNode != NULL) {
        
        if ((myNode->key == key)&&(myNode->objectID == element)&&(newElement=='?')) {  //search and display in MC
            
            if (myNode->validity != -1) {
            
                temp=key-numOfDps;
                //printf("the id is %d\n",temp);
                
                strcpy(myNode->timeStamp, time_stamp());
                
                printf("MC                 : %d\n", temp);
                printf("Object             : %c\n", myNode->objectID);
                printf("Address of DP      : %d\n", myNode->addressDP);
                printf("TimeStamp          : %s\n", myNode->timeStamp);
                printf("Validity           : %d\n", myNode->validity);
            
                flag = 1;
                break;
            }
            else {
                printf("Element available in hash table, but invalid!!!\n");
                return -1;
            }
        }
        
        if((myNode->key == key)&&(myNode->objectID == element)){        //search display and change object in MC
            
            printf("Object changed from %c to %c\n",element, newElement);
            temp=key-numOfDps;
            myNode->objectID=newElement;
            myNode->validity=1;
            //printf("the id is %d\n",temp);
            
            strcpy(myNode->timeStamp, time_stamp());

            
            printf("MC                 : %d\n", temp);
            printf("Object             : %c\n", myNode->objectID);
            printf("Address of DP      : %d\n", myNode->addressDP);
            printf("TimeStamp          : %s\n", myNode->timeStamp);
            printf("Validity           : %d\n", myNode->validity);
            
            flag = 1;
            break;
        }
        myNode = myNode->next;
    }
    if (!flag) {
        printf("Search element unavailable in hash table\n");
        returnFromHash = 0;
    }
    return returnFromHash;
}


// Search in Server to bring the element because MC doesn't have it.

int searchInServersHash(int key, char element, char newElement, int numOfDps) {
    int hashIndex = key % eleCount, temp=0, returnFromServer, proxyToCacheElement;
    struct node *myNode;
    myNode = hashTable[hashIndex].head;
    if (!myNode) {
        printf("Element unavailable in server\n");
        returnFromServer = 0;
    }

    while (myNode != NULL) {
        
        if ((key==0)&&(myNode->objectID == element)&&(newElement=='?')) {           // Find element in the server
            
            if (myNode->flagCachingLocation == 1) {         // CCM mode
                returnFromServer = 1;
                printf("Element found in server in CCM mode\n\n");
                break;
            }
            else if (myNode->flagCachingLocation == 0) {    // DPM mode
                proxyToCacheElement = myNode->dpIds[(rand() % numOfDps)+1];
                returnFromServer = proxyToCacheElement;
                printf("Element found in server in DPM mode\n\n");
                break;
            }
        }
        myNode = myNode->next;
    }
    return returnFromServer;
}



// Search in DP's to bring the element because it's cached there.

int searchInDPsHash(int i, char element) {
    int hashIndex = i % eleCount, elementsProxy=0;
    struct node *myNode;
    myNode = hashTable[hashIndex].head;
    if (!myNode) {
        printf("Element unavailable in DP's\n");
    }
    
    while (myNode != NULL) {
        if (myNode->objectID == element) {           // Find element in DPs
            elementsProxy = myNode->key;
            break;
        }
        myNode = myNode->next;
    }
    return elementsProxy;
}




//display objects in server, data proxies and Mobile clients

void display(int numOfDps) {
    struct node *myNode;
    int i,z=0;
    for (i = 0; i < eleCount; i++) {
        if (hashTable[i].count == 0)
            continue;
        myNode = hashTable[i].head;
        if (!myNode)
            continue;
        
            //printf("\nData at index %d in Hash Table:\n", i);
            while (myNode != NULL) {
                
                if(i==0){
                if(myNode->flagCachingLocation==0){
                        printf("object:");
                        printf("%c\n", myNode->objectID);
                        printf("---------------------------\n");
                        printf("Ids of DPs:\n");

                    for(z=0;z<=3;z++){
                        printf("%d\t",myNode->dpIds[z]);
                    }
                    printf("\n---------------------------\n");
                    printf("\n\n");
                }
                else if(myNode->flagCachingLocation==1){
                    printf("object:");
                    printf("%c\n", myNode->objectID);
                    printf("--------------\n");
                    printf("Ids of the MCs:\n");
                    
                    for(z=0;z<=3;z++){
                        printf("%d\t",myNode->mcIds[z]);
                    }
                    printf("\n---------------------------\n");
                    printf("\n\n");
                    }
                }
                else {
                    if(myNode->flagCachingLocation==0){
                        printf("DP:%d object:",myNode->key);
                        printf("%c\n", myNode->objectID);
                        printf("--------------\n");
                    }
                    else {
                        printf("MC:%d object:", (myNode->key)-numOfDps);
                        printf("%c\n", myNode->objectID);
                        printf("Caching Location:");
                        printf("DP:%d\n",myNode->addressDP);
                        printf("Last time accessed:");
                        printf("%s\n",myNode->timeStamp);
                        printf("Validation flag[-1 invalid, 1 valid, 0 not yet accessed]:");
                        printf("%d\n",myNode->validity);
                        printf("----------------------------------------------------------\n");
                        printf("\n\n");
                    }
                }
                myNode = myNode->next;
            }
    }
    return;
}


//Default exampe of server data proxie and mobile client initialization

void initServerObjectsdefault(){
    
    //srand (time(NULL));
    char letters[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    int i=0,j=0,z=0, key=0, mcIds[4], dpIds[4], flagCachingLocation=0, addressDP=0, validity=0;
    char objectID;
    
    for(i=0;i<=25;i++){  //assign DPM CCM mode in each data object
        
        if(j>=0 && j<=12){
            key=0;
            flagCachingLocation=0;
            addressDP=0;
            validity=0;
            objectID=letters[i];
            
            for(z=0;z<=3;z++){
                dpIds[z]=z+1;
                key=dpIds[z];
                //printf("DP\n");
                //printf("the key is %d\n", key);
                //printf("the ids are: %d\n",dpIds[z]);
                //printf("the flag is %d\n",flagCachingLocation);
                //printf("the object is %c\n",objectID);
                insertToHash(key, objectID, NULL, NULL, flagCachingLocation, 0, 0, 0); //store element at data proxy position key=dpIds[z]
            }
            insertToHash(0, objectID, NULL, dpIds, flagCachingLocation ,0 , 0, validity); //store element at server position key=0
        }
        else {
            objectID=letters[i];
            flagCachingLocation=1;
            
            for (z=0; z<=3; z++) {
                mcIds[z]=z+1;
                key=dpIds[z]+4;
                //printf("MC\n");
                //printf("the keys is %d\n", key);
                //printf("the ids are: %d\n",mcIds[z]);
                //printf("the flag is %d\n",flagCachingLocation);
                //printf("the object is %c\n",objectID);
                insertToHash(key, objectID, NULL, NULL, flagCachingLocation, addressDP, 0, validity); //store element at mobile client position key=dpIds[z]+#currentMClient
            }
            insertToHash(0, objectID, mcIds, NULL, flagCachingLocation, addressDP, 0, validity ); //store element at server position key=0
        }
        j++;
    }
}


//custom initialization, input provided from the user

void initServerObjectsCustom(int numOfDps, int numOfMcs, int numOfCachedObj){
    //srand (time(NULL));
    int i=0,j=0,z=0, key=0, mcIds[4], dpIds[4], flagCachingLocation=0, addressDP=0, validity=0, numOfDpsCounter=1, numOfMcsCounter=1;
    char letters[60] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        '1','2','3','4','5','6','7','8'}, objectID;

    
    for(i=0;i<=numOfCachedObj-1;i++){  //assign DPM CCM mode in each data object
        
        if(j>=0 && j<=(numOfCachedObj-1)/2){
            key=0;
            flagCachingLocation=0;
            addressDP=0;
            validity=0;
            objectID=letters[i];
            
            for(z=0;z<=3;z++){
                dpIds[z]=(rand() % numOfDps)+1;
                key=dpIds[z];
                //printf("DP\n");
                //printf("the key is %d\n", key);
                //printf("the ids are: %d\n",dpIds[z]);
                //printf("the flag is %d\n",flagCachingLocation);
                //printf("the object is %c\n",objectID);
                insertToHash(key, objectID, NULL, NULL, flagCachingLocation, 0, 0, 0); //store element at data proxy position key=dpIds[z]
            }
            insertToHash(0, objectID, NULL, dpIds, flagCachingLocation ,0 , 0, validity); //store element at server position key=0
        }
        else {
            objectID=letters[i];
            flagCachingLocation=1;
            
            for (z=0; z<=3; z++) {
                mcIds[z]=(rand() % numOfMcs)+1;
                key=mcIds[z]+numOfDps;
                //printf("MC\n");
                //printf("the keys is %d\n", key);
                //printf("the ids are: %d\n",mcIds[z]);
                //printf("the flag is %d\n",flagCachingLocation);
                //printf("the object is %c\n",objectID);
                
                insertToHash(key, objectID, NULL, NULL, flagCachingLocation, addressDP, 0, validity); //store element at mobile client position key=dpIds[z]+#currentMClient
            }
            insertToHash(0, objectID, mcIds, NULL, flagCachingLocation, addressDP, 0, validity ); //store element at server position key=0
        }
        j++;
    }
    printf("---------------------------------------------\n\n");
}



// Adaptive per-user per-object cache consistency managment for mobile data access in WMN algorithm

void appccmAlgorithm(int key, char element, char newElement, int numOfDps, int elementFoundInHash) {
    int broughtFromServer, flagCachingLocation, validationReport, proxyWithTheElement, i;
    
    if (elementFoundInHash == 1) {
    
        validationReport = validateElementInHash(key, element, newElement, numOfDps);
        
        if (validationReport){
            printf("Object found in MC\n");
        }
        else {
            for (i=1; i<=numOfDps; i++) {
                proxyWithTheElement = searchInDPsHash(i, element);
            }
            printf("Proxy that (also) has the element: %d\n",proxyWithTheElement);  // Find proxy that has the element
            
            flagCachingLocation = 1;
            insertToHash(key, element, NULL, NULL, flagCachingLocation, 0, 0, 1);   // Store element in MC and validate it
            printf("MC's cache is updated again\n");
        }
    }
    else {
        broughtFromServer = searchInServersHash(0, element, newElement, numOfDps);         // Bring object from server
            
        if (broughtFromServer == 1) {               // CCM mode selected
            flagCachingLocation = 1;
            printf("MC's cache is updated with the requested element at: %s\n\n",time_stamp());
            printf("---------------------------------------------------------\n");

        }
        else {                                      // DPM mode selected
            flagCachingLocation = 0;
            printf("Element will be cached in DP: %d\n",broughtFromServer+1);           // Store object in DP
            insertToHash(broughtFromServer+1, element, NULL, NULL, flagCachingLocation, 0, 0, 0);
            printf("DP's cache updated at: %s\n",time_stamp());
        }
        insertToHash(key, element, NULL, NULL, flagCachingLocation, 0, 0, 1);
        printf("MC's status updated at: %s\n",time_stamp());
    }
}




//  Selection Input


int main() {
    int n, key, numOfCachedObj, numOfDps, numOfMcs, sum, elementFound;
    char selection, element, newElement, ch;
    
    printf("Enter Y/N if you want to run the default example\n");
    scanf("%c",&selection);
    
    if(selection=='Y' || selection=='y'){
        
        printf("--------Executing default example--------\n");
        numOfDps=4;
        numOfMcs=4;
        sum=numOfMcs+numOfDps+1;
        eleCount = sum;
        hashTable = (struct hash *)calloc(sum, sizeof (struct hash));
        printf("the index of hash table is %d\n",eleCount);

        initServerObjectsdefault();
    }
    else if(selection=='N'|| selection=='n'){
        
        printf("--------Executing custom example--------\n");
        
        while(1){
            printf("Enter number of MC's and press enter\n");
            printf("Note: enter a number between 1 & 24 \n");
            scanf("%d",&numOfMcs);
            if(numOfMcs>=1 && numOfMcs<=24){break;}
            else{
                printf("Wrong input\n");
                numOfMcs=0;
            }
        }
        while(1){
            printf("Enter number of DP's and press enter\n");
            printf("Note: enter a number between 1 & 24 \n");  
            scanf("%d",&numOfDps);
            if(numOfDps>=1 && numOfDps<=24){break;}
            else{
                printf("Wrong input\n");
                numOfDps=0;
            }
        }
        while(1){
            printf("Enter number of data objects and press enter\n");
            printf("Note: enter a number between 12 &  60\n");
            scanf("%d",&numOfCachedObj);
            if(numOfCachedObj>=12 && numOfCachedObj<=60){break;}
            else{
                printf("Wrong input\n");
                numOfCachedObj=0;
            }
        }
        printf("----------------------------------------\n");
        
        sum=numOfMcs+numOfDps+1;
        
        eleCount = sum;
        
        printf("the index of hash table is %d\n\n",eleCount);
        /* create hash table with "n" no of buckets */
        hashTable = (struct hash *)calloc(sum, sizeof (struct hash));
        
        initServerObjectsCustom(numOfDps,numOfMcs,numOfCachedObj);
    }
    else {
        printf("Wrong input\n");
    }
    
    while (1) {
        printf("1. Display Data of the MCs, DPs and the Server in the WMN\n");
        printf("2. Choose an MC and fetch an element\n");
        printf("3. Choose an MC and change an element\n");
        printf("4. Exit\n");
        printf("Enter your choice:");
        scanf(" %c", &ch);
        switch (ch) {
            case '1':
                display(numOfDps);
                break;
            case '2':
                printf("Enter the ID of the MC to perform search: ");
                scanf("%d", &key);
                printf("Enter the element that you want to fetch: ");
                scanf(" %c", &element);
                newElement='?';
                elementFound = searchInHash(key+numOfDps, element, newElement, numOfDps);
                appccmAlgorithm(key+numOfDps, element, newElement, numOfDps, elementFound);
                break;
            case '3':
                printf("Enter the ID of the MC to perform search:");
                scanf("%d", &key);
                printf("Enter the element that you want to change:");
                scanf(" %c", &element);
                printf("Enter the new element that you want to add:");
                scanf(" %c", &newElement);
                searchInHash(key+numOfDps,element, newElement, numOfDps);
                for (key=0; key < numOfDps+numOfMcs; key++) {
                    invalidationReport(key, element, newElement);
                }
                printf("-------------------------------------------\n");
                printf("The object %c was invalidated at %s\n",element, time_stamp());
                printf("-------------------------------------------\n\n\n");
                break;
            case '4':
                exit(0);
            default:
                printf("You have entered wrong option!!!\n");
                break;
        }
    }
    return 0;
}
