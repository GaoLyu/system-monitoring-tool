#include<stdio.h>
#include<stdlib.h>
#include<sys/utsname.h>
#include<sys/sysinfo.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>

struct memory{
   float phy_used,phy_tot,vir_used,vir_tot;
};

void repeat(char *string, int num){
   for(int i=0;i<num;i++){
      printf("%s",string);
   }
}

void system_info(){
   printf("-----------------------------------\n");
   printf("### System Information ###\n");
   struct utsname uts_name;
   uname(&uts_name);
   printf("System Name = %s\n",uts_name.sysname);
   printf("Machine Name = %s\n",uts_name.nodename);
   printf("Version = %s\n",uts_name.version);
   printf("Release = %s\n",uts_name.release);
   printf("Architecture = %s\n",uts_name.machine);
   printf("-----------------------------------\n");
}

// store the current memory information in 
// array memories at index i
void memory(struct memory memories[], int i){
   struct sysinfo info;
   sysinfo(&info);
   //convert byte to gitabyte
   float gb=1073741824.0;
   float p_used=(info.totalram-info.freeram)/gb;
   float p_tot=info.totalram/gb;
   float v_used=p_used+(info.totalswap-info.freeswap)/gb;
   float v_tot=p_tot+info.totalswap/gb;
   memories[i].phy_used=p_used;
   memories[i].phy_tot=p_tot;
   memories[i].vir_used=v_used;
   memories[i].vir_tot=v_tot;
}

// print all stored memory information 
// from index 0 to i (inclusive)
void print_memory(struct memory memories[], int i){
   for(int j=0;j<=i;j++){
      printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n",
         memories[j].phy_used,
         memories[j].phy_tot,
         memories[j].vir_used,
         memories[j].vir_tot);
   }
}

// print the memory information at index j
void print_one_memory(struct memory memories[], int j){
   printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n",
      memories[j].phy_used,
      memories[j].phy_tot,
      memories[j].vir_used,
      memories[j].vir_tot);
}

void user_session(){
   struct utmp *u;
   setutent();
   while((u=getutent())){
      if(u->ut_type==USER_PROCESS){
         printf("%s      %s (%s)\n",
            u->ut_user,u->ut_line,u->ut_host);
      }
   }
}

// get the first line information from /proc/stat
float cpu_usage(){
   FILE *fptr;
   int a1,a2,a3,a4,a5,a6,a7;
   fptr = fopen("/proc/stat","r");
   if(fptr==NULL){
      fprintf(stderr,"Error opening the file.");
      exit(1);
   }
   else{
      char b[1024];
      fscanf(fptr,"%s %d %d %d %d %d %d %d",
         b, &a1, &a2, &a3, &a4, &a5, &a6, &a7);
      int totaluse=a1+a2+a3+a5+a6+a7;
      int i=fclose(fptr);
      if(i!=0){
         fprintf(stderr,"Error closing the file.");
         exit(1);
      }
      else{
         return totaluse;
      }
   }
}

// get the cpu usage from 2 samples taken
// between 10 microseconds. Resukt has unit %
void cpu_use(){
   float previous=cpu_usage();
   usleep(100);
   float current=cpu_usage();
   printf("total cpu use = %.10f %% \n",(current-previous)/previous*100);
}

// get number of cores by calculating how many iterations needed
// to get to line with "intr"
int cpu_core(){
   int num=0;
   FILE *fptr;
   fptr = fopen("/proc/stat","r");

   if(fptr==NULL){
      fprintf(stderr,"Error opening the file.");
      exit(1);
   }
   else{
      char line[1024];
      fgets(line,1024,fptr);
      while(strncmp(line,"intr",4)!=0){
         fgets(line,1024,fptr);
         num++;
      }
      int i=fclose(fptr);
      if(i!=0){
         fprintf(stderr,"Error closing the file.");
         exit(1);
      }
      else{
         return num-1;
      }
   }
}

void program_usage(){
   struct rusage usage;
   getrusage(RUSAGE_SELF, &usage);
   printf("Memory usage: %ld kilobytes\n",usage.ru_maxrss);
}

bool isnumber(char string[]){
   for(int i=0;i<strlen(string);i++){
      if(!(isdigit(string[i]))){
         return false;
      }
   }
   return true;
}

void get_command(int argc, char **argv, struct option long_options[]){
   int c;
   int option_index;
   int numbers[3]={-1,-1,-1}; // used for the positional argument
   int i=0;
   
   while((c=getopt_long(argc,argv,"",long_options,&option_index))!=-1){
      switch(c){
         case 0:
            if(strcmp(long_options[option_index].name, "samples")==0){
               if(optarg){
                  *(long_options[option_index].flag)=atoi(optarg);
               }
            }

            if(strcmp(long_options[option_index].name, "tdelay")==0){
               if(optarg){
                  *(long_options[option_index].flag)=atoi(optarg);
               }
            }

      }
   }
   if(*(long_options[3].flag)==-1 && *(long_options[4].flag)==-1){
      while(optind<argc && i<3){
         if(isnumber(argv[optind])){
            numbers[i]=atoi(argv[optind]);
            i++;
         }
         optind++;
      }
      if(i==2){
         *(long_options[3].flag)=numbers[0];
         *(long_options[4].flag)=numbers[1];
      }
      else if (i==1 || i==3){
         printf("!!Unrecognized command line arguments!!\n");
      }
   }
   if(*(long_options[3].flag)!=-1 || *(long_options[4].flag)!=-1){
      while(optind<argc){
         if(isnumber(argv[optind])){
            printf("!!unrecognized command line arguments!!\n");
            break;
         }
         optind++;
      }
   }
}

void sample_tdelay(int sample,int time){
   printf("Nbr of samples: %d -- every %d secs\n",sample,time);
}

void sequential(int sample,int time,struct memory memories[],
   struct option long_options[]){
   sample_tdelay(sample,time);
   int i=0;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      printf(">>> iteration %d\n",i);
      program_usage();
      printf("-----------------------------------\n");
      //print memory
      if(*(long_options[0].flag)==1 || 
         (*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1)){
         printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
         repeat("\n",i);
         memory(memories, i);
         print_one_memory(memories,i);
         repeat("\n",sample-1-i);
         printf("-----------------------------------\n");
         printf("Number of cores: %d\n",num);
         cpu_use();
         if(*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1){
            printf("-----------------------------------\n");
         }
      }
      if(*(long_options[1].flag)==1 || 
         (*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1)){
         printf("### Sessions/users ###\n");
         user_session();
      }
      sleep(time);
   }
}

void system_opt(int sample,int time, struct memory memories[]){
   int i=0;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      system("clear");
      sample_tdelay(sample,time);
      program_usage();
      printf("-----------------------------------\n");
      printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
      memory(memories,i);
      print_memory(memories,i);
      repeat("\n",sample-1-i);
      printf("-----------------------------------\n");
      printf("Number of cores: %d\n",num);
      cpu_use();
      sleep(time);  
   }
}  

void user_opt(int sample,int time){
   int i;
   for(i=0;i<sample;i++){
      system("clear");
      sample_tdelay(sample,time);
      program_usage();
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      user_session();
      sleep(time);
   }
}

void all(int sample, int time, struct memory memories[]){
   int i;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      system("clear");
      sample_tdelay(sample,time);
      program_usage(); 
      printf("-----------------------------------\n");
      printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
      memory(memories,i);
      print_memory(memories,i);
      repeat("\n",sample-1-i);
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      user_session();
      printf("-----------------------------------\n");
      printf("Number of cores: %d\n",num);
      cpu_use();
      sleep(time);
   }
}


int main(int argc, char **argv){
	int system_flag=0;
   int user_flag=0;
   int sequential_flag=0;
   int sample=-1;
   int time=-1;
   struct option long_options[]={
      {"system",0,&system_flag,1},
      {"user",0,&user_flag,1},
      {"sequential",0,&sequential_flag,1},
      {"samples",2,&sample,10},
      {"tdelay",2,&time,1},
      {0,0,0,0}
   };
   get_command(argc,argv,long_options);
   if(sample==-1){
      sample=10;
   }
   if(time==-1){
      time=1;
   }
   // have recognized all the command line arguments
   struct memory memories[sample]; 
   

   if(sequential_flag==0){
      if(system_flag==1 && user_flag==0){
         system_opt(sample,time,memories);
      }
      
      else if(user_flag==1 && system_flag==0){
         user_opt(sample,time);
      }
      else{
         all(sample,time,memories);
      }
   }
   else{
      sequential(sample,time,memories,long_options);
   }
   system_info();
   return 0;
}
