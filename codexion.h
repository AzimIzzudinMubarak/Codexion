#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <sys/time.h>

/* --- Structures --- */

typedef struct s_heap_dongle {
	int					coder_id;
	long				priority;
} t_heap_node;


typedef struct s_dongle {
	int					id;
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	int					in_use;
	int					owner_id;
	long				cooldown_until;
	t_heap_node			*queue;
	int					queue_size;
	long				arrival_counter;
} t_dongle;

typedef struct s_coder {
	int					id;
	pthread_t			thread;
	struct s_sim		*sim;
	long				last_compile;
	int					compile_count;
	t_dongle			*left;
	t_dongle			*right;
	int					burned_out;
} t_coder;

typedef struct s_sim {
	t_coder				*coders;
	t_dongle			*dongles;
	pthread_t			monitor;
	pthread_mutex_t		log_mutex;
	pthread_mutex_t		stop_mutex;
	int					stop;
	int					nb_coders;
	long				time_to_burnout;
	long				time_to_compile;
	long				time_to_debug;
	long				time_to_refactor;
	int					nb_compiles_required;
	long				dongle_cooldown;
	int					scheduler;
	long				start_time;
} t_sim;

/* --- Functions --- */

int init_dongles(t_sim *sim);
int init_coders(t_sim *sim);
int init_sim(t_sim *sim, int argc, char **argv);

int parse_args(t_sim *sim, int argc, char **argv);

void cleanup_sim(t_sim *sim);

long get_time_ms(void);

#endif