    #include <stdio.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/ipc.h>
    #include <sys/sem.h>
    #include <sys/types.h>
    #include <pthread.h>


    /* Variablen: */

    int SemID;
    int Kasse1 = 0;
    int Kasse2 = 0;
    

    int Kasse1frei = 0;
    int Kasse2frei = 0;
    pthread_t Besucher[30];
    int numbers[30];

    /* Semaphoren */

    struct sembuf sem_p[2], sem_v[2];

    /* Prototypen: */

    void * Reisende(void *i);

    int main(void){
       unsigned short initsem;
       int y = 30;
       int j = 0;

       SemID = semget( IPC_PRIVATE, 2, IPC_CREAT | 0770 );
       
       // Vorbereitung der P und V Operationen fuer die Warteschlange

       sem_p[0].sem_num = 0; sem_v[0].sem_num = 0;
       sem_p[0].sem_op = -1; sem_v[0].sem_op = 1;
       sem_p[0].sem_flg = 0; sem_v[0].sem_flg= 0;
          
	//fuer die n Kassen:

       sem_p[1].sem_num = 1; sem_v[1].sem_num = 1;
       sem_p[1].sem_op = -1; sem_v[1].sem_op = 1;
       sem_p[1].sem_flg = 0; sem_v[1].sem_flg= 0;
       

       initsem = 0;
       semctl(SemID,0,SETVAL,3);
       semctl(SemID,1,SETALL,initsem);

       for(j=0; j<y; j++){
          numbers[j] = j+1; //[0]=1;[1]=2;[2]=3.....
       }
       
       for(j=0; j<y; j++){
          pthread_create(&Besucher[j], NULL, Reisende, &numbers[j]);
       }

       //Kassen oeffnen

       Kasse1frei = 1;
       Kasse2frei = 1;
       
       semop(SemID,&sem_v[1],1);

       //Besucher los - warten, bis alle bedient

       for(j=0; j<y; j++){
          pthread_join(Besucher[j],NULL);
       }

       semctl(SemID,0,IPC_RMID,0);
       semctl(SemID,1,IPC_RMID,0);

	printf("\nStatistik:\n[Kasse 1]: %2d \n[Kasse2 2]: %2d\n \n", Kasse1, Kasse2);
       printf("---------------------ENDE----------------------- \n");

       return 0;
    }

    void * Reisende(void *i){
       printf("[Reisender %d]:In Warteschlange... \n",*(int*)i );

       //Kasse belegen:

       semop(SemID,&sem_p[0],1);

       //Kasse ergattert


       //globale Variablen exclusiver Zugriff

       semop(SemID,&sem_p[1],1);
       if( Kasse1frei == 1 ){
          Kasse1frei = 0;

          //Globale Variablen freigeben:

          semop(SemID,&sem_v[1],1);
          sleep(1);
          
          printf("[Besucher %3d]: Wird bedient an Kasse 1\n", *(int*)i );

          //Globale Variablen sperren:

          semop(SemID,&sem_p[1],1);      
          Kasse1 += 1;
          Kasse1frei = 1;

          //Globale Variablen freigeben:

          semop(SemID,&sem_v[1],1);


          //Kasse freigeben

          semop(SemID,&sem_v[0],1);

       }else {
          Kasse2frei = 0;

          //Globale Variablen freigeben:

          semop(SemID,&sem_v[1],1);
          sleep(1);
                
          printf("[Besucher %3d]: Wird kontrolliert an Kasse 2\n", *(int*)i );

          //Globale Variablen sperren:

          semop(SemID,&sem_p[1],1);
          Kasse2 += 1;
          Kasse2frei = 1;

          //Globale Variablen freigeben:

          semop(SemID,&sem_v[1],1);


          //Kasse freigeben

          semop(SemID,&sem_v[0],1);
       return NULL;
    }
}


