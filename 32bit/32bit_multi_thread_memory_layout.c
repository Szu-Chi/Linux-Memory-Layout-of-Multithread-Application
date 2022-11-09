#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define ARRAY_SIZE 16
#define MALLOC_SIZE 32
#define THREAD_NUM 3

#define __NR_gettid 224
#define __NR_hubert 385
#define __NR_getms 386
#define __NR_get_phy_addr 387

typedef struct thread_data_package{
    int id;
    char* malloc_ptr;
}td_pack;

extern char **environ;

char global_array[ARRAY_SIZE];
char global_var;
char global_var_with_init=150;

void *thread_func(void*);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


long hubert_syscall(void)
{
	return syscall(__NR_hubert);
}

long getms_syscall(int pid)
{
        return syscall(__NR_getms, pid);
}

long get_phy_addr_syscall(int pid, unsigned int vaddr)
{
        return syscall(__NR_get_phy_addr, pid, vaddr);
}


int main(int argc, char* argv[]){
    int pid =  syscall(__NR_gettid);
    long activity;
    (void)printf("main tid: %d\n", syscall(__NR_gettid));
    char main_var;
    char main_var_with_init = 10;
    char main_array[ARRAY_SIZE];
    char *main_ptr;
    main_ptr = (char *)malloc(MALLOC_SIZE);

    (void)printf("High address                                                 \n");
    (void)printf("--------------------------(args and env)---------------------\n");
    (void)printf("environ[0] at        : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)environ, (unsigned int)get_phy_addr_syscall(pid, &environ));

    (void)printf("-----------------------------Stack---------------------------\n");
    (void)printf("main_var             : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&main_var, (unsigned int)get_phy_addr_syscall(pid, &main_var));
    (void)printf("main_array           : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&main_array, (unsigned int)get_phy_addr_syscall(pid, &main_array));
    (void)printf("main_var_with_init   : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&main_var_with_init, (unsigned int)get_phy_addr_syscall(pid, &main_var_with_init));

    (void)printf("-----------------------------Lib-----------------------------\n");
    (void)printf("printf library func  : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&printf, (unsigned int)get_phy_addr_syscall(pid, &printf));
    (void)printf("malloc library func  : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&malloc, (unsigned int)get_phy_addr_syscall(pid, &malloc));

    (void)printf("-----------------------------HEAP----------------------------\n");
    (void)printf("main_malloc_end      : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)main_ptr+MALLOC_SIZE, (unsigned int)get_phy_addr_syscall(pid, main_ptr+MALLOC_SIZE));
    (void)printf("main_malloc_start    : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)main_ptr, (unsigned int)get_phy_addr_syscall(pid, main_ptr));

    (void)printf("-----------------------------BSS-----------------------------\n");
    (void)printf("global_array         : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)global_array, (unsigned int)get_phy_addr_syscall(pid, global_array));
    (void)printf("global_var           : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&global_var, (unsigned int)get_phy_addr_syscall(pid, &global_var));

    (void)printf("-----------------------------Data----------------------------\n");
    (void)printf("global_var_with_init : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&global_var_with_init, (unsigned int)get_phy_addr_syscall(pid, &global_var_with_init));

    (void)printf("-----------------------------Text----------------------------\n");
    (void)printf("thread_func          : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&thread_func, (unsigned int)get_phy_addr_syscall(pid, &thread_func));
    (void)printf("main_func            : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&main, (unsigned int)get_phy_addr_syscall(pid, &main));

    pthread_attr_t attr;
    pthread_attr_init(&attr);


    pthread_t tid[THREAD_NUM];
    td_pack  pack[THREAD_NUM];
    for(int i=0; i<THREAD_NUM; i++){
        pack[i].id = i;
        td_pack *pack_ptr = &pack[i];
        pthread_create(&tid[i], &attr, thread_func, (void*)pack_ptr);
    }

    for(int i=0; i<THREAD_NUM; i++){
        pthread_join(tid[i], NULL);
    }

    activity = hubert_syscall();
    printf("%ld\n", activity);
    if(activity < 0)
    {
	perror("Sorry, Hubert. Your system call appers to have failed\n");
    }
    else
    {
	printf("Congratulations, Hubert! Your system call is functional. Run the command dmesg in the terminal and find out! \n");
    }

    activity = getms_syscall(pid);
    printf("%ld\n", activity);
    if(activity < 0)
    {
        perror("Sorry, Hubert. Your system call appers to have failed\n");
    }
    else
    {
       printf("Congratulations, Hubert! Your system call is functional. Run the command dmesg in the terminal and find out! \n");
    }

    free(main_ptr);

    //fgetc(stdin);
    return EXIT_SUCCESS;
}


void *thread_func(void* data){
    char func_var;
    td_pack* pack = (td_pack*)data;
    int id = pack->id;
    int pid =  syscall(__NR_gettid);
    pack->malloc_ptr = (char*)malloc(MALLOC_SIZE);

    pthread_mutex_lock(&mutex1);
    (void)printf("\n\nThread %d (tid %d)\n", id, syscall(__NR_gettid));

    (void)printf("High address                                                 \n");
    (void)printf("--------------------------(args and env)---------------------\n");
    (void)printf("environ[0] at        : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)environ, (unsigned int)get_phy_addr_syscall(pid, environ));

    (void)printf("-----------------------------Stack---------------------------\n");
    (void)printf("func_var             : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&func_var, (unsigned int)get_phy_addr_syscall(pid, &func_var));

    (void)printf("-----------------------------Lib-----------------------------\n");
    (void)printf("printf library func  : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&printf, (unsigned int)get_phy_addr_syscall(pid, &printf));
    (void)printf("malloc library func  : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&malloc, (unsigned int)get_phy_addr_syscall(pid, &malloc));

    (void)printf("-----------------------------HEAP----------------------------\n");
    (void)printf("thread_malloc_end    : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)pack->malloc_ptr+MALLOC_SIZE, (unsigned int)get_phy_addr_syscall(pid, pack->malloc_ptr+MALLOC_SIZE));
    (void)printf("thread_malloc_start  : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)pack->malloc_ptr, (unsigned int)get_phy_addr_syscall(pid, pack->malloc_ptr));

    (void)printf("-----------------------------BSS-----------------------------\n");
    (void)printf("global_array         : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)global_array, (unsigned int)get_phy_addr_syscall(pid, global_array));
    (void)printf("global_var           : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&global_var, (unsigned int)get_phy_addr_syscall(pid, &global_var));

    (void)printf("-----------------------------Data----------------------------\n");
    (void)printf("global_var_with_init : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&global_var_with_init, (unsigned int)get_phy_addr_syscall(pid, &global_var_with_init));

    (void)printf("-----------------------------Text----------------------------\n");
    (void)printf("thread_func          : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&thread_func, (unsigned int)get_phy_addr_syscall(pid, &thread_func));
    (void)printf("main_func            : 0x%08x ----> physical address = 0x%08x\n", 
                (unsigned int)&main, (unsigned int)get_phy_addr_syscall(pid, &main));
    pthread_mutex_unlock(&mutex1);
}
