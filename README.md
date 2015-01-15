# sneakymalloc
Program that hooks malloc/free and dumps memory contents to a file when the memory is 'freed'. 

To test it you need to have your shared object loaded before the real malloc/free:
 
   LD_PRELOAD=/path/to/sneakymalloc.so 
   
   So, for example, in the directory with the make file and src file run the following commands:
      
      make
      LD_PRELOAD=./sneakymalloc.so /bin/ls # Note - it's better to use absolute path for sneakymalloc.so
      ls -ltr /tmp/
      
  You should see a file memspy.xxxx.dat, where xxxx is the pid # of the process. 
