#include "codexion.h"

static void	cleanup_dongles(t_sim *sim, int count)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		if (sim->dongles[i].queue)
			free(sim->dongles[i].queue);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

void	cleanup_sim(t_sim *sim)
{
	if (sim->dongles)
		cleanup_dongles(sim, sim->nb_coders);
	if (sim->coders)
	{
		free(sim->coders);
		sim->coders = NULL;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
}
