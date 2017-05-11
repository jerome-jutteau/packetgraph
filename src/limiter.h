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

#ifndef _PG_LIMITER_H
#define _PG_LIMITER_H

struct pg_limiter {
	uint64_t next_date;
	uint64_t mtu_mean;
	uint64_t burst_size_mean;
};

void pg_limiter_init(struct pg_limiter *l);

/* Update poll limiter
 *
 * @l		limiter structure
 * @pkts	current burst
 * @count	number of packets in burst
 * @bps		bits par seconds to target
 */
void pg_limiter_update(struct pg_limiter *l,
		       struct rte_mbuf **pkts,
		       uint16_t count,
		       uint64_t bps);

/* Check if it's time to poll
 *
 * @return	1 when it's time to poll, 0 otherwise.
 * */
int pg_limiter_go(struct pg_limiter *l);

#endif /* _PG_LIMITER_H */
