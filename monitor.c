#include "codexion.h"

static void broadcast_all_dongles(t_sim *sim)
{
	int i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int check_all_done(t_sim *sim)
{
	int i;
	int done_count;

	done_count = 0;
	i = 0;
	while (i < sim->nb_coders)
	{
		if (sim->coders[i].compile_count
				>= sim->nb_compiles_required)
			done_count++;
		i++;
	}
	return (done_count == sim->nb_coders);
}

static void set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	broadcast_all_dongles(sim);
}

void *monitor_routine(void *arg)
{
	t_sim *sim;
	int i;
	long now;
	long last;

	sim = (t_sim *)arg;
	while (!should_stop(sim))
	{
		now = get_time_ms();
		i = 0;
		while (i < sim->nb_coders)
		{
			pthread_mutex_lock(&sim->log_mutex);
			last = sim->coders[i].last_compile;
			pthread_mutex_unlock(&sim->log_mutex);
			if (now - last > sim->time_to_burnout)
			{
				pthread_mutex_lock(&sim->log_mutex);
				printf("%ld %d has burned out\n",
					get_time_ms() - sim->start_time,
					sim->coders[i].id);
				pthread_mutex_unlock(&sim->log_mutex);
				set_stop(sim);
				return (NULL);
			}
			i++;
		}
		if (check_all_done(sim))
		{
			set_stop(sim);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}