/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azmubara <azmubara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:09:08 by azmubara          #+#    #+#             */
/*   Updated: 2026/05/10 14:09:08 by azmubara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

static int	grab_both_dongles(t_coder *coder, t_sim *sim)
{
	grab_dongle(coder, coder->first);
	if (should_stop(sim))
	{
		release_dongle(coder, coder->first);
		return (0);
	}
	grab_dongle(coder, coder->second);
	if (should_stop(sim))
	{
		release_dongle(coder, coder->first);
		release_dongle(coder, coder->second);
		return (0);
	}
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

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
		if (grab_both_dongles(coder, sim))
			do_compile(coder, sim);
	}
	return (NULL);
}
