#include "codexion.h"

void print_debug_header(t_sim *sim)
{
	int i;

	pthread_mutex_lock(&sim->log_mutex);
	printf("%-8s | %-28s", "time", "event");
	i = 0;
	while (i < sim->nb_coders)
	{
		printf(" | coder %-8d", i + 1);
		i++;
	}
	printf("\n");
	printf("%-8s-+-%-28s", "--------", "----------------------------");
	i = 0;
	while (i < sim->nb_coders)
	{
		printf("-+---------------");
		i++;
	}
	printf("\n");
	pthread_mutex_unlock(&sim->log_mutex);
}

/*
** event      — full description e.g. "C1 added to heap D0"
** coder_id   — which coder column gets the short status badge
** status     — short label e.g. "in heap D0", "grabbed D0", "compiling"
*/
void debug_log(t_sim *sim, int coder_id, char *event, char *status)
{
	int i;

	pthread_mutex_lock(&sim->log_mutex);
	printf("%6ldms | %-28s", get_time_ms() - sim->start_time, event);
	i = 0;
	while (i < sim->nb_coders)
	{
		if (i + 1 == coder_id)
			printf(" | %-15s", status);
		else
			printf(" | %-15s", "");
		i++;
	}
	printf("\n");
	pthread_mutex_unlock(&sim->log_mutex);
}