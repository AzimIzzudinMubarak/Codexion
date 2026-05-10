/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_queue.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azmubara <azmubara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:08:33 by azmubara          #+#    #+#             */
/*   Updated: 2026/05/10 14:08:33 by azmubara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	get_smallest(t_dongle *dongle, int i)
{
	int		smallest;
	int		left;
	int		right;

	left = 2 * i + 1;
	right = 2 * i + 2;
	smallest = i;
	if (left < dongle->queue_size
		&& dongle->queue[left].priority < dongle->queue[smallest].priority)
		smallest = left;
	if (right < dongle->queue_size
		&& dongle->queue[right].priority < dongle->queue[smallest].priority)
		smallest = right;
	return (smallest);
}

void	push_to_queue(t_coder *coder, t_dongle *dongle, long priority)
{
	int			i;
	int			parent;
	t_heap_node	tmp;

	i = dongle->queue_size;
	dongle->queue[i].coder_id = coder->id;
	dongle->queue[i].priority = priority;
	dongle->queue_size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (dongle->queue[parent].priority <= dongle->queue[i].priority)
			break ;
		tmp = dongle->queue[parent];
		dongle->queue[parent] = dongle->queue[i];
		dongle->queue[i] = tmp;
		i = parent;
	}
}
