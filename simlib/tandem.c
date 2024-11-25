/* External definitions for single-server queueing system using simlib. */

#include "simlib.h"		/* Required for use of simlib.c. */

#define EVENT_ARRIVAL_1      1	/* Event type for arrival. */
#define EVENT_DEPARTURE_1    2	/* Event type for departure. */
#define EVENT_ARRIVAL_2      3	/* Event type for arrival. */
#define EVENT_DEPARTURE_2    4	/* Event type for departure. */
#define EVENT_ENDOFTIME      5	/* Event type for end time. */
#define LIST_QUEUE_1         1	/* List number for queue. */
#define LIST_SERVER_1        2	/* List number for server. */
#define LIST_QUEUE_2         3	/* List number for queue. */
#define LIST_SERVER_2        4	/* List number for server. */
#define SAMPST_DELAYS_1      1	/* sampst variable for delays in queue. */
#define SAMPST_DELAYS_2      2	/* sampst variable for delays in queue. */
#define STREAM_INTERARRIVAL  1	/* Random-number stream for interarrivals. */
#define STREAM_SERVICE_1     2	/* Random-number stream for service 1 times. */
#define STREAM_SERVICE_2     3	/* Random-number stream for service 2 times. */
#define STREAM_TRANSIT       4	/* Random-number stream for transit times. */

/* Declare non-simlib global variables. */

int num_custs_delayed, num_delays_required;
double mean_interarrival, mean_service_1, mean_service_2, min_transit, max_transit, time_end;

FILE *infile, *outfile;


void
init_model (void)		/* Initialization function. */
{
  num_custs_delayed = 0;

  event_schedule (sim_time + expon (mean_interarrival, STREAM_INTERARRIVAL), EVENT_ARRIVAL_1);
  event_schedule (time_end, EVENT_ENDOFTIME);
}

void
arrive_1 ()			/* Arrival event function. */
{
  /* Schedule next arrival. */

  event_schedule (sim_time + expon (mean_interarrival, STREAM_INTERARRIVAL), EVENT_ARRIVAL_1);

  /* Check to see whether server is busy (i.e., list SERVER contains a
     record). */

  if (list_size[LIST_SERVER_1] == 1)
    {

      /* Server is busy, so store time of arrival of arriving customer at end
         of list LIST_QUEUE. */

      transfer[1] = sim_time;
      list_file (LAST, LIST_QUEUE_1);
    }

  else
    {

      /* Server is idle, so start service on arriving customer, who has a
         delay of zero.  (The following statement IS necessary here.) */

      sampst (0.0, SAMPST_DELAYS_1);

      /* Increment the number of customers delayed. */

      ++num_custs_delayed;

      /* Make server busy by filing a dummy record in list LIST_SERVER. */

      list_file (FIRST, LIST_SERVER_1);

      /* Schedule a departure (service completion). */

      event_schedule (sim_time + expon (mean_service_1, STREAM_SERVICE_1), EVENT_DEPARTURE_1);
    }
}

void
depart_1 (void)			/* Departure event function. */
{
  /* Check to see whether queue is empty. */

  if (list_size[LIST_QUEUE_1] == 0)

    /* The queue is empty, so make the server idle and leave the departure
       (service completion) event out of the event list. (It is currently
       not in the event list, having just been removed by timing before
       coming here.) */

    list_remove (FIRST, LIST_SERVER_1);

  else
    {

      /* The queue is nonempty, so remove the first customer from the queue,
         register delay, increment the number of customers delayed, and
         schedule departure. */

      list_remove (FIRST, LIST_QUEUE_1);
      sampst (sim_time - transfer[1], SAMPST_DELAYS_1);
      ++num_custs_delayed;
      event_schedule (sim_time + expon (mean_service_1, STREAM_SERVICE_1), EVENT_DEPARTURE_1);
    }

  /* Schedule next arrival, based uniform random */
  event_schedule (sim_time + uniform (min_transit, max_transit, STREAM_TRANSIT), EVENT_ARRIVAL_2);
}

void
arrive_2 ()			/* Arrival event function. */
{

  /* Check to see whether server is busy (i.e., list SERVER contains a
     record). */

  if (list_size[LIST_SERVER_2] == 1)
    {

      /* Server is busy, so store time of arrival of arriving customer at end
         of list LIST_QUEUE. */

      transfer[1] = sim_time;
      list_file (LAST, LIST_QUEUE_2);
    }

  else
    {

      /* Server is idle, so start service on arriving customer, who has a
         delay of zero.  (The following statement IS necessary here.) */

      sampst (0.0, SAMPST_DELAYS_2);

      /* Increment the number of customers delayed. */

      ++num_custs_delayed;

      /* Make server busy by filing a dummy record in list LIST_SERVER. */

      list_file (FIRST, LIST_SERVER_2);

      /* Schedule a departure (service completion). */
      event_schedule (sim_time + expon (mean_service_2, STREAM_SERVICE_2), EVENT_DEPARTURE_2);
    }
}

void
depart_2 (void)			/* Departure event function. */
{
  /* Check to see whether queue is empty. */

  if (list_size[LIST_QUEUE_2] == 0)

    /* The queue is empty, so make the server idle and leave the departure
       (service completion) event out of the event list. (It is currently
       not in the event list, having just been removed by timing before
       coming here.) */

    list_remove (FIRST, LIST_SERVER_2);

  else
    {

      /* The queue is nonempty, so remove the first customer from the queue,
         register delay, increment the number of customers delayed, and
         schedule departure. */

      list_remove (FIRST, LIST_QUEUE_2);
      sampst (sim_time - transfer[1], SAMPST_DELAYS_2);
      ++num_custs_delayed;
      event_schedule (sim_time + expon (mean_service_2, STREAM_SERVICE_2), EVENT_DEPARTURE_2);
    }
}



void
report (void)			/* Report generator function. */
{
  /* Get and write out estimates of desired measures of performance. */

  fprintf (outfile, "\nDelays in queue, in minutes:\n");
  out_sampst (outfile, SAMPST_DELAYS_1, SAMPST_DELAYS_2);
  fprintf (outfile, "\nQueue length (1) and server 1 utilization (2):\n");
  fprintf (outfile, "\nTransit Queue length (3) and server 2 utilization (4):\n");
  out_filest (outfile, LIST_QUEUE_1, LIST_SERVER_2);
  fprintf (outfile, "\nTime simulation ended:%12.3f minutes\n", sim_time);
}

int
main ()				/* Main function. */
{
  int ievent;
  /* Open input and output files. */

  infile = fopen ("01-15.in", "r");
  outfile = fopen ("01-15smlb.out", "w");



  /* Read input parameters. */

  fscanf (infile, "%lg %lg %lg %lg %lg %lg", &mean_interarrival, &mean_service_1,
    &mean_service_2, &min_transit, &max_transit, &time_end);

  /* Write report heading and input parameters. */
  fprintf(outfile, "Problem 1.15\n\n");
  fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n", mean_interarrival);
  fprintf(outfile, "Mean service time 1%14.3f minutes\n\n", mean_service_1);
  fprintf(outfile, "Mean service time 2%14.3f minutes\n\n", mean_service_2);
  fprintf(outfile, "Transit-time limits%14.3f%7.3f minutes\n\n", min_transit, max_transit);
  fprintf(outfile, "Length of the simulation%9.3f minutes\n\n", time_end);

  /* Set maxatr = max(maximum number of attributes per record, 4) */

  maxatr = 4;			/* NEVER SET maxatr TO BE SMALLER THAN 4. */

  /* Initialize simlib */

  init_simlib ();

  /* Initialize the model. */

  init_model ();

  /* Run the simulation while more delays are still needed. */

  while (list_size[LIST_EVENT] > 0)
    {
      /* Determine the next event. */

      timing ();

      /* Invoke the appropriate event function. */

      switch (next_event_type)
	{
	case EVENT_ARRIVAL_1:
	  arrive_1 ();
	  break;
/*	case EVENT_ARRIVAL_2:
	  arrive_2 ();
	  break;*/
	case EVENT_DEPARTURE_1:
	  depart_1 ();
	  break;
	/*case EVENT_DEPARTURE_2:
	  depart_2 ();
	  break;*/
	case EVENT_ENDOFTIME:
      for (ievent = EVENT_ARRIVAL_1; ievent <= EVENT_DEPARTURE_2; ievent++)
        event_cancel (ievent);
	  break;
	}
    }

  /* Invoke the report generator and end the simulation. */

  report ();

  fclose (infile);
  fclose (outfile);

  return 0;
}
