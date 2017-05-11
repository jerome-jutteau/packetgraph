/* Copyright 2017 Outscale SAS
 *
 * This file is part of Packetgraph.
 *
 * Packetgraph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Packetgraph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Packetgraph.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <packetgraph/packetgraph.h>
#include "limiter.h"
#include <rte_cycles.h>
#include <rte_mbuf.h>

void pg_limiter_init(struct pg_limiter *l,
		     uint16_t burst_size_max,
		     uint64_t bps)
{
	l->next_date = 0;
	l->last_go_date = 0;
	l->pkt_size_mean = 98;
	l->burst_size_max = burst_size_max;
	l->bps = bps ? bps : 10e9;
}

#define SLIDING_MEAN(mean, new_value, rate) \
	{ mean = ((mean) * (rate) + (new_value)) / (1.0 * ((rate) + 1)); }

void pg_limiter_update(struct pg_limiter *l,
		       struct rte_mbuf **pkts,
		       uint16_t count)
{
	uint64_t burst_size_bits;
	double time_per_burst;
	uint64_t cycles_per_poll;

	if (count) {
		uint64_t data_sum = 0;

		for (int i = 0; i < count; i++)
			data_sum += rte_pktmbuf_pkt_len(pkts[i]);
		SLIDING_MEAN(l->pkt_size_mean,
			     data_sum / (1.0 * count),
			     10);
	}

	burst_size_bits = l->pkt_size_mean * 8 * l->burst_size_max;
	time_per_burst = burst_size_bits / (l->bps * 1.0);
	cycles_per_poll = rte_get_timer_hz() * time_per_burst;
	l->next_date = l->last_go_date + cycles_per_poll;
}
#undef SLIDING_MEAN

int pg_limiter_go(struct pg_limiter *l)
{
	uint64_t date = rte_get_timer_cycles();

	if (date < l->next_date)
		return 0;
	l->last_go_date = date;
	return 1;
}

