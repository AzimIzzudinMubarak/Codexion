#include "codexion.h"

static void cleanup_dongles(t_sim *sim)
{
	int i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->nb_coders)
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

static void	cleanup_coders(t_sim *sim)
{
    if (!sim->coders)
        return ;
    free(sim->coders);
    sim->coders = NULL;
}

void cleanup_sim(t_sim *sim)
{
    cleanup_dongles(sim);
    cleanup_coders(sim);
    pthread_mutex_destroy(&sim->log_mutex);
    pthread_mutex_destroy(&sim->stop_mutex);
}