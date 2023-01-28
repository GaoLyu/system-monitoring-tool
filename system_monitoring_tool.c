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

void repeat(char *string, int num){
   for(int i=0;i<num;i++){
      printf("%s\n",string);
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
}

void memory(){
   struct sysinfo info;
   sysinfo(&info);
   float phy_used=(info.totalram-info.freeram)/(float)1073741824;
   float phy_tot=info.totalram/(float)1073741824;
   float vir_used=phy_used+(info.totalswap-info.freeswap)/(float)1073741824;
   float vir_tot=phy_tot+info.totalswap/(float)1073741824;

   printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n",phy_used,phy_tot,vir_used,vir_tot);
}
void samples(int size, unsigned time){
   
   
   for(int i=0;i<size;i++){
      
      memory();
      sleep(time);
   }
}

void user_session(){
   struct utmp *u;
   while((u=getutent())){
      if(u->ut_type==USER_PROCESS){
         printf("%s      %s (%s)\n",u->ut_user,u->ut_line,u->ut_host);
      }
   }
}
// convert character digit to int
int convert(char n){
   int result=n-'0';
   return result;
}

//cpu usage of one sample in percentage
float cpu_usage(){
   FILE *fptr;
   fptr = fopen("/proc/stat","r");
   char line[1024];
   fgets(line,1024,fptr);
   int totaluse=convert(line[1])+convert(line[2])+convert(line[3])+convert(line[4]);
   float usage=100.0*convert(line[1])/totaluse;
   fclose(fptr);
   return usage;
}

void cpu_use(){
   float previous=cpu_usage();
   float current=cpu_usage();
   printf("total cpu use = %.10f%%\n",current-previous);
}

//number of cores
void cpu_core(){
   FILE *fptr;
   fptr = fopen("/proc/stat","r");
   char line1[1024];
   char line2[1024];
   fgets(line1,1024,fptr);
   fgets(line2,1024,fptr);
   while(strncmp(line2,"intr",4)!=0){
      strcpy(line1,line2);
      fgets(line2,1024,fptr);
   }
   printf("Number of cores: %d\n",convert(line1[3])+1);
   fclose(fptr);
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
   int numbers[3]={-1,-1,-1};
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
         printf("~~~~~~~~~~~~~~~~~~~~~\nunrecognized command line arguments");
      }
   }
   if(*(long_options[3].flag)!=-1 || *(long_options[4].flag)!=-1){
      while(optind<argc){
         if(isnumber(argv[optind])){
            printf("~~~~~~~~~~~~~~~~~~~~~\nunrecognized command line arguments");
            break;
         }
         optind++;
      }
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
   


   
   if(system_flag==1){
      system_info();
   }
   if(user_flag==1){
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      user_session();
   }
   if(system_flag==1 || user_flag==1){
      if(sample!=-1 || time!=-1){
         if(sample==-1){
            sample=10;
         }
         if(time==-1){
            time=1;
         }
      }
      samples(sample,time);
   }
   

   if(system_flag==0 && user_flag==0){
      // system information
      system_info(); //unchange
      // program usage
      program_usage();
      // CPU
      printf("-----------------------------------\n");
      cpu_core(); //unchange
      cpu_use();


      // user and session
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      user_session();

      // Memory
      printf("-----------------------------------\n");
      printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

      if(sample==-1){
         sample=10;
      }
      if(time==-1){
         time=1;
      }
      samples(sample,time);
   }

   return 0;
}
