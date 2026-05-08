#include "codexion.h"

long	get_priority(t_coder *coder, t_dongle *dongle)
{
	if (coder->sim->scheduler == 0)
		return (dongle->arrival_counter++);
	return (coder->last_compile + coder->sim->time_to_burnout);
}

static void	do_compile(t_coder *coder, t_sim *sim)
{
	pthread_mutex_lock(&sim->log_mutex);
	coder->last_compile = get_time_ms();
	pthread_mutex_unlock(&sim->log_mutex);
	log_state(sim, coder->id, "is compiling");
	usleep(sim->time_to_compile * 1000);
	pthread_mutex_lock(&sim->log_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->log_mutex);
	release_dongle(coder, coder->first);
	if (coder->second != coder->first)
		release_dongle(coder, coder->second);
	log_state(sim, coder->id, "is debugging");
	usleep(sim->time_to_debug * 1000);
	log_state(sim, coder->id, "is refactoring");
	usleep(sim->time_to_refactor * 1000);
}

static int	try_grab_second(t_coder *coder)
{
	t_dongle	*d;
	int			got;

	d = coder->second;
	got = 0;
	pthread_mutex_lock(&d->mutex);
	if (!d->in_use && get_time_ms() >= d->cooldown_until)
	{
		d->in_use = 1;
		d->owner_id = coder->id;
		got = 1;
	}
	pthread_mutex_unlock(&d->mutex);
	return (got);
}

static int	grab_both_dongles(t_coder *coder, t_sim *sim)
{
	grab_dongle(coder, coder->first);
	if (should_stop(sim))
	{
		release_dongle(coder, coder->first);
		return (0);
	}
	if (!try_grab_second(coder))
	{
		release_dongle(coder, coder->first);
		usleep(1000);
		return (-1);
	}
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	if (should_stop(sim))
	{
		release_dongle(coder, coder->first);
		release_dongle(coder, coder->second);
		return (0);
	}
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;
	int		result;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (sim->nb_coders == 1)
	{
		while (!should_stop(sim))
			usleep(1000);
		return (NULL);
	}
	while (!should_stop(sim))
	{
		if (coder->compile_count >= sim->nb_compiles_required)
			break ;
		result = grab_both_dongles(coder, sim);
		if (result == 0)
			break ;
		if (result == 1)
			do_compile(coder, sim);
	}
	return (NULL);
}
