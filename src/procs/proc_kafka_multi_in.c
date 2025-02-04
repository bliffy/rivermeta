/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2012, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Apache Kafka consumer & producer performance tester
 * using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */


//#define DEBUG 1
#define PROC_NAME "kafka_multi_in"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if (defined WIN32 || defined WIN64 || defined WINDOWS) 
#include "win_mman.h"
#else
#include <sys/mman.h>
#endif
#include <signal.h>
#include "librdkafka/rdkafka.h"  /* for Kafka driver */
#include "waterslide.h"
#include "waterslidedata.h"
#include "procloader.h"
#include "sysutil.h"
#include "wstypes.h"
#include "datatypes/wsdt_tuple.h"
#include "datatypes/wsdt_binary.h"

char proc_version[]     = "1.1";
char *proc_tags[]     = { "source", "input", NULL };
char *proc_alias[]     = { NULL };
char proc_name[]       = PROC_NAME;
char proc_purpose[]    = "multi topic kafka subscriber";
char proc_nonswitch_opts[] = "kafka topic:partition to subscribe";
proc_option_t proc_opts[] = {
     /*  'option character', "long option string", "option argument",
	 "option description", <allow multiple>, <required>*/
     {'b',"","broker",
     "Specify a host and port for broker",0,0},
     {'L',"","label",
     "Specify an output label default DATA",0,0},
     {'g',"","group",
     "consumer group",0,0},
     {'S',"","",
     "detect if buffer is ascii strings",0,0},
     //the following must be left as-is to signify the end of the array
     {' ',"","",
     "",0,0}
};

//function prototypes for local functions
static int data_source(void *, wsdata_t*, ws_doutput_t*, int);

#define GRP_MAX 64
typedef struct _proc_instance_t {
     uint64_t meta_process_cnt;
     uint64_t outcnt;

     ws_outtype_t * outtype_tuple;
     char * tasks;
     char * brokers;
     int partition;
     char * topic;
     char group_default[GRP_MAX];
     char * group;
     wslabel_t * label_buf;
     wslabel_t * label_tuple;
     wslabel_t * label_topic;
     wslabel_t * label_datetime;

     rd_kafka_t *rk;
	rd_kafka_conf_t *conf;
	rd_kafka_topic_conf_t *topic_conf;
     rd_kafka_topic_partition_list_t *topics;
	char errstr[512];
     ws_doutput_t * dout;

     int stringdetect;
} proc_instance_t;

static int proc_cmd_options(int argc, char ** argv, 
                             proc_instance_t * proc, void * type_table) {

     int op;

     while ((op = getopt(argc, argv, "g:Sb:p:L:")) != EOF) {
          switch (op) {
          case 'g':
               proc->group = optarg;
               break;
          case 'b':
               proc->brokers = optarg;
               tool_print("using broker: %s", optarg);
               break;
          case 'p':
               proc->partition = atoi(optarg);
               tool_print("using partition: %d", proc->partition);
               break;
          case 'L':
               proc->label_buf = wsregister_label(type_table, optarg);
               break;
          case 'S':
               proc->stringdetect = 1;
               break;
          default:
               return 0;
          }
     }
     if (optind < argc) {
          proc->topics = rd_kafka_topic_partition_list_new(argc - optind);
     }
     int i;
     for (i = optind ; i < argc ; i++) {
          /* Parse "topic[:part] */
          char *topic = argv[i];
          char *t;
          int32_t partition = -1;

          if ((t = strstr(topic, ":"))) {
               *t = '\0';
               partition = atoi(t+1);
          }

          rd_kafka_topic_partition_list_add(proc->topics, topic, partition);
          if (partition >= 0) {
               tool_print("reading from topic %s:%d", topic, partition);
          }
          else {
               tool_print("reading from topic %s", topic);
          }
     }

     return 1;
}
/*
static void print_partition_list (FILE *fp,
                                  const rd_kafka_topic_partition_list_t
                                  *partitions) {
     int i;
     for (i = 0 ; i < partitions->cnt ; i++) {
          fprintf(stderr, "%s %s [%"PRId32"] offset %"PRId64,
                  i > 0 ? ",":"",
                  partitions->elems[i].topic,
                  partitions->elems[i].partition,
                  partitions->elems[i].offset);
     }
     fprintf(stderr, "\n");

}*/

static void rebalance_cb (rd_kafka_t *rk,
                          rd_kafka_resp_err_t err,
                          rd_kafka_topic_partition_list_t *partitions,
                          void *opaque) {
     proc_instance_t * proc = (proc_instance_t*)opaque;
     tool_print("%% Consumer group rebalanced: ");

     switch (err) {
     case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
          tool_print("      assigned");
          //print_partition_list(stderr, partitions);
          rd_kafka_assign(proc->rk, partitions);
          break;

     case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
          tool_print("      revoked");
          //print_partition_list(stderr, partitions);
          rd_kafka_assign(proc->rk, NULL);
          break;

     default:
          tool_print("      failed: %s", rd_kafka_err2str(err));
          rd_kafka_assign(proc->rk, NULL);
          break;
     }
}

static void err_cb (rd_kafka_t *rk, int err, const char *reason, void *opaque) {
	printf("%% ERROR CALLBACK: %s: %s: %s\n",
	       rd_kafka_name(rk), rd_kafka_err2str(err), reason);
}

static void throttle_cb (rd_kafka_t *rk, const char *broker_name,
			 int32_t broker_id, int throttle_time_ms,
			 void *opaque) {
	printf("%% THROTTLED %dms by %s (%"PRId32")\n", throttle_time_ms,
	       broker_name, broker_id);
}
                                        
// the following is a function to take in command arguments and initalize
// this processor's instance..
//  also register as a source here..
// return 1 if ok
// return 0 if fail
int proc_init(wskid_t * kid, int argc, char ** argv, void ** vinstance, ws_sourcev_t * sv,
              void * type_table) {

     //allocate proc instance of this processor
     proc_instance_t * proc =
          (proc_instance_t*)calloc(1,sizeof(proc_instance_t));
     *vinstance = proc;

     proc->label_buf = wsregister_label(type_table, "BUF");
     proc->label_tuple = wsregister_label(type_table, "KAFKA");
     proc->label_topic = wsregister_label(type_table, "TOPIC");
     proc->label_datetime = wsregister_label(type_table, "DATETIME");

     snprintf(proc->group_default,GRP_MAX,"%s:%d", PROC_NAME, rand());
     proc->group = proc->group_default;

     proc->conf = rd_kafka_conf_new();
	rd_kafka_conf_set_error_cb(proc->conf, err_cb);
	rd_kafka_conf_set_throttle_cb(proc->conf, throttle_cb);

     /* Kafka topic configuration */
	proc->topic_conf = rd_kafka_topic_conf_new();
     rd_kafka_topic_conf_set(proc->topic_conf, "auto.offset.reset", "earliest",
                             NULL, 0);
     rd_kafka_topic_conf_set(proc->topic_conf, "offset.store.method",
                             "broker", NULL, 0);
     rd_kafka_conf_set_default_topic_conf(proc->conf, proc->topic_conf);

     proc->brokers = "localhost:9092";

     /* Create Kafka handle */
          //read in command options
     if (!proc_cmd_options(argc, argv, proc, type_table)) {
          return 0;
     }
     tool_print("using subscriber group %s", proc->group);

     if (rd_kafka_conf_set(proc->conf, "group.id", proc->group,
                           proc->errstr, sizeof(proc->errstr)) !=
         RD_KAFKA_CONF_OK) {
          fprintf(stderr, "%% %s\n", proc->errstr);
          exit(1);
     }
     rd_kafka_conf_set_opaque(proc->conf, (void*)proc);
     rd_kafka_conf_set_rebalance_cb(proc->conf, rebalance_cb);
     if (!(proc->rk = rd_kafka_new(RD_KAFKA_CONSUMER, proc->conf,
                             proc->errstr, sizeof(proc->errstr)))) {
          fprintf(stderr,
                  "%% Failed to create Kafka consumer: %s\n",
                  proc->errstr);
          return 0;
     }

     /* Add broker(s) */
     if (rd_kafka_brokers_add(proc->rk, proc->brokers) < 1) {
          fprintf(stderr, "%% No valid brokers specified\n");
          return 0;
     }

     rd_kafka_poll_set_consumer(proc->rk);

     if (!proc->topics || !proc->topics->cnt) {
          tool_print("no topics specified");
          return 0;
     }
     rd_kafka_resp_err_t err;

     /* Start consuming */
     if ((err = rd_kafka_subscribe(proc->rk, proc->topics))) {
          fprintf(stderr,
                  "%% Failed to start consuming topics: %s\n",
                  rd_kafka_err2str(err));
          return 0;
     }


     proc->outtype_tuple =
          ws_register_source_byname(type_table, "TUPLE_TYPE", data_source, sv);


     if (proc->outtype_tuple == NULL) {
          tool_print("registration failed");
          return 0;
     }

     return 1; 
}

// this function needs to decide on processing function based on datatype
// given.. also set output types as needed (unless a sink)
//return 1 if ok
// return 0 if problem
proc_process_t proc_input_set(void * vinstance, wsdatatype_t * input_type,
                              wslabel_t * port,
                              ws_outlist_t* olist, int type_index,
                              void * type_table) {
     return NULL;
}

//callback per kafka message
static void msg_consume (rd_kafka_message_t *rkmessage, void *vproc) {
     proc_instance_t * proc = (proc_instance_t*)vproc;
	if (rkmessage->err) {
		if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
               dprint("%% Consumer reached end of "
                      "%s [%"PRId32"] "
                      "message queue at offset %"PRId64"\n",
                      rd_kafka_topic_name(rkmessage->rkt),
                      rkmessage->partition, rkmessage->offset);
               return;
          }

		printf("%% Consume error for topic \"%s\" [%"PRId32"] "
		       "offset %"PRId64": %s\n",
		       rkmessage->rkt ? rd_kafka_topic_name(rkmessage->rkt):"",
		       rkmessage->partition,
		       rkmessage->offset,
		       rd_kafka_message_errstr(rkmessage));


                /*
                   if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION ||
                    rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC)
                        run = 0;
                 */
		return;
	}
     if (rkmessage->len) {

          //allocate tuple
          wsdata_t * tuple = wsdata_alloc(dtype_tuple);
          if (!tuple) {
               return;
          }
          const char * topic = rd_kafka_topic_name(rkmessage->rkt);
          if (topic) {
               tuple_dupe_string(tuple, proc->label_topic, topic,
                                 strlen(topic));
          }
          if (proc->stringdetect) {
               int isbinary = 0;
               int i;
               int len = (int)rkmessage->len;
               char * buf = (char *)rkmessage->payload;
               for (i = 0; i < len; i++) {
                    if (!isprint(buf[i])) {
                         isbinary = 1;
                         break;
                    }
               }
               if (isbinary) {
                    tuple_dupe_binary(tuple, proc->label_buf, (char *)rkmessage->payload,
                            (int)rkmessage->len);
               }
               else {
                    tuple_dupe_string(tuple, proc->label_buf, (char *)rkmessage->payload,
                                      (int)rkmessage->len);
               }
          }
          else {
               tuple_dupe_binary(tuple, proc->label_buf, (char *)rkmessage->payload,
                                 (int)rkmessage->len);
          }

          ws_set_outdata(tuple, proc->outtype_tuple, proc->dout);
          proc->outcnt++;

     }
     dprint("payload> %.*s\n",
            (int)rkmessage->len,
            (char *)rkmessage->payload);
}

//// proc processing function assigned to a specific data type in proc_io_init
//return 1 if output is available
// return 0 if not output
//
static int data_source(void * vinstance, wsdata_t* source_data,
                       ws_doutput_t * dout, int type_index) {

     proc_instance_t * proc = (proc_instance_t*)vinstance;
     proc->dout = dout;

     proc->meta_process_cnt++;
     rd_kafka_message_t *rkmessage;

     rkmessage = rd_kafka_consumer_poll(proc->rk, 1000);
     if (rkmessage) {
          dprint("got message");
          msg_consume(rkmessage, proc);
          rd_kafka_message_destroy(rkmessage);
     }

     return 1;
}

//return 1 if successful
//return 0 if no..
int proc_destroy(void * vinstance) {
     proc_instance_t * proc = (proc_instance_t*)vinstance;
     tool_print("meta_proc cnt %" PRIu64, proc->meta_process_cnt);
     tool_print("output cnt %" PRIu64, proc->outcnt);

     if (proc->rk) {
          rd_kafka_resp_err_t err;
          err = rd_kafka_consumer_close(proc->rk);
          if (err) {
               fprintf(stderr, "%% Failed to close consumer: %s\n",
                       rd_kafka_err2str(err));
          }
     }
     if (proc->topics) {
          rd_kafka_topic_partition_list_destroy(proc->topics);
     }

     if (proc->rk) {
          rd_kafka_destroy(proc->rk);
     }
     /* Let background threads clean up and terminate cleanly. */
     tool_print("kafka listener destroy");
     rd_kafka_wait_destroyed(2000);

     //free dynamic allocations
     free(proc);
     return 1;
}

