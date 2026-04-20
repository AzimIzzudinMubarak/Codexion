#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>

/* --- Structures --- */

typedef struct s_dongle {
	pthread_mutex_t		mutex;
	pthread_cond_t		condition;
	long				cooldown_end;
} t_dongle;

typedef struct s_coder {
	int					id;
	pthread_t			thread_id;
	int					compiles_done;
	long				last_compile_start;

	t_dongle			*left_dongle;
	t_dongle			*right_dongle;

	pthread_mutex_t		coder_mutex;
	t_data				*data;
} t_coder;

typedef struct s_data {
	int					num_coders;
	int					time_to_burnout;
	int					time_to_compile;
	int					time_to_debug;
	int					time_to_refactor;
	int					num_compiles_required;
	int					dongle_cooldown;
	int					scheduler;

	long				start_time;
	int					simulation_running;
	pthread_mutex_t		stop_mutex;
	pthread_mutex_t		log_mutex;

	struct s_coder		*coders;
	t_dongle			*dongles;
} t_data;

#endif