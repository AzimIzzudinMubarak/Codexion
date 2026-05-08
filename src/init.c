#include "codexion.h"

int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->nb_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].owner_id = -1;
		sim->dongles[i].cooldown_until = 0;
		sim->dongles[i].queue_size = 0;
		sim->dongles[i].arrival_counter = 0;
		sim->dongles[i].queue = malloc(sizeof(t_heap_node) * sim->nb_coders);
		if (!sim->dongles[i].queue)
			return (0);
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
			return (0);
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

static void	assign_dongles(t_sim *sim, int i, t_dongle *left, t_dongle *right)
{
	if (left->id < right->id)
	{
		sim->coders[i].first = left;
		sim->coders[i].second = right;
	}
	else
	{
		sim->coders[i].first = right;
		sim->coders[i].second = left;
	}
}

int	init_coders(t_sim *sim)
{
	int			i;
	t_dongle	*left;
	t_dongle	*right;

	sim->coders = malloc(sizeof(t_coder) * sim->nb_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->nb_coders)
	{
		left = &sim->dongles[i];
		right = &sim->dongles[(i + 1) % sim->nb_coders];
		sim->coders[i].id = i + 1;
		sim->coders[i].sim = sim;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile = sim->start_time;
		sim->coders[i].burned_out = 0;
		assign_dongles(sim, i, left, right);
		i++;
	}
	return (1);
}

static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (0);
	return (1);
}

int	init_sim(t_sim *sim, int argc, char **argv)
{
	sim->coders = NULL;
	sim->dongles = NULL;
	sim->stop = 0;
	if (!parse_args(sim, argc, argv))
		return (0);
	if (!init_mutexes(sim))
		return (0);
	sim->start_time = get_time_ms();
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	return (1);
}
