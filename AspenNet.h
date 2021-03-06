/*
 * Aspen-Codes integration model
 * Author: Mark Blanco
 * Date: 24 June 2015
 * Note: this model is directly based off of the codes-base example (example.c)
 * They may look very similar for a while
 * The CODES project can be found at http://www.mcs.anl.gov/research/projects/codes/
 * The ROSS project, which codes is based on, can be found at 
 * https://github.com/carothersc/ROSS/wiki
 * Finally, ASPEN can be found at https://ft.ornl.gov/node/1566...sort of.
 */

#ifndef ASPEN_HEADER
#define ASPEN_HEADER

#include <string.h>
#include <assert.h>
#include <ross.h>
#include <codes/configuration.h>

typedef struct svr_msg aspen_svr_msg;
typedef struct svr_state aspen_svr_state;

/* Types of network traffic patterns */
typedef enum traffic_pattern_enum
{
    NEXTNEIGHBOR,   /* Default traffic pattern */
    RANDOM          /* Random traffic pattern */
};

/* types of events that will constitute server activities */
enum svr_event
{
    KICKOFF,    /* initial event */
    RESTART,    /* multi-round kickoff */
    REQ,        /* request event */
    ACK,        /* ack event */
    LOCAL,      /* local event */
    ASPENCOMP,  /* event during which Aspen will be called */
    DATA        /* Send start and end timestamps to the 0 LP */
};

/* this struct serves as the ***persistent*** state of the LP representing the 
 * server in question. This struct is setup when the LP initialization function
 * ptr is called */
struct svr_state
{
    int msg_sent_count;   /* requests sent */
    int msg_recvd_count;  /* requests recvd */
    int local_recvd_count; /* number of local messages received */
    tw_stime start_ts;    /* time that we started sending requests */
    tw_stime end_ts;      /* time that last request finished */
    tw_stime start_global;  /* global earliest start time */
    tw_stime end_global;    /* global latest end time */
    /* Note that the globals above are obviously not global,
     * but need to be stored here for purposes of reverse event handling */ 
    unsigned int data_recvd; /* counter for data sent to LP 0 */
};

/* this struct serves as the ***temporary*** event data, which can be thought
 * of as a message between two LPs. */
struct svr_msg
{
    enum svr_event aspen_svr_event_type;
    tw_lpid src;          /* source of this request or ack */
    int incremented_flag; /* helper for reverse computation */
    tw_stime start_ts;    /* storage of start time for data */
    tw_stime end_ts;      /* storage of end time for data */
};

// Aspen runtimeCalc function declaration:
double runtimeCalc(char * app, char * machine, char * socket);

/* ROSS expects four functions per LP:
 * - an LP initialization function, called for each LP
 * - an event processing function
 * - a *reverse* event processing function (rollback), and
 * - a finalization/cleanup function when the simulation ends
 */
static void aspen_svr_init(
    aspen_svr_state * ns,
    tw_lp * lp);
static void aspen_svr_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void aspen_svr_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void aspen_svr_finalize(
    aspen_svr_state * ns,
    tw_lp * lp);

/* set up the function pointers for ROSS, as well as the size of the LP state
 * structure (NOTE: ROSS is in charge of event and state (de-)allocation) */
tw_lptype svr_lp = {
    (init_f) aspen_svr_init,
    (pre_run_f) NULL,
    (event_f) aspen_svr_event,
    (revent_f) aspen_svr_rev_event,
    (final_f)  aspen_svr_finalize,
    (map_f) codes_mapping,
    sizeof(aspen_svr_state),
};

extern const tw_lptype* svr_get_lp_type();
static void svr_add_lp_type();
static tw_stime ns_to_s(tw_stime ns);
static tw_stime s_to_ns(tw_stime ns);

/* as we only have a single event processing entry point and multiple event
 * types, for clarity we define "handlers" for each (reverse) event type */
static void handle_kickoff_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_restart_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_ack_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_req_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_local_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
   tw_lp * lp);
static void handle_data_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_computation_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_local_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
   tw_lp * lp);
static void handle_kickoff_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_restart_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_ack_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_req_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_data_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);
static void handle_computation_rev_event(
    aspen_svr_state * ns,
    tw_bf * b,
    aspen_svr_msg * m,
    tw_lp * lp);


/* for this simulation, each server contacts its neighboring server in an id.
 * this function shows how to use the codes_mapping API to calculate IDs when
 * having to contend with multiple LP types and counts. */
static tw_lpid get_next_server(tw_lp *lp);

/* arguments to be handled by ROSS - strings passed in are expected to be
 * pre-allocated */
static char aspen_conf_file_name[256] = {0};
/* this struct contains user-specific arguments to be handled 
 * by the ROSS config sys. Pass it in prior to calling tw_init */

/* two value-swapper functions for processing the start and end 
 * timestamps received in data events */

static char network_conf_file_name[256] = {0};
/* This struct contains the ROSS/network configuration settings. */

inline void swap_start(aspen_svr_state * ns, aspen_svr_msg * m)
{
    tw_stime temp = ns->start_global;
    ns->start_global = m->start_ts;
    m->start_ts = temp;
}

inline void swap_end(aspen_svr_state * ns, aspen_svr_msg * m)
{
    tw_stime temp = ns->end_global;
    ns->end_global = m->end_ts;
    m->end_ts = temp;
}

const tw_optdef app_opt [] =
{
	TWOPT_GROUP("Model net test case" ),
        TWOPT_CHAR("conf", aspen_conf_file_name, "name of AspenNet configuration file"),
	TWOPT_END()
};

/* Helper function to return a stringified version of num.
 * This returns an int giving the number of digits, as well as
 * filling a buffer (arg: array) with the stringified number */
int int_to_array(int num, char** array){
    int temp = num;
    int count = 0;
    while (temp > 0){
        temp /= 10;
        count ++;
    }
    if (count == 0) count = 1;
    *array = calloc(count, sizeof(char));
    temp = count;
    while (count > 0){
        count --;
        (*array)[count] = (num % 10) + '0';
        num /= 10;
    }
    return temp;
}
#endif
