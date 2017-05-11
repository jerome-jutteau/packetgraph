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

void pg_limiter_init(struct pg_limiter *l)
{
	l->next_date = 0;
	l->mtu_mean = 68;
	l->burst_size_mean = 0;
}

#define MEAN_RATE 30
void pg_limiter_update(struct pg_limiter *l,
		       struct rte_mbuf **pkts,
		       uint16_t count,
		       uint64_t bps)
{
	uint64_t data_sum = 0;
	uint16_t mtu = 0;
	uint64_t cycles_per_poll;

	if (count) {
		for (int i = 0; i < count; i++)
			data_sum += rte_pktmbuf_pkt_len(pkts[i]);
		mtu = data_sum / count;
		l->mtu_mean = (l->mtu_mean * MEAN_RATE + mtu) / (MEAN_RATE + 1);
	}

	l->burst_size_mean = (l->burst_size_mean * MEAN_RATE + count) /
		(MEAN_RATE + 1);

	if (bps == 0)
		bps = 10e9;

	cycles_per_poll = rte_get_timer_hz() * l->mtu_mean  * 8 *
		l->burst_size_mean / bps;
	l->next_date = rte_get_timer_cycles() + cycles_per_poll;
}
#undef MEAN_RATE

int pg_limiter_go(struct pg_limiter *l)
{
	return likely(rte_get_timer_cycles() < l->next_date) ? 1 : 0;
}

