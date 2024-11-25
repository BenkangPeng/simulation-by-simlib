/* External definitions for time-shared computer model. */

#include "simlib.h"               /* Required for use of simlib.c. */

#define EVENT_ARRIVAL          1  /* Event type for arrival of job to CPU. */
#define EVENT_END_CPU_RUN      2  /* Event type for end of a CPU run. */
#define EVENT_END_CPU_RUN_1    3  /* Event type for end of a CPU run. */
#define EVENT_END_CPU_RUN_2    4  /* Event type for end of a CPU run. */
#define EVENT_END_CPU_RUN_3    5  /* Event type for end of a CPU run. */
#define EVENT_END_SIMULATION   6  /* Event type for end of the simulation. */
#define LIST_QUEUE             1  /* List number for CPU queue. */
#define LIST_CPU               2  /* List number for CPU. */
#define LIST_CPU_1             3  /* List number for CPU-1. */
#define LIST_CPU_2             4  /* List number for CPU-2. */
#define LIST_CPU_3             5  /* List number for CPU-3. */
#define SAMPST_RESPONSE_TIMES  1  /* sampst variable for response times. */
#define STREAM_THINK           1  /* Random-number stream for think times. */
#define STREAM_SERVICE         2  /* Random-number stream for service times. */

/* Declare non-simlib global variables. */

int   min_terms, max_terms, incr_terms, num_terms, num_responses,
      num_responses_required, term;
float mean_think, mean_service, quantum, swap;
FILE  *infile, *outfile, *inputdebug;

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
    outfile = fopen("tscomp_ucup.out", "w");
    inputdebug = fopen("tscomp_ucup.log", "w");

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
    fprintf(outfile, "Number of      Average         Average");
    // fprintf(outfile, "       Utilization\n");
    // fprintf(outfile, "terminals   response time  number in queue     of CPU");
    
    /* edit by ucup */
    fprintf(outfile, "       Utilization     Utilization     Utilization\n");
    fprintf(outfile, "terminals   response time  number in queue     of CPU1         of CPU2         of CPU3");
    /* end of editing */

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
    fclose(inputdebug);

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
    
    /* edit by ucup */
    transfer[2] = uniform(0.0, 2*mean_service, STREAM_SERVICE);
    float t2ucup = transfer[2];
    // printf("--DEBUG--\n");
    // printf("transfer[1]: %.3f\n", transfer[1]);
    // printf("transfer[2]: %.3f\n", transfer[2]);
    // printf("--DEBUG--\n");
    /* end of editing */

    list_file(LAST, LIST_QUEUE);

    /* If the CPU is idle, start a CPU run. */

    // if (list_size[LIST_CPU] == 0)
    //     start_CPU_run();

    /* edit by ucup */
    // printf("t2ucup: %.3f\n", t2ucup);
    if (t2ucup <= 10.0 && list_size[LIST_CPU_1] == 0) {
        fprintf(inputdebug, "%.3f\t%d\n", t2ucup,1);
        start_CPU_run_ucup(1);
    } else if (t2ucup > 10.0 && t2ucup <= 40.0 && list_size[LIST_CPU_2] == 0) {
        start_CPU_run_ucup(2);
        fprintf(inputdebug, "%.3f\t%d\n", t2ucup,2);
    } else if (t2ucup > 40.0 && list_size[LIST_CPU_3] == 0) {
        start_CPU_run_ucup(3);
        fprintf(inputdebug, "%.3f\t%d\n", t2ucup,3);
    }
    /* end of editing */
}

void start_CPU_run_ucup(int id) 
{
    float run_time;

    /* Remove the first job from the queue. */
    // printf("transfer[2] before: %.3f\n", transfer[2]);
    list_remove(FIRST, LIST_QUEUE);
    // printf("transfer[2] after: %.3f\n", transfer[2]);
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

    /* Place the job into the CPU. */

    // list_file(FIRST, LIST_CPU);
    
    /* add by ucup*/   
    // printf("before\n");
    // printf("LIST_CPU_1: %d\n", list_size[LIST_CPU_1]);
    // printf("LIST_CPU_2: %d\n", list_size[LIST_CPU_2]);
    // printf("LIST_CPU_3: %d\n", list_size[LIST_CPU_3]);
    if (id == 1) {
        list_file(FIRST, LIST_CPU_1);
    } else if (id == 2) {
        list_file(FIRST, LIST_CPU_2);
    } else if (id == 3) {
        list_file(FIRST, LIST_CPU_3);
    }
    // printf("after\n");
    // printf("LIST_CPU_1: %d\n", list_size[LIST_CPU_1]);
    // printf("LIST_CPU_2: %d\n", list_size[LIST_CPU_2]);
    // printf("LIST_CPU_3: %d\n", list_size[LIST_CPU_3]);
    /* end addition */

    /* Schedule the end of the CPU run. */
    if (id == 1) {
        event_schedule(sim_time + run_time, EVENT_END_CPU_RUN_1);
    } else if (id == 2) {
        event_schedule(sim_time + run_time, EVENT_END_CPU_RUN_2);
    } else if (id == 3) {
        event_schedule(sim_time + run_time, EVENT_END_CPU_RUN_3);
    }
    
}

void end_CPU_run_ucup(int id)
{
    /* Remove the job from the CPU. */

    // list_remove(FIRST, LIST_CPU);

    /* edit by ucup */
    // printf("before\n");
    // printf("LIST_CPU_1: %d\n", list_size[LIST_CPU_1]);
    // printf("LIST_CPU_2: %d\n", list_size[LIST_CPU_2]);
    // printf("LIST_CPU_3: %d\n", list_size[LIST_CPU_3]);
    if (id == 1) {
        list_remove(FIRST, LIST_CPU_1);
    } else if (id == 2) {
        list_remove(FIRST, LIST_CPU_2);
    } else if (id == 3) {
        list_remove(FIRST, LIST_CPU_3);
    }
    // printf("after\n");
    // printf("LIST_CPU_1: %d\n", list_size[LIST_CPU_1]);
    // printf("LIST_CPU_2: %d\n", list_size[LIST_CPU_2]);
    // printf("LIST_CPU_3: %d\n", list_size[LIST_CPU_3]);
    /* end of editing */

    /* Check to see whether this job requires more CPU time. */

    float t2ucup = transfer[2];
    if (transfer[2] > 0.0) {

        /* This job requires more CPU time, so place it at the end of the queue
           and start the first job in the queue. */

        list_file(LAST, LIST_QUEUE);
        if (t2ucup <= 10.0 && list_size[LIST_CPU_1] == 0) {
            start_CPU_run_ucup(1);
        } else if (t2ucup > 10.0 && t2ucup <= 40.0 && list_size[LIST_CPU_2] == 0) {
            start_CPU_run_ucup(2);
        } else if (t2ucup > 40.0 && list_size[LIST_CPU_3] == 0) {
            start_CPU_run_ucup(3);
        }
    }

    else {

        /* This job is finished, so collect response-time statistics and send it
           back to its terminal, i.e., schedule another arrival from the same
           terminal. */

        sampst(sim_time - transfer[1], SAMPST_RESPONSE_TIMES);

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

            if (list_size[LIST_QUEUE] > 0)
            {
                if (transfer[2] <= 10.0 && list_size[LIST_CPU_1] == 0) {
                    start_CPU_run_ucup(1);
                } else if (transfer[2] > 10.0 && transfer[2] <= 40.0 && list_size[LIST_CPU_2] == 0) {
                    start_CPU_run_ucup(2);
                } else if (transfer[2] > 40.0 && list_size[LIST_CPU_3] == 0) {
                    start_CPU_run_ucup(3);
                }
            }
        }
    }
}

// void end_CPU_run(void)  /* Event function to end a CPU run of a job. */
// {
//     /* Remove the job from the CPU. */

//     // list_remove(FIRST, LIST_CPU);

//     /* edit by ucup */
//     if (transfer[2] <= 10.0) {
//         list_remove(FIRST, LIST_CPU_1);
//     } else if (transfer[2] <= 40.0) {
//         list_remove(FIRST, LIST_CPU_2);
//     } else {
//         list_remove(FIRST, LIST_CPU_3);
//     }
//     /* end of editing */

//     /* Check to see whether this job requires more CPU time. */

//     if (transfer[2] > 0.0) {

//         /* This job requires more CPU time, so place it at the end of the queue
//            and start the first job in the queue. */

//         list_file(LAST, LIST_QUEUE);
//         start_CPU_run();
//     }

//     else {

//          This job is finished, so collect response-time statistics and send it
//            back to its terminal, i.e., schedule another arrival from the same
//            terminal. 

//         sampst(sim_time - transfer[1], SAMPST_RESPONSE_TIMES);

//         event_schedule(sim_time + expon(mean_think, STREAM_THINK),
//                        EVENT_ARRIVAL);

//         /* Increment the number of completed jobs. */

//         ++num_responses;

//         /* Check to see whether enough jobs are done. */

//         if (num_responses >= num_responses_required)

//             /* Enough jobs are done, so schedule the end of the simulation
//                immediately (forcing it to the head of the event list). */

//             event_schedule(sim_time, EVENT_END_SIMULATION);

//         else

//             /* Not enough jobs are done; if the queue is not empty, start
//                another job. */

//             if (list_size[LIST_QUEUE] > 0)
//                 start_CPU_run();
//     }
// }


void report(void)  /* Report generator function. */
{
    /* Get and write out estimates of desired measures of performance. */

    // fprintf(outfile, "\n\n%5d%16.3f%16.3f%16.3f", num_terms,
    //         sampst(0.0, -SAMPST_RESPONSE_TIMES), filest(LIST_QUEUE),
    //         filest(LIST_CPU));

    /* edit by ucup */
    fprintf(outfile, "\n\n%5d%16.3f%16.3f%16.3f%16.3f%16.3f", num_terms,
            sampst(0.0, -SAMPST_RESPONSE_TIMES), filest(LIST_QUEUE),
            filest(LIST_CPU_1), filest(LIST_CPU_2), filest(LIST_CPU_3));    
    /* end of editing */
}

