#include "codexion.h"

long get_time_ms(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void log_state(t_sim *sim, int id, char *state)
{
	pthread_mutex_lock(&sim->log_mutex);
	if (!should_stop(sim))
		printf("%ld %d %s\n", get_time_ms() - sim->start_time, id, state);
	pthread_mutex_unlock(&sim->log_mutex);
}

int should_stop(t_sim *sim)
{
	int result;

	pthread_mutex_lock(&sim->stop_mutex);
	result = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (result);
}

int should_done(t_sim *sim)
{
	int result;

	pthread_mutex_lock(&sim->stop_mutex);
	result = sim->done;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (result);
}