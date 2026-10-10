#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

// total jobs
int numofjobs = 0;

struct job {
    // job id is ordered by the arrival; jobs arrived first have smaller job id, always increment by 1
    int id;
    int arrival; // arrival time; safely assume the time unit has the minimal increment of 1
    int length;
    int time_ran; // Accumulated time it has ran for 
    int start_time; // The time the job is first scheduled
    int completion_time; // The time the job is completed
    int last_ran; // The time the job was last ran.
    int wait_time; // Accumulated wait time.
    int tickets; // number of tickets for lottery scheduling 
    // TODO: add any other metadata you need to track here
    struct job *next;

    // Used to track current statistics for the tests. Reset after printing.
    int current_time_ran;
    int current_start_time;
};

// the workload list
struct job *head = NULL;

/**
 * Find a job that can be run and has not been completed
 */
int get_job(struct job **job, int time);

/**
 * Update the state of a job based on the current time, and return time - 1
 */
int cpu_tick(struct job *job, int time);

/**
 * Prints a formatted line expected in the tests.
 */
void print_status(struct job *job);

/**
 * Prints formatted lines expected in the analysis tests.
 */
void print_analysis();

/**
 * Checks if all jobs have been completed
 */
bool check_done();


void append_to(struct job **head_pointer, int arrival, int length, int tickets){
    struct job *cur = *head_pointer;
    struct job *prev = NULL;

    int id = 0;
    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
        id++;
    }

    cur = malloc(sizeof(struct job));
    cur->id = id;
    cur->arrival = arrival;
    cur->length = length;
    cur->time_ran = 0;
    cur->tickets = tickets;
    cur->next = NULL;

    cur->start_time = -1;
    cur->completion_time = -1;
    cur->last_ran = -1;
    cur->wait_time = 0;

    cur->current_time_ran = 0;
    cur->current_start_time = -1;

    if (prev != NULL) {
        prev->next = cur;
    }
    else {
        *head_pointer = cur;
    }

    numofjobs++;
}


void read_job_config(const char* filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int tickets  = 0;

    char* delim = ",";
    char *arrival = NULL;
    char *length = NULL;

    // TODO, error checking
    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // TODO: if the file is empty, we should just exit with error
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if( line[read-1] == '\n' )
            line[read-1] =0;
        arrival = strtok(line, delim);
        length = strtok(NULL, delim);
        tickets += 100;

        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    fclose(fp);
    if (line) free(line);

    if (numofjobs == 0) {
        fprintf(stderr, "Error: the job file is empty.\n");
        exit(EXIT_FAILURE);
    }
}


void policy_SJF()
{
    printf("Execution trace with SJF:\n");

    // TODO: implement SJF policy

    printf("End of execution with SJF.\n");

}


void policy_STCF()
{
    printf("Execution trace with STCF:\n");

    struct job *cur_job;

    struct job *prev_job = NULL;

    int t = 0; 

    while (1) { 
        // Get a valid (but not necessarily best) job
        while (get_job(&cur_job, t) < 0) {
            t++;
        } 

        // Get the next job
        for (struct job *job = head; job != NULL; job = job->next) {
            if (job->arrival > t) continue;
            else if (job->completion_time >= 0) continue;

            int time_left = job->length - job->time_ran;

            if (time_left < (cur_job->length - cur_job->time_ran)) {
                cur_job = job;
            }
        }

        if (prev_job == cur_job) {
            prev_job->current_time_ran++;
        }
        else {
            if (prev_job != NULL) {
                print_status(prev_job);
                prev_job->current_time_ran = 0;
            }

            cur_job->current_time_ran = 1;
            cur_job->current_start_time = t;
        }

        t = cpu_tick(cur_job, t);

        if (check_done()) break;

        prev_job = cur_job;
    }

    print_status(cur_job);

    printf("End of execution with STCF.\n");
}


void policy_RR(int slice)
{
    printf("Execution trace with RR:\n");

    // TODO: implement RR policy

    printf("End of execution with RR.\n");
}


void policy_LT(int slice)
{
    printf("Execution trace with LT:\n");

    // Leave this here, it will ensure the scheduling behavior remains deterministic
    srand(42);

    // In the following, you'll need to:
    // Figure out which active job to run first
    // Pick the job with the shortest remaining time
    // Considers jobs in order of arrival, so implicitly breaks ties by choosing the job with the lowest ID

    // To achieve consistency with the tests, you are encouraged to choose the winning ticket as follows:
    // int winning_ticket = rand() % total_tickets;
    // And pick the winning job using the linked list approach discussed in class, or equivalent

    printf("End of execution with LT.\n");

}


void policy_FIFO(){
    printf("Execution trace with FIFO:\n");
    int now = 0;
    int done = 0;

    while (done < numofjobs) {
        struct job *best = NULL;
        // FIFO runs the earliest-arriving unfinished job to completion.
        for (struct job *j = head; j != NULL; j = j->next) {
            if (j->remaining <= 0)
                continue;
            if (best == NULL || j->arrival < best->arrival ||
                (j->arrival == best->arrival && j->id < best->id))
                best = j;
        }
        if (now < best->arrival)
            now = best->arrival;  // CPU was idle.
        int duration = best->remaining;
        run_segment(best, now, duration);
        now += duration;
        done++;
    }

    // TODO: implement FIFO policy

    printf("End of execution with FIFO.\n");
}


int main(int argc, char **argv){

    static char usage[] = "usage: %s analysis policy slice trace\n";

    int analysis;
    char *pname;
    char *tname;
    int slice;


    if (argc < 5)
    {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, usage, argv[0]);
		exit(1);
    }

    // if 0, we don't analysis the performance
    analysis = atoi(argv[1]);

    // policy name
    pname = argv[2];

    // time slice, only valid for RR
    slice = atoi(argv[3]);

    // workload trace
    tname = argv[4];

    read_job_config(tname);

    if (strcmp(pname, "FIFO") == 0){
        policy_FIFO();
        if (analysis == 1){
            // TODO: perform analysis
        }
    }
    else if (strcmp(pname, "SJF") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "STCF") == 0)
    {
        policy_STCF();

        if (analysis == 1) {
            printf("Begin analyzing STCF:\n");

            print_analysis();

            printf("End analyzing STCF.\n");
        } 
    }
    else if (strcmp(pname, "RR") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "LT") == 0)
    {
        // TODO
    }

	exit(0);
}

int get_job(struct job **job, int time) {
    *job = NULL;

    int t = 0;
    while (*job == NULL) {
        for (struct job *cur = head; cur != NULL; cur = cur->next) {
            if (cur->arrival <= t && cur->completion_time < 0) {
                *job = cur;
                return 0;
            }
        }

        t++;

        if (t > time) break;
    }

    return -1;
}

int cpu_tick(struct job *job, int time) {
    if (job->start_time < 0) {
        job->start_time = time;
    }

    job->time_ran++;

    if (job->last_ran >= 0) {
        job->wait_time += time - job->last_ran - 1;
    }
    else {
        job->wait_time = time - job->arrival;
    }

    job->last_ran = time; 

    if (job->time_ran == job->length) {
        job->completion_time = time + 1;
    }

    return time + 1;
}

bool check_done() {
    for (struct job *cur = head; cur != NULL; cur = cur->next) {
        if (cur->completion_time < 0) {
            return false;
        }
    }

    return true;
}

void print_status(struct job *job) {
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", job->current_start_time, job->id, job->arrival, job->current_time_ran);
}

void print_analysis() {
    int total_response = 0;
    int total_turnaround = 0;
    int total_wait = 0;

    for (struct job* cur = head; cur != NULL; cur = cur->next) {
        int response = cur->start_time - cur->arrival;
        int turnaround = cur->completion_time - cur->arrival;
        int wait = cur->wait_time;

        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", cur->id, response, turnaround, wait);

        total_response += response;
        total_turnaround += turnaround;
        total_wait += wait;
    }

    float average_response = (float)total_response / numofjobs;
    float average_turnaround = (float)total_turnaround / numofjobs;
    float average_wait = (float)total_wait / numofjobs;

    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", average_response, average_turnaround, average_wait);
}
