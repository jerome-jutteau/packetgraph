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

void pg_limiter_init(struct pg_limiter *l, uint16_t mtu)
{
	l->mtu = mtu;
}

#define MEAN_RATE 30
void pg_limiter_update(struct pg_limiter *l,
		       uint16_t mtu,
		       uint16_t burst_s,
		       uint64_t bps)
{
	/* sliding mean */
	l->mtu = (l->mtu * MEAN_RATE + mtu) / (MEAN_RATE + 1);
	if (bps == 0)
		bps = 10e9;
	l->cycles_per_poll = rte_get_timer_hz() * l->mtu  * 8 * burst_s / bps;
	l->next_date = rte_get_timer_cycles() + l->cycles_per_poll;
	l->need_update = 0;
}
#undef MEAN_RATE

enum pg_limiter_action pg_limiter_go(struct pg_limiter *l)
{
	uint64_t date = rte_get_timer_cycles();

	if (likely(date < l->next_date))
		return PG_LIMITER_WAIT;
	l->next_date = date + l->cycles_per_poll;
	if (l->need_update == 30000)
		return PG_LIMITER_POLL | PG_LIMITER_UPDATE;
	l->need_update++;
	return PG_LIMITER_POLL;
}

