#include "codexion.h"

static void	push_to_queue(t_coder *coder, t_dongle *dongle)
{
	int			i;
	int			parent;
	t_heap_node	tmp;

	i = dongle->queue_size;
	dongle->queue[i].coder_id = coder->id;
	dongle->queue[i].priority = get_priority(coder, dongle);
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

static int	get_smallest(t_dongle *dongle, int i)
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

static void	pop_from_queue(t_dongle *dongle)
{
	int			i;
	int			smallest;
	t_heap_node	tmp;

	if (dongle->queue_size == 0)
		return ;
	dongle->queue_size--;
	dongle->queue[0] = dongle->queue[dongle->queue_size];
	i = 0;
	while (1)
	{
		smallest = get_smallest(dongle, i);
		if (smallest == i)
			break ;
		tmp = dongle->queue[smallest];
		dongle->queue[smallest] = dongle->queue[i];
		dongle->queue[i] = tmp;
		i = smallest;
	}
}

void	grab_dongle(t_coder *coder, t_dongle *dongle)
{
	struct timespec	ts;
	long			target;

	pthread_mutex_lock(&dongle->mutex);
	push_to_queue(coder, dongle);
	while (dongle->in_use
		|| dongle->queue[0].coder_id != coder->id
		|| get_time_ms() < dongle->cooldown_until)
	{
		target = dongle->cooldown_until;
		ts.tv_sec = target / 1000;
		ts.tv_nsec = (target % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		if (should_stop(coder->sim))
		{
			pop_from_queue(dongle);
			pthread_mutex_unlock(&dongle->mutex);
			return ;
		}
	}
	pop_from_queue(dongle);
	dongle->in_use = 1;
	dongle->owner_id = coder->id;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->owner_id = -1;
	dongle->cooldown_until = get_time_ms() + coder->sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
