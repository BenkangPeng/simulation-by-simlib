/* External definitions for time-shared computer model. */

#include "simlib.h"               /* Required for use of simlib.c. */

#define EVENT_ARRIVAL          0  /* Event type for arrival of job to CPU. */
#define EVENT_END_CPU_RUN_1    1  /* Event type for end of a CPU run. */
#define EVENT_END_CPU_RUN_2    2  /* Event type for end of a CPU run. */
#define EVENT_END_CPU_RUN_3    3  /* Event type for end of a CPU run. */
#define EVENT_END_SIMULATION   10  /* Event type for end of the simulation. */
#define LIST_QUEUE_1           11  /* List number for CPU queue. */
#define LIST_QUEUE_2           12  /* List number for CPU queue. */
#define LIST_QUEUE_3           13  /* List number for CPU queue. */
#define LIST_CPU_1             1  /* List number for CPU-1. */
#define LIST_CPU_2             2  /* List number for CPU-2. */
#define LIST_CPU_3             3  /* List number for CPU-3. */
#define SAMPST_RESPONSE_TIMES_1  1  /* sampst variable for response times. */
#define SAMPST_RESPONSE_TIMES_2  2  /* sampst variable for response times. */
#define SAMPST_RESPONSE_TIMES_3  3  /* sampst variable for response times. */
#define STREAM_THINK           1  /* Random-number stream for think times. */
#define STREAM_SERVICE         2  /* Random-number stream for service times. */

/* Declare non-simlib global variables. */

int   min_terms, max_terms, incr_terms, num_terms, num_responses,
      num_responses_required, term;
float mean_think, mean_service, quantum, swap;
FILE  *infile, *outfile;

/* Declare non-simlib functions. */

void arrive(void);
// void start_CPU_run(void);
void start_CPU_run_ucup(int id);
void end_CPU_run_ucup(int id);
// void end_CPU_run(void);
void report(void);


int main()  /* Main function. */
{
    /* Open input and output files. */

    infile  = fopen("tscomp.in",  "r");
    outfile = fopen("tscomp.out", "w");

    /* Read input parameters. */

    fscanf(infile, "%d %d %d %d %f %f %f %f",
           &min_terms, &max_terms, &incr_terms, &num_responses_required,
           &mean_think, &mean_service, &quantum, &swap);

    /* Write report heading and input parameters. */

    fprintf(outfile, "Time-shared computer model\n\n");
    fprintf(outfile, "Number of terminals%9d to%4d by %4d\n\n",
            min_terms, max_terms, incr_terms);
    fprintf(outfile, "Mean think time  %11.3f seconds\n\n", mean_think);
    fprintf(outfile, "Mean service time%11.3f seconds\n\n", mean_service);
    fprintf(outfile, "Quantum          %11.3f seconds\n\n", quantum);
    fprintf(outfile, "Swap time        %11.3f seconds\n\n", swap);
    fprintf(outfile, "Number of jobs processed%12d\n\n\n",
            num_responses_required);
    fprintf(outfile, "+-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+\n");
    fprintf(outfile, "| nbTrm | AvrRspTimeCPU1 | AvrRspTimeCPU2 | AvrRspTimeCPU3 | AvrNbQueueCPU1 | AvrNbQueueCPU2 | AvrNbQueueCPU3 |   UtilOfCPU1   |   UtilOfCPU2   |   UtilOfCPU3   |\n");
    fprintf(outfile, "+-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+\n");
    
    /* Run the simulation varying the number of terminals. */

    for (num_terms = min_terms; num_terms <= max_terms;
         num_terms += incr_terms) {

        /* Initialize simlib */

        init_simlib();

        /* Set maxatr = max(maximum number of attributes per record, 4) */

        maxatr = 4;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

        /* Initialize the non-simlib statistical counter. */

        num_responses = 0;

        /* Schedule the first arrival to the CPU from each terminal. */

        for (term = 1; term <= num_terms; ++term)
            event_schedule(expon(mean_think, STREAM_THINK), EVENT_ARRIVAL);

        /* Run the simulation until it terminates after an end-simulation event
           (type EVENT_END_SIMULATION) occurs. */

        do {

            /* Determine the next event. */

            timing();

            /* Invoke the appropriate event function. */

            switch (next_event_type) {
                case EVENT_ARRIVAL:
                    arrive();
                    break;
                case EVENT_END_CPU_RUN_1:
                    end_CPU_run_ucup(1);
                    break;
                case EVENT_END_CPU_RUN_2:
                    end_CPU_run_ucup(2);
                    break;
                case EVENT_END_CPU_RUN_3:
                    end_CPU_run_ucup(3);
                    break;
                case EVENT_END_SIMULATION:
                    report();
                    break;
            }

        /* If the event just executed was not the end-simulation event (type
           EVENT_END_SIMULATION), continue simulating.  Otherwise, end the
           simulation. */

        } while (next_event_type != EVENT_END_SIMULATION);
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}


void arrive(void)  /* Event function for arrival of job at CPU after think
                      time. */
{

    /* Place the arriving job at the end of the CPU queue.
       Note that the following attributes are stored for each job record:
            1. Time of arrival to the computer.
            2. The (remaining) CPU service time required (here equal to the
               total service time since the job is just arriving). */

    transfer[1] = sim_time;

    /* execute time using uniform distribution from 0 to 2*mean*/

    transfer[2] = uniform(0.0, 2*mean_service, STREAM_SERVICE);

    /*  add queue of CPU depend on execution time
        t <= 10.0       then    Job-1
        10 < t <= 40.0  then    Job-2
        t > 40.0        then    Job-3
    */
    if (transfer[2] <= 10.0) {
        list_file(LAST, LIST_QUEUE_1);
    } else if (transfer[2] <= 40.0) {
        list_file(LAST, LIST_QUEUE_2);
    } else if (transfer[2] > 40.0) {
        list_file(LAST, LIST_QUEUE_3);
    }

    /*  If the CPU is idle, check Queue on each CPU then start a CPU run. */

    if (list_size[LIST_CPU_1] == 0 && list_size[LIST_QUEUE_1] > 0) {
        start_CPU_run_ucup(1);
    } else if (list_size[LIST_CPU_2] == 0 && list_size[LIST_QUEUE_2] > 0) {
        start_CPU_run_ucup(2);
    } else if (list_size[LIST_CPU_3] == 0 && list_size[LIST_QUEUE_3] > 0) {
        start_CPU_run_ucup(3);
    }
}

void start_CPU_run_ucup(int id) 
{
    float run_time;

    /*  Remove the first job from the queue. 
        id = {1,2,3}
        [11] = LIST_QUEUE_1 = Queue of CPU 1
        [12] = LIST_QUEUE_2 = Queue of CPU 2
        [13] = LIST_QUEUE_3 = Queue of CPU 3
    */

    list_remove(FIRST, id+10);
    
    /* Determine the CPU time for this pass, including the swap time. */
    
    if (quantum < transfer[2])
        run_time = quantum + swap;
    else
        run_time = transfer[2] + swap;

    /* Decrement remaining CPU time by a full quantum.  (If less than a full
       quantum is needed, this attribute becomes negative.  This indicates that
       the job, after exiting the CPU for the current pass, will be done and is
       to be sent back to its terminal.) */

    transfer[2] -= quantum;

    /*  Place the job into the CPU.
        id = {1,2,3}
        [1] = LIST_CPU_1 = processing job of CPU 1
        [2] = LIST_CPU_2 = processing job of CPU 2
        [3] = LIST_CPU_3 = processing job of CPU 3
    */

    list_file(FIRST, id);

    /* Schedule the end of the CPU run. */

    event_schedule(sim_time + run_time, id);
    
}

void end_CPU_run_ucup(int id)
{
    /* Remove the job from the CPU. */

    list_remove(FIRST, id);

    /* Check to see whether this job requires more CPU time. */

    if (transfer[2] > 0.0) {

        /*  This job requires more CPU time, so place it at the end of the queue
            and start the first job in the queue. 
            id = {1,2,3}
            [11] = LIST_QUEUE_1 = Queue of CPU 1
            [12] = LIST_QUEUE_2 = Queue of CPU 2
            [13] = LIST_QUEUE_3 = Queue of CPU 3*/

        list_file(LAST, id+10);

        /*  If the CPU is idle, check Queue on each CPU then start a CPU run. */
        
        if (list_size[LIST_CPU_1] == 0 && list_size[LIST_QUEUE_1] > 0) {
            start_CPU_run_ucup(1);
        } else if (list_size[LIST_CPU_2] == 0 && list_size[LIST_QUEUE_2] > 0) {
            start_CPU_run_ucup(2);
        } else if (list_size[LIST_CPU_3] == 0 && list_size[LIST_QUEUE_3] > 0) {
            start_CPU_run_ucup(3);
        }
    }

    else {

        /*  This job is finished, so collect response-time statistics and send it
            back to its terminal, i.e., schedule another arrival from the same
            terminal. 
            count response-time depend on CPU
            [1] = SAMPST_RESPONSE_TIMES_1 = sampst variable for response-time of CPU 1
            [2] = SAMPST_RESPONSE_TIMES_2 = sampst variable for response-time of CPU 2
            [3] = SAMPST_RESPONSE_TIMES_3 = sampst variable for response-time of CPU 3
        */

        sampst(sim_time - transfer[1], id);

        event_schedule(sim_time + expon(mean_think, STREAM_THINK),
                       EVENT_ARRIVAL);

        /* Increment the number of completed jobs. */

        ++num_responses;

        /* Check to see whether enough jobs are done. */

        if (num_responses >= num_responses_required)
        {

            /* Enough jobs are done, so schedule the end of the simulation
               immediately (forcing it to the head of the event list). */

            event_schedule(sim_time, EVENT_END_SIMULATION);

        } else {

            /* Not enough jobs are done; if the queue is not empty, start
               another job. */

            if (list_size[LIST_QUEUE_1] > 0) {
                start_CPU_run_ucup(1);
            } else if (list_size[LIST_QUEUE_2] > 0) {
                start_CPU_run_ucup(2);
            }else if (list_size[LIST_QUEUE_3] > 0) {
                start_CPU_run_ucup(3);
            }
            
        }
    }
}

void report(void)  /* Report generator function. */
{
    /* Report example
        +-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
        | nbTrm | AvrRspTimeCPU1 | AvrRspTimeCPU2 | AvrRspTimeCPU3 | AvrNbQueueCPU1 | AvrNbQueueCPU2 | AvrNbQueueCPU3 |   UtilOfCPU1   |   UtilOfCPU2   |   UtilOfCPU3   |
        +-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
        | 12345 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 | 1234567890.123 |
        +-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
    */
    /* Get and write out estimates of desired measures of performance. */
    fprintf(outfile, "| %5d | %*.3f | %*.3f | %*.3f | %*.3f | %*.3f | %*.3f | %*.3f | %*.3f | %*.3f |\n", 
        num_terms, 14, sampst(0.0, -SAMPST_RESPONSE_TIMES_1), 14, sampst(0.0, -SAMPST_RESPONSE_TIMES_2), 14, sampst(0.0, -SAMPST_RESPONSE_TIMES_3),
        14, filest(LIST_QUEUE_1),14, filest(LIST_QUEUE_2), 14, filest(LIST_QUEUE_3),
        14, filest(LIST_CPU_1), 14, filest(LIST_CPU_2), 14, filest(LIST_CPU_3));
    fprintf(outfile, "+-------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+\n");
}