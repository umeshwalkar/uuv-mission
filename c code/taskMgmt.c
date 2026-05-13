#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
//#include "interfaceDefines.h"
//#include "interfaceExterns.h"
#include "taskMgmt.h"

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct
{
	pthread_t threadId;
	pthread_attr_t threadAttr;
	void (*entryPoint)(void);
	sem_t semId;
	uint16_t priority;
	uint8_t interval;

	TICKS prevtick;

} TCB_t; // TaskControlBlock

/*GLOBALS*/
int stopMainThread = 0;
TCB_t tcb[NUM_OF_THREADS];
unsigned int tcbCnt = 1; //0th position is reserved

TICKS schedulerTick = 0;
timer_t mainSchedTimerId;
sem_t mainThreadSchedSem;
int cntr;

/*FORWARD DECLARATIONS*/
void mainThread(void);
void mainTimerHndlr(int sig, siginfo_t *si, void *uc);
//==========================================================

/*****************************************************************************
 **  task_wrapper is a pthread used to 'contain' a application task.
 *****************************************************************************/
void *task_wrapper(TCB_t *task)
{
	errno = 0;
	int status = 0;

	(*(task->entryPoint))();
	//  If for some reason the task above DOES return, clean up the  pthread and task resources and kill the pthread.

	printf("\npthread_cancellation request for thread id= %ld", pthread_self());
	status = pthread_cancel(pthread_self());
	printf(" pthread_cancel status %d", status);

	printf("\nstopping main thread\n");
	stopMainThread = 1;
	return NULL;
}

void mainTimerHndlr(int sig, siginfo_t *si, void *uc)
{
	schedulerTick++;
	//	printf("time epalsed =%d\n", sysSchedCount);

	sem_post(&mainThreadSchedSem);
	//	sem_post(&mainThreadSchedSem2);

	for (cntr = 1; cntr < tcbCnt; cntr++) // tcbCnt
	{
		if (GetTaskTickDiff(tcb[cntr].prevtick) >= tcb[cntr].interval)
		{
			tcb[cntr].prevtick = GetTaskTick();
			sem_post(&tcb[cntr].semId);
		}
	}

	/*release user app semaphores*/
	//	releaseAppSems();
}

void mainThread(void)
{
	struct sigaction mainTimerSa;
	sigset_t mainTimerSigMask;
	struct sigevent mainTimerSigEv;
	struct itimerspec mainTimerSpecs;
	//	unsigned int sysSchedCountPrev = 0;

	printf("mainThread started\n");

	if (sem_init(&mainThreadSchedSem, 0, 0) == -1)
	{
		errExit("sem_init");
	}

	/*Establish handler for timer signal*/
	mainTimerSa.sa_flags = SA_SIGINFO;
	mainTimerSa.sa_sigaction = mainTimerHndlr;
	sigemptyset(&mainTimerSa.sa_mask);

	if (sigaction(MAIN_SCHED_TMR_SIG, &mainTimerSa, NULL) == -1)
	{
		errExit("sigaction");
	}

	sigemptyset(&mainTimerSigMask);
	sigaddset(&mainTimerSigMask, MAIN_SCHED_TMR_SIG);

	if (sigprocmask(SIG_SETMASK, &mainTimerSigMask, NULL) == -1)
	{
		errExit("sigprocmask");
	}

	mainTimerSigEv.sigev_notify = SIGEV_SIGNAL;
	mainTimerSigEv.sigev_signo = MAIN_SCHED_TMR_SIG;
	mainTimerSigEv.sigev_value.sival_ptr = &mainSchedTimerId;

	/*Create the main timer*/
	if (timer_create(CLOCK_REALTIME, &mainTimerSigEv, &mainSchedTimerId) == -1)
	{
		errExit("timer_create");
	}

	/*Populate the timer struct values*/
	mainTimerSpecs.it_value.tv_sec = 0;
	mainTimerSpecs.it_value.tv_nsec = BASE_SCHEDULE_INTERVAL * 1000000;
	mainTimerSpecs.it_interval.tv_sec = 0;
	mainTimerSpecs.it_interval.tv_nsec = BASE_SCHEDULE_INTERVAL * 1000000;

	/*Set time & start timer*/
	if (timer_settime(mainSchedTimerId, 0, &mainTimerSpecs, NULL) == -1)
	{
		errExit("timer_settime");
	}

	/*allow user defined task to create*/
	//	createAppTasks();
	int i = 1, status;
	unsigned long mask = 1;
	int policy = 0;
	struct sched_param param;
	for (i = 1; i < tcbCnt; i++)
	{
#if 1
		status = pthread_attr_init(&tcb[i].threadAttr);
		if (status != 0)
		{
			printf("Error in Thread Attributes init\n");
			stopMainThread = 1;
			break;
		}

		pthread_attr_setdetachstate(&tcb[i].threadAttr, PTHREAD_CREATE_DETACHED);

		pthread_attr_setscope(&tcb[i].threadAttr, PTHREAD_SCOPE_SYSTEM);

		status = pthread_attr_setschedpolicy(&tcb[i].threadAttr, SCHED_FIFO);

		if (status != 0)
		{
			printf("Error in Thread Attributes setting policy \n");
			stopMainThread = 1;
			break;
		}

		status = pthread_attr_setinheritsched(&tcb[i].threadAttr,
				PTHREAD_EXPLICIT_SCHED);
		if (status != 0)
		{
			printf("Error in Thread Attributes setting inherit sched \n");
			stopMainThread = 1;
			break;
		}

		/* safe to get existing scheduling param */
		status = pthread_attr_getschedparam(&tcb[i].threadAttr, &param);

		if (status != 0)
		{
			printf("Error in Thread Attributes get schedule params\n");
			stopMainThread = 1;
			break;
		}

		/* set the priority; others are unchanged */
		param.__sched_priority = tcb[i].priority;

		/* setting the new scheduling param */
		status = pthread_attr_setschedparam(&tcb[i].threadAttr, &param);

		if (status != 0)
		{
			printf("Error in Thread Attributes setting schedule param\n");
			stopMainThread = 1;
			break;
		}

		mask = 1;

		/* bind process to processor 0 */
		if (pthread_attr_setaffinity_np(&tcb[i].threadAttr, sizeof(mask),
				(cpu_set_t*) &mask) < 0)
		{
			printf("pthread_setaffinity_np - not set");
		}

		pthread_attr_getaffinity_np(&tcb[i].threadAttr, sizeof(mask),
				(cpu_set_t*) &mask);

		if (mask != 1)
		{
			printf("Affinity not set properly to processor 1\n");
			stopMainThread = 1;
			break;
		}

#endif
		status = pthread_create(&tcb[i].threadId, &tcb[i].threadAttr,
				(void*) task_wrapper, (void *) &tcb[i]);
		if (status != 0)
		{
			handle_error_en(status, "pthread_create");
			printf("\n task id #%d Could not be spawned\nApplication Exit!!\n",
					i);
			//			timer_delete(mainSchedTimerId);
			stopMainThread = 1;
			break;
		}

#if 1
		status = pthread_getschedparam(tcb[i].threadId, &policy, &param);
		if (status != 0)
		{
			printf("Error in Thread get schedule params\n");
			stopMainThread = 1;
			break;
		}
#endif
	} // for(;;)

	while (!stopMainThread)
	{
		if (sem_wait(&mainThreadSchedSem) == -1)
		{
			errExit("sem_wait: mainThreadSchedSem");
		}

		//Do nothing; Go back to wait for sempahore
		//		if (sysSchedCount - sysSchedCountPrev >= 100)
		//		{
		//			sysSchedCountPrev = sysSchedCount;
		//			printf("\n***** main thread****** \n");
		//		}

		//		for (cntr = 1; cntr < tcbCnt; cntr++) // tcbCnt
		//		{
		//			if (GetTaskTickDiff(tcb[cntr].prevtick) >= tcb[cntr].interval)
		//			{
		//				tcb[cntr].prevtick = GetTaskTick();
		//				sem_post(&tcb[cntr].semId);
		//			}
		//		}
	}

	// stop timer
	timer_delete(mainSchedTimerId);
	errExit("***See you again***");
}

//============================================
void TaskSchdlInit()
{
	printf("\n\nTotal Tasks #%d\n\n", tcbCnt);

	/*Create main thread*/
	tcb[0].entryPoint = &mainThread;
	pthread_create(&tcb[0].threadId, &tcb[0].threadAttr, (void*) task_wrapper,
			(void *) &tcb[0]);
	pthread_join(tcb[0].threadId, NULL);
}

int TaskCreate(void(*entryPoint)(void), uint16_t priority,
		uint16_t interval_msec, int8_t *handler)
{
	// check if scheduler is initialized
	//	if (tcbCnt == 0)
	//		TaskSchdlInit();

	*handler = -1;

	if (tcbCnt >= NUM_OF_THREADS)
		return -1;

	tcb[tcbCnt].entryPoint = entryPoint;
	tcb[tcbCnt].interval = (uint16_t) (interval_msec / 10); // timer tick is 10msec

	tcb[tcbCnt].priority = priority;
	//	pthread_create(&tcb[tcbCnt].threadId, &tcb[tcbCnt].threadAttr, tcb[tcbCnt].entryPoint, NULL);
	tcb[tcbCnt].prevtick = 0;

	tcbCnt++;

	*handler = tcbCnt;

	return (1);
}

void errExit(const char* msg)
{
	printf("\n%s\n", msg);
	exit(1);
}

int semCreate(int8_t taskHandler)
{
	taskHandler--;

	if ((taskHandler < 0) || (taskHandler >= tcbCnt))
		return -1;

	return sem_init(&tcb[taskHandler].semId, 0, 0);

	//	if (sem_init(&tcb[taskHandler].semId, 0, 0) == -1)
	//		return 0;
	//
	//	return 1;

}

int semTake(int8_t taskHandler)
{
	taskHandler--;

	if ((taskHandler < 0) || (taskHandler >= tcbCnt))
		return -1;

	return sem_wait(&tcb[taskHandler].semId);

	//	if (sem_wait(&tcb[taskHandler].semId) == -1)
	//		return 0;
	//
	//	return 1;
}

int semGive(int8_t taskHandler)
{
	taskHandler--;

	if ((taskHandler < 0) || (taskHandler >= tcbCnt))
		return -1;

	return sem_post(&tcb[taskHandler].semId);

	//	if (sem_post(&tcb[taskHandler].semId) == -1)
	//		return 0;
	//
	//	return 1;
}
