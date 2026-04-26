#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <sys/time.h>

/* --- Structures --- */

typedef struct s_heap_node {
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

typedef struct s_sim t_sim;

typedef struct s_coder {
	int					id;
	pthread_t			thread;
	struct s_sim		*sim;
	long				last_compile;
	int					compile_count;
	t_dongle			*first;
	t_dongle			*second;
	int					burned_out;
} t_coder;

struct s_sim {
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
};

/* --- Functions --- */

// utils
long	get_time_ms(void);
void	log_state(t_sim *sim, int id, char *state);
int		should_stop(t_sim *sim);

// parser
int		parse_args(t_sim *sim, int argc, char **argv);

// init
int		init_sim(t_sim *sim, int argc, char **argv);

// cleanup
void	cleanup_sim(t_sim *sim);

// coder thread
void	*coder_routine(void *arg);

// monitor thread
void	*monitor_routine(void *arg);

// debug log
void	print_debug_header(t_sim *sim);
void	debug_log(t_sim *sim, int coder_id, char *event, char *status);

#endif