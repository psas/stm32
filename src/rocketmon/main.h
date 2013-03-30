//defines
#define BUF_SIZE            1024     // buffer size in bytes
#define PORT_NUM            31337     // Port number 
#define PEND_CONNECTIONS     100     // pending connections to hold 
#define TRUE                   1
#define FALSE                  0
#define NTHREADS 5                     /* Number of child threads        */ 
#define NUM_LOOPS  10                  /* Number of local loops          */
#define SCHED_INTVL 5                  /* thread scheduling interval     */
#define HIGHPRIORITY 10
//function prototypes
int serverInit();
