#include "codexion.h"

static void push_to_queue(t_dongle *dongle, int coder_id, long priority)
{
	int i;
	int parent;
	t_heap_node tmp;

	i = dongle->queue_size;
	dongle->queue[i].coder_id = coder_id;
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

static void pop_from_queue(t_dongle *dongle)
{
	int i;
	int left, right, smallest;
	t_heap_node tmp;

	if (dongle->queue_size == 0)
		return ;
	dongle->queue_size--;
	dongle->queue[0] = dongle->queue[dongle->queue_size];
	i = 0;
	while (1)
	{
		left  = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < dongle->queue_size
			&& dongle->queue[left].priority < dongle->queue[smallest].priority)
			smallest = left;
		if (right < dongle->queue_size
			&& dongle->queue[right].priority < dongle->queue[smallest].priority)
			smallest = right;
		if (smallest == i)
			break;
		tmp = dongle->queue[smallest];
		dongle->queue[smallest] = dongle->queue[i];
		dongle->queue[i] = tmp;
		i = smallest;
	}
}

long get_priority(t_coder *coder, t_dongle *dongle)
{
	if (coder->sim->scheduler == 0)
		return (dongle->arrival_counter++);
	return (coder->last_compile + coder->sim->time_to_burnout);
}

void	grab_dongle(t_coder *coder, t_dongle *dongle)
{
	struct timespec ts;
	long priority;

	pthread_mutex_lock(&dongle->mutex);
	priority = get_priority(coder, dongle);
	push_to_queue(dongle, coder->id, priority);
	while (dongle->in_use
		|| dongle->queue[0].coder_id != coder->id
		|| get_time_ms() < dongle->cooldown_until)
	{
		long target = dongle->cooldown_until + 1;
		ts.tv_sec  = target / 1000;
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

static void release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use         = 0;
	dongle->owner_id       = -1;
	dongle->cooldown_until = get_time_ms() + coder->sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void *coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!should_stop(sim))
	{
		log_state(sim, coder->id, "is waiting for dongles");
		grab_dongle(coder, coder->first);
		if (should_stop(sim))
		{
			release_dongle(coder, coder->first);
			break ;
		}
		grab_dongle(coder, coder->second);
		if (should_stop(sim))
		{
			release_dongle(coder, coder->first);
			release_dongle(coder, coder->second);
			break ;
		}
		// compile
		coder->last_compile = get_time_ms();
		log_state(sim, coder->id, "is compiling");
		usleep(sim->time_to_compile * 1000);
		coder->compile_count++;
		// release both
		release_dongle(coder, coder->first);
		release_dongle(coder, coder->second);
		// check if done
		if (coder->compile_count >= sim->nb_compiles_required)
		{
			log_state(sim, coder->id, "is done");
			break ;
		}
		// debug phase
		log_state(sim, coder->id, "is debugging");
		usleep(sim->time_to_debug * 1000);
		// refactor phase
		log_state(sim, coder->id, "is refactoring");
		usleep(sim->time_to_refactor * 1000);
	}
	return (NULL);
}