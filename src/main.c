/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azmubara <azmubara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:08:52 by azmubara          #+#    #+#             */
/*   Updated: 2026/05/10 14:08:52 by azmubara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	launch_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL,
			coder_routine, &sim->coders[i]);
		i++;
	}
}

static void	join_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (!init_sim(&sim, argc, argv))
	{
		write(2, "Error: invalid arguments\n", 25);
		return (1);
	}
	launch_coders(&sim);
	pthread_create(&sim.monitor, NULL, monitor_routine, &sim);
	join_coders(&sim);
	pthread_join(sim.monitor, NULL);
	cleanup_sim(&sim);
	return (0);
}
