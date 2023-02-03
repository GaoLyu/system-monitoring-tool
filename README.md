# system-monitoring-tool
This is a C program that will report different metrics of the utilization of a given system. 
The program will work on Linux operating system.

How to use the program:
  After compiling the file using as such: 
    gcc a1.c
  You can run the program with the combinations of these command-line options:
    --samples=N
      This specifies that N samples will be collected. 
      N must be a positive integer.
      If there is no value, samples will be set to default value 10.
      If there are multiple --samples=N, samples will be set to the latest value.
    --tdelay=T
      This specifies that the time between 2 samples collected will be T seconds.
      T must be a positive integer.
      If there is no value, tdelay will be set to default value 1.
      If there are multiple --tdelay=T, tdelay will be set to the latest value.
    samples tdelay
      This specifies the samples and tdelay by treating them as positional argument in fixed order.
      There must be exactly 2 postive integers both without flag and the order is fixed.
      There can be no --samples=N nor --tdelay=T. 
    
    Some command line arguments and results:
      ./a.out
        samples=10, tdelay=1
      ./a.out --samples
        samples=10, tdelay=1
      ./a.out --samples=5
        samples=5, tdelay=1
      ./a.out --samples --tdelay=2
        samples=10, tdelay=2
      ./a.out --tdelay --samples
        samples=10, tdelay=1
      ./a.out 5 1
        samples=5, tdelay=1
      ./a.out --samples=2 --samples=3 --tdelay=2 --tdelay=3
        samples=3, tdelay=3
      ./a.out --samples 2
        samples=10, tdelay=1
      ./a.out --samples=2 10
        samples=2, tdelay=1
      ./a.out 2 3 4
        samples=10, tdelay=1
      ./a.out 2
        samples=10, tdelay=1
        
    --system
      This indicates that only the system usage should be generated.
      There will be samples samples taken with delay tdelay seconds.
      Information displayed will be:
        Number of samples, and delay between updates
        Program memory
        Utilization of the CPU
        Utilization of memory 
        General system information
    
    --user
      This indicates that only the user usage should be generated.
      There will be samples samples taken with delay tdelay seconds.
      Information displayed will be:
        Number of samples, and delay between updates
        Program memory
        Users and their connected sessions
        General system information
   
    --sequential
      This indicates that the information will be output sequentially
      
    --graphics
      This indicates that graphical representation will be generated to show the variation of data
      
    Combinations of the command line arguments are allowed and the order is flexible.
  
An overview of the functions and how I solved the problems:
  1.  To get the command line arguments, I created a function called get_command(), 
      and used the getopt_long function from getopt.h. To check positional argument, 
      isnumber() is created to check if the command is an integer.
      The result of samples and tdelay is printed using sample_tdelay().
  2.  To get memory utilization for each sample, I created struct memory that 
      stores used/total physical/virtual memory obtained by using sysinfo() from sys/sysinfo.h.
      The struct memory is stored in an array using memory().
      print_one_memory() prints the memory for a specific sample.
      print_memory() prints all the stored memory.
  3.  To get graphical representation of memory, I compare the 2 consecutive memory usage. If they differ
      by a negative value, ":" followed by "@" will be displayed. Otherwise, "|" followed by "*" will be 
      displayed. Each symbol corresponds to a 0.1 difference. print_one_memory_graphics() prints the
      memory for a specific sample. print_memory_graphics() prints all the stored memory.
  4.  To get cpu core, in cpu_core(), I opened the file /proc/stat and count the number of lines
      needed to get to the line starting with "intr". The number-1 is the cpu core.
  5.  To get cpu usage, I first created a function called cpu_usage() that opens the file /proc/stat
      and calculates the total amount of time the CPU has spent performing different kinds of work
      except the idle. The number is stored in an array. cpu_use_value() compares the cpu usage of 2
      consecutive samples. Since the first sample has nothing to compare to, the result is set to 0.
      cpu_use() prints out the cpu usage. 
  6.  To show the graphical representation of cpu usage, I first used find_base(), power() and amplify()
      to determine the position of the first significant number of the first cpu usage that is not 0. 
      Then, based on the position, certain number of "|" is displayed. Each cpu usage has at least 3 "|"
      by default which does not represent the value of the cpu usage. For example, if the cpu usage 
      of 3 samples are 0.00000%, 0.00023%, 0.00104%, then each "|" represents that the cpu usage is 
      0.0001% greater than 0, and the 3 samples will have 3, 5, 13 "|" respectively. 
      cpu_use_one_graphics() and cpu_use_graphics() prints out the result.
  7.  To get users and their connect sessions, I created a function called user_session()
      which uses setutent() and getutent() from utmp.h to get user name, line, and host.
  8.  --system is achieved in sys_opt(). After displaying essential information for each sample, 
      it sleeps for tdelay seconds using sleep(), and then refreshes the entire screen using system("clear").
      After the last sample information is displayed, it prints the general system information.
      sys_opt() keeps track of the number of samples collected, so that for the next refresh, it
      can print out all the memory information and leave blank lines for future samples. To leave blank lines
      efficiently, repeat() is created that repeats the same string n times. 
  9.  --user is achieved in user_opt(). After displaying essential information for each sample, 
      it sleeps for tdelay seconds using sleep(), and then refreshes the entire screen using system("clear").
      After the last sample information is displayed, it prints the general system information.
  0.  If both system and user information is required, the program will call all() whose logic is similar to
      sys_opt() and user_opt().
  11. --sequential is achieved in sequential(). This is similar to the above 3 functions except that there 
      is no system("clear").
  
      
  

